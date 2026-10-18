"""Write PDB class layouts into the JetEngine stub headers.

    python3 fillheaders.py <types.txt> <syms.txt> <lines.txt> <JetEngine root>

A class goes to the header named after it, else to the header next to the .cpp its methods were compiled from.
Templates and nested types are left out.
"""
import collections, os, re, sys
from pdblines import ranges

types_txt, syms, lines, root = sys.argv[1:5]
SHOW = sys.argv[5:]  # class names: print them (with vtable offsets) instead of writing headers
rec = {}
cur = None
for l in open(types_txt, errors="replace"):
    m = re.match(r"\s+0x([0-9A-F]+) \| (LF_\w+) \[size = \d+\](?: `(.*)`)?\s*$", l)
    if m:
        cur = rec[int(m.group(1), 16)] = [m.group(2), m.group(3), []]
    elif cur:
        cur[2].append(l.strip())

SIMPLE = {"char": 1, "signed char": 1, "unsigned char": 1, "bool": 1, "short": 2, "unsigned short": 2, "wchar_t": 2,
          "__int64": 8, "unsigned __int64": 8, "double": 8}
by_name = {}
for t, (k, n, b) in rec.items():
    if k in ("LF_CLASS", "LF_STRUCTURE", "LF_UNION") and "forward ref" not in b[-1]:
        by_name.setdefault(n, t)
simple = {}


def body(t):
    return " ".join(rec[t][2])


def ref(s, key):
    m = re.search(key + r"(?: =|:) 0x([0-9A-F]+)(?: \(([^)]*)\))?", s)
    if m:
        t = int(m.group(1), 16)
        if m.group(2) and t < 0x1000:
            simple[t] = m.group(2)
        return t


def sizeof(t):
    if t < 0x1000:
        n = simple.get(t, "int")
        return 4 if n.endswith("*") else SIMPLE.get(n, 4)
    k, n, _ = rec[t]
    b = body(t)
    if k in ("LF_POINTER", "LF_PROCEDURE"):
        return 4
    if k == "LF_MODIFIER":
        return sizeof(ref(b, "referent"))
    if k == "LF_ARRAY":
        return int(re.search(r"size: (\d+)", b).group(1))
    if k in ("LF_CLASS", "LF_STRUCTURE", "LF_UNION"):
        d = by_name.get(n, t)
        return int(re.search(r"sizeof (\d+)", body(d)).group(1))
    return 4


def args(t):
    return ", ".join(re.findall(r"`([^`]*)`", body(t))) if t and t in rec else ""


def decl(t, n):
    if t is None:
        return f"int {n}"
    if t < 0x1000:
        return f"{simple.get(t, 'int')} {n}".strip()
    k, name, _ = rec[t]
    b = body(t)
    if k == "LF_POINTER":
        r = ref(b, "referent")
        star = "&" if "mode = ref" in b else "*"
        if r in rec and rec[r][0] in ("LF_PROCEDURE", "LF_MFUNCTION"):
            rb = body(r)
            return decl(ref(rb, "return type"), f"(*{n})({args(ref(rb, 'param list'))})")
        if r in rec and rec[r][0] == "LF_ARRAY":
            return decl(r, f"({star}{n})")
        return decl(r, star + n)
    if k == "LF_MODIFIER":
        return ("const " if "const" in b else "") + ("volatile " if "volatile" in b else "") + decl(ref(b, "referent"), n)
    if k == "LF_ARRAY":
        e = ref(b, "element type")
        return decl(e, f"{n}[{sizeof(t) // max(sizeof(e), 1)}]")
    if k == "LF_BITFIELD":
        return decl(ref(b, "type"), n) + " : " + re.search(r"# bits = (\d+)", b).group(1)
    if k in ("LF_PROCEDURE", "LF_MFUNCTION"):
        return decl(ref(b, "return type"), f"{n}({args(ref(b, 'param list'))})")
    return f"{name} {n}"


def entries(fl):
    out = []
    for l in rec[fl][2]:
        if l.startswith("- "):
            out.append(l[2:])
        elif out:
            out[-1] += " " + l
    return out


def method(cls, name, mt, attrs):
    if name.startswith("__") or "compiler" in attrs:
        return None
    b = body(mt)
    short = cls.split("::")[-1]
    static = "this type = <no type>" in b and "static" in attrs
    sig = f"{name}({args(ref(b, 'param list'))})"
    if name not in (short, "~" + short):
        sig = decl(ref(b, "return type"), sig)
    pre = "static " if "static" in attrs else "virtual " if "virtual" in attrs else ""
    vo = re.search(r"vftable offset = (\d+)", attrs)
    return pre + sig + (" = 0" if "pure" in attrs else "") + ";" + (f"  // vt 0x{int(vo.group(1)):X}" if vo and SHOW else "")


def render(t):
    k, name, _ = rec[t]
    b = body(t)
    kw = {"LF_CLASS": "class", "LF_STRUCTURE": "struct", "LF_UNION": "union"}[k]
    fl = ref(b, "field list")
    bases, members, methods = [], [], []
    for e in entries(fl) if fl in rec else []:
        acc = re.search(r"attrs = (\w+)", e)
        acc = acc.group(1) if acc else "public"
        if e.startswith("LF_BCLASS"):
            bt = ref(e, "type")
            bases.append(f"{acc} {rec[bt][1] if bt in rec else hex(bt)}")
        elif e.startswith("LF_MEMBER"):
            n = re.search(r"name = `([^`]+)`", e).group(1)
            members.append((acc, f"{decl(ref(e, 'Type'), n)};", int(re.search(r"offset = (\d+)", e).group(1))))
        elif e.startswith("LF_STMEMBER"):
            n = re.search(r"name = `([^`]+)`", e).group(1)
            members.append((acc, f"static {decl(ref(e, 'type'), n)};", None))
        elif e.startswith("LF_ONEMETHOD"):
            n = re.search(r"name = `([^`]+)`", e).group(1)
            s = method(name, n, ref(e, "type"), e.split("type = ")[-1])
            if s:
                methods.append((acc, s))
        elif e.startswith("LF_METHOD"):
            n = re.search(r"name = `([^`]+)`", e).group(1)
            ol = ref(e, "overload list")
            for mm in rec[ol][2] if ol in rec else []:
                s = method(name, n, ref(mm, "type"), mm.split("type = ")[-1])
                if s:
                    methods.append((re.search(r"attrs = (\w+)", mm).group(1), s))
    ns, short = name.rsplit("::", 1) if "::" in name else ("", name)
    out = [f"{kw} {short}" + (" : " + ", ".join(bases) if bases else "") + f" {{  // sizeof 0x{sizeof(t):X}"]
    last = "private" if kw == "class" else "public"
    for acc, s in methods + [(a, s + (f"  // 0x{o:X}" if o is not None else "")) for a, s, o in members]:
        if acc != last:
            out.append(f"{acc}:")
            last = acc
        out.append("    " + s)
    out.append("};")
    if ns:
        out = [f"namespace {ns} {{"] + out + ["}"]
    return ns, "\n".join(out)


if SHOW:
    for n in SHOW:
        print(render(by_name[n])[1] if n in by_name else f"// {n}: not found", "\n")
    sys.exit()
headers = collections.defaultdict(list)
for dp, _, fs in os.walk(root):
    if "/.git" in dp or "/tools" in dp:
        continue
    for f in fs:
        if f.endswith((".h", ".hpp")):
            headers[os.path.splitext(f)[0].lower()].append(os.path.join(dp, f))

cls_cpp = collections.defaultdict(collections.Counter)
for path, fn in ranges(syms, lines).items():
    stem = os.path.splitext(path.split("\\")[-1])[0]
    if path.endswith((".cpp", ".c")):
        for n, _, _ in fn:
            if "::" in n:
                cls_cpp[n.rsplit("::", 1)[0]][stem] += 1

out = collections.defaultdict(list)
for name, t in sorted(by_name.items()):
    if "<" in name or "is nested" in body(t) or "unnamed" in name or name.startswith(("std::", "stdext::")):
        continue
    short = name.split("::")[-1].lower()
    cands = headers.get(short) or next((headers.get(s) for s, _ in cls_cpp[name].most_common(1)), None)
    if cands and len(cands) == 1:
        out[cands[0]].append(render(t)[1])

for h, defs in out.items():
    src = open(h).read()
    i = src.rindex("#endif")
    open(h, "w").write(src[:i].rstrip("\n") + "\n\n" + "\n\n".join(defs) + "\n\n" + src[i:])
print("headers", len(out), "classes", sum(map(len, out.values())), "of", len(by_name))

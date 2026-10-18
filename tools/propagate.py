"""Spread names from vtnames.py through a Thumb-2 MSVC ARM binary.

    python3 propagate.py <dll> <vtnames.json> <out.json>

Rules (repeated until nothing changes):
- a function that stores a vftable into an object is that class's ctor (or dtor, when a named __vecDelDtor calls it)
- a function that is only a branch (thunk) passes its name to its target, and the other way round
- a function that loads an exact "Class::Method" string is named after it
- a function that loads a source path string ("..\\source\\game\\Game.cpp") is tagged with that file
"""
import collections, json, re, struct, sys
import capstone, pefile

dll, vtjson, out = sys.argv[1:4]
pe = pefile.PE(dll)
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
u32 = lambda rva: struct.unpack_from("<I", img, rva)[0]

d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
starts = sorted({(u32(d.VirtualAddress + i) & ~1) for i in range(0, d.Size, 8)})
ends = starts[1:] + [starts[-1] + 0x400]

md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_THUMB)
calls, refs, only_branch = {}, {}, {}
for s, e in zip(starts, ends):
    code = bytes(img[s:min(e, s + 0x4000)])
    cl, rf, movw = [], [], {}
    ins = list(md.disasm(code, base + s))
    for i in ins:
        if i.mnemonic in ("bl", "blx") and i.op_str.startswith("#"):
            cl.append((int(i.op_str[1:], 16) & ~1) - base)
        elif i.mnemonic.startswith("ldr") and "[pc" in i.op_str:
            m = re.search(r"\[pc, #(-?0x[0-9a-f]+|-?\d+)\]", i.op_str)
            if m:
                a = ((i.address + 4) & ~3) + int(m.group(1), 0) - base
                if 0 <= a < len(img) - 4:
                    rf.append(u32(a) - base)
        elif i.mnemonic.startswith("movw"):
            r, v = i.op_str.split(", #")
            movw[r] = int(v, 0)
        elif i.mnemonic.startswith("movt"):
            r, v = i.op_str.split(", #")
            if r in movw:
                rf.append(((int(v, 0) << 16) | movw.pop(r)) - base)
    calls[s], refs[s] = cl, rf
    if ins and ins[0].mnemonic in ("b", "b.w") and ins[0].op_str.startswith("#"):
        only_branch[s] = (int(ins[0].op_str[1:], 16) & ~1) - base

vt = {int(k, 16) - base: v for k, v in json.load(open(vtjson)).items()}
names = dict(vt)
tags = {}

# vftable rva -> class, from RTTI (same walk as vtnames)
dwords = collections.defaultdict(list)
for sec in pe.sections:
    if sec.Name.startswith((b".rdata", b".data")):
        for rva in range(sec.VirtualAddress, sec.VirtualAddress + sec.Misc_VirtualSize - 3, 4):
            dwords[u32(rva)].append(rva)
vftable = {}
for m in re.finditer(rb"\.\?AV([A-Za-z0-9_@]+?)@@\0", img):
    cls = "::".join(reversed([p for p in m.group(1).decode().split("@") if p]))
    for ref in dwords.get(base + m.start() - 8, []):
        col = ref - 12
        if col >= 0 and u32(col) == 0:
            for vref in dwords.get(base + col, []):
                vftable[vref + 4] = cls


def cstr(rva):
    if not 0 <= rva < len(img):
        return None
    e = img.find(b"\0", rva, rva + 300)
    s = img[rva:e] if e > rva else b""
    return s.decode() if len(s) >= 4 and all(32 <= c < 127 for c in s) else None


qual = re.compile(r"^[A-Za-z_]\w*(::[A-Za-z_~]\w*)+$")
src = re.compile(r"\\(source|core|libs|framework)\\.*\.(cpp|c|cc|h|hpp)$", re.I)
for f, rf in refs.items():
    for r in rf:
        s = cstr(r)
        if not s:
            continue
        if src.search(s):
            tags.setdefault(f, s.split("\\")[-1])
        elif qual.match(s) and f not in names and "::" in s and len(s) < 120:
            names[f] = s

vec_dtor = {f for f, n in names.items() if n.endswith("::__vecDelDtor")}
rounds = 0
while True:
    rounds += 1
    new = {}
    for f in starts:
        if f in names:
            continue
        cls = next((vftable[r] for r in refs.get(f, []) if r in vftable), None)
        if cls:
            short = cls.split("::")[-1]
            dtor = any(f in calls.get(v, []) and names[v].startswith(cls + "::") for v in vec_dtor)
            new[f] = f"{cls}::{'~' if dtor else ''}{short}"
            continue
        t = only_branch.get(f)
        if t is not None and t in names:
            new[f] = names[t] + "_thunk"
    for f, t in only_branch.items():
        if f in names and t not in names and not names[f].endswith("_thunk"):
            new[t] = names[f]
    if not new:
        break
    names.update(new)

json.dump({f"0x{base + a:08X}": n for a, n in sorted(names.items())}, open(out, "w"), indent=0)
json.dump({f"0x{base + a:08X}": t for a, t in sorted(tags.items())}, open(out.replace(".json", "_files.json"), "w"), indent=0)
kinds = collections.Counter("ctor" if n.split("::")[-1] == n.split("::")[-2] else "dtor" if "::~" in n else
                            "thunk" if n.endswith("_thunk") else "string/vt" for n in names.values())
print(f"functions {len(starts)}  named {len(names)} (vtables {len(vt)})  file-tagged {len(tags)}  rounds {rounds}", dict(kinds))

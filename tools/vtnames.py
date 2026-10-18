"""Name functions in a Jet/Minion Rush MSVC binary from the v0.0.3 PDB, through virtual function tables.

    python3 vtnames.py <types.txt from `llvm-pdbutil dump -types`> <DespicableMeComponent.dll> <out.json>

PDB side: each class's slot table (intro virtuals carry their slot offset; overrides are matched by name, bases via
LF_BCLASS). Binary side: RTTI type name -> complete object locator -> primary vftable -> function addresses.
"""
import collections, json, re, struct, sys
import pefile

types_txt, dll, out = sys.argv[1:4]

# ---- PDB: field lists, classes, bases, virtual methods
fields = collections.defaultdict(list)  # fieldlist id -> [(kind, name, voff, attrs, base)]
classes = {}  # name -> fieldlist id (definitions only)
cur_fl = cur = None
for line in open(types_txt, errors="replace"):
    m = re.match(r"\s+0x([0-9A-F]+) \| (LF_\w+)", line)
    if m:
        tid, kind = int(m.group(1), 16), m.group(2)
        cur_fl = tid if kind == "LF_FIELDLIST" else None
        cur = (tid, kind)
        mc = re.search(r"LF_(?:CLASS|STRUCTURE) \[size = \d+\] `([^`]+)`", line)
        cur_name = mc.group(1) if mc else None
        continue
    if cur and cur[1] in ("LF_CLASS", "LF_STRUCTURE") and "field list:" in line:
        fl = re.search(r"field list: 0x([0-9A-F]+)", line)
        if fl:
            classes[cur_name] = int(fl.group(1), 16)
    if cur_fl is None:
        continue
    m = re.search(r"- LF_ONEMETHOD \[name = `([^`]+)`\]", line)
    if m:
        fields[cur_fl].append(["m", m.group(1), None, ""])
        continue
    m = re.search(r"- LF_METHOD \[name = `([^`]+)`, # overloads = \d+", line)
    if m:
        fields[cur_fl].append(["ml", m.group(1), None, ""])
        continue
    m = re.search(r"vftable offset = (-?\d+), attrs = (.*)", line)
    if m and fields[cur_fl]:
        f = fields[cur_fl][-1]
        if f[0] == "m":
            f[2], f[3] = int(m.group(1)), m.group(2)
        continue
    m = re.search(r"- LF_BCLASS\s*$", line)
    if m:
        fields[cur_fl].append(["b", None, None, ""])
        continue
    m = re.search(r"type = 0x([0-9A-F]+), offset = 0", line)
    if m and fields[cur_fl] and fields[cur_fl][-1][0] == "b" and fields[cur_fl][-1][1] is None:
        fields[cur_fl][-1][1] = int(m.group(1), 16)

# type id -> class name (for base references, which point at forward refs or definitions)
tid_name = {}
for line in open(types_txt, errors="replace"):
    m = re.match(r"\s+0x([0-9A-F]+) \| LF_(?:CLASS|STRUCTURE) \[size = \d+\] `([^`]+)`", line)
    if m:
        tid_name[int(m.group(1), 16)] = m.group(2)


def slots(name, seen=()):
    """slot index -> qualified name, following the primary base chain."""
    if name not in classes or name in seen:
        return {}
    fl = fields.get(classes[name], [])
    base = next((tid_name.get(f[1]) for f in fl if f[0] == "b" and f[1]), None)
    s = dict(slots(base, seen + (name,))) if base else {}
    by_method = {v.split("::")[-1]: k for k, v in s.items()}
    for kind, mname, voff, attrs in fl:
        if kind == "m" and voff is not None and voff >= 0 and "intro" in attrs:
            s[voff // 4] = f"{name}::{mname}"
        elif kind in ("m", "ml") and mname in by_method and "virtual" in (attrs or "virtual"):
            s[by_method[mname]] = f"{name}::{mname}"
    return s


# ---- binary: RTTI -> vftables
pe = pefile.PE(dll, fast_load=True)
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
text = next(s for s in pe.sections if s.Name.startswith(b".text"))
tlo, thi = text.VirtualAddress, text.VirtualAddress + text.Misc_VirtualSize
u32 = lambda rva: struct.unpack_from("<I", img, rva)[0]
dwords = collections.defaultdict(list)
for s in pe.sections:
    if s.Name.startswith((b".rdata", b".data")):
        for rva in range(s.VirtualAddress, s.VirtualAddress + s.Misc_VirtualSize - 3, 4):
            dwords[u32(rva)].append(rva)

names = {}
stats = collections.Counter()
for m in re.finditer(rb"\.\?AV([A-Za-z0-9_@]+?)@@\0", img):
    parts = [p for p in m.group(1).decode().split("@") if p]
    cls = "::".join(reversed(parts))
    td = base + m.start() - 8
    for ref in dwords.get(td, []):
        col = ref - 12
        if col < 0 or u32(col) != 0 or u32(col + 4) != 0:  # signature 0, primary (offset 0) only
            continue
        for vref in dwords.get(base + col, []):
            vt, i = vref + 4, 0
            sl = slots(cls)
            stats["vftables"] += 1
            while True:
                fn = u32(vt + 4 * i)
                if not (tlo <= fn - base < thi):
                    break
                addr = (fn & ~1) - base
                if i in sl and addr not in names:
                    names[addr] = sl[i]
                    stats["named"] += 1
                i += 1
json.dump({f"0x{base + a:08X}": n for a, n in sorted(names.items())}, open(out, "w"), indent=0)
print(dict(stats), "classes in pdb", len(classes))

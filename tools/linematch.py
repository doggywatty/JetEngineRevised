"""Name file-tagged functions from PDB line tables: an assert passes __FILE__ and __LINE__, so the PDB function in that
file whose line range covers the line is the match.

    python3 linematch.py <dll> <syms.txt> <lines.txt> <names.json> <out.json>
"""
import json, sys
import capstone, pefile

dll, syms, lines, nj, out = sys.argv[1:6]
from pdblines import ranges as pdb_ranges
ranges = {}
for f, r in pdb_ranges(syms, lines).items():
    ranges.setdefault(f.split("\\")[-1], []).extend(r)

pe = pefile.PE(dll)
base, img = pe.OPTIONAL_HEADER.ImageBase, pe.get_memory_mapped_image()
d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
fs = sorted({(int.from_bytes(img[d.VirtualAddress + i:d.VirtualAddress + i + 4], "little") & ~1) for i in range(0, d.Size, 8)})
end = dict(zip(fs, fs[1:] + [fs[-1] + 0x400]))
names = {int(k, 16) - base: v for k, v in json.load(open(nj)).items()}
tags = {int(k, 16) - base: v.lower() for k, v in json.load(open(nj.replace(".json", "_files.json"))).items()}
md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_THUMB)
hit = 0
for f, t in tags.items():
    if f in names or t not in ranges:
        continue
    imms = {int(v, 0) for i in md.disasm(bytes(img[f:min(end[f], f + 0x4000)]), base + f)
            if i.mnemonic.split(".")[0] in ("movs", "mov", "movw") and (v := i.op_str.split("#")[-1]) != i.op_str
            and v.lstrip("-").replace("x", "").isalnum() and 10 < int(v, 0) < 20000}
    c = {n for n, lo, hi in ranges[t] for L in imms if lo <= L <= hi}
    if len(c) == 1:
        names[f] = c.pop()
        hit += 1
json.dump({f"0x{base + a:08X}": n for a, n in sorted(names.items())}, open(out, "w"), indent=0)
print("files", len(ranges), "tagged", len(tags), "named by line", hit, "total", len(names))

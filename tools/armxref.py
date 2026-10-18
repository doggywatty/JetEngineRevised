"""Cross-reference index of the 0.0.3 .so: every bl/b target and every PIC (ldr [pc]; add pc) address.

    python3 tools/armxref.py SO build              # one-time scan -> SO.xref (pickle)
    python3 tools/armxref.py SO to 0x1234|symbol   # who references an address
    python3 tools/armxref.py SO str "text"         # who references a C string containing text
"""
import os, pickle, struct, sys
from collections import defaultdict
from capstone import Cs, CS_ARCH_ARM, CS_MODE_ARM
sys.path.insert(0, os.path.dirname(__file__))
import armdis


def build(lib, path):
    text = lib.elf.get_section_by_name(".text")
    base, data = text["sh_addr"], text.data()
    refs = defaultdict(list)
    md = Cs(CS_ARCH_ARM, CS_MODE_ARM)
    md.skipdata = True
    pool = {}  # reg -> (literal value, at)
    gotbase = lib.byname.get("_GLOBAL_OFFSET_TABLE_", (0xcfe914, 0))[0]
    got = lib.elf.get_section_by_name(".got")
    slot = {}
    symval = {n: v for n, (v, _) in lib.byname.items()}
    for off in range(got["sh_addr"], got["sh_addr"] + got["sh_size"], 4):
        if off in lib.got:
            v = symval.get(lib.got[off])
            if v:
                slot[off] = v & ~1
        else:
            v = lib.u32(off)
            if v:
                slot[off] = v
    for addr, size, mn, ops in md.disasm_lite(data, base):
        if mn in ("bl", "b", "blx") or (mn.startswith("b") and len(mn) <= 4 and ops.startswith("#")):
            if ops.startswith("#"):
                try:
                    refs[int(ops[1:], 16)].append(addr)
                except ValueError:
                    pass
            continue
        if mn.startswith("ldr") and "[pc, #" in ops:
            r = ops.split(",")[0]
            try:
                d = int(ops.split("#")[1].rstrip("]"), 16)
            except ValueError:
                continue
            v = lib.u32(addr + 8 + d)
            if v is not None:
                pool[r] = v
                if (addr + 8 + d) in lib.relocated:
                    refs[v].append(addr)
                g = (gotbase + v) & 0xffffffff
                if g in slot:  # GOT-relative offset: ldr rX,[rGOT,rOff] reaches the global in that slot
                    refs[slot[g]].append(addr)
            continue
        if mn == "add" and "pc" in ops:
            p = [x.strip() for x in ops.split(",")]
            if len(p) == 3 and p[1] == "pc" and p[2] in pool:
                refs[(pool[p[2]] + addr + 8) & 0xffffffff].append(addr)
            elif len(p) == 3 and p[2] == "pc" and p[1] in pool:
                refs[(pool[p[1]] + addr + 8) & 0xffffffff].append(addr)
    pickle.dump(dict(refs), open(path, "wb"))
    return dict(refs)


def main():
    so = sys.argv[1]
    lib = armdis.Lib(so)
    cache = so + ".xref"
    if sys.argv[2] == "build" or not os.path.exists(cache):
        refs = build(lib, cache)
        if sys.argv[2] == "build":
            print(len(refs), "targets")
            return
    else:
        refs = pickle.load(open(cache, "rb"))
    if sys.argv[2] == "to":
        a = sys.argv[3]
        addr = int(a, 16) if a.startswith("0x") else lib.byname[a][0] & ~1
        targets = [addr]
    else:
        # all string addresses containing text
        needle = sys.argv[3].encode()
        ro = lib.elf.get_section_by_name(".rodata")
        rb, rd = ro["sh_addr"], ro.data()
        targets, i = [], rd.find(needle)
        while i >= 0:
            s = rd.rfind(b"\0", 0, i) + 1
            targets.append(rb + s)
            i = rd.find(needle, i + 1)
        targets = sorted(set(targets))
    for t in targets:
        for at in refs.get(t, []):
            print("%08x <- %08x %s   %s" % (t, at, lib.name(at) or "", repr(lib.cstr(t)) if lib.cstr(t) else ""))


if __name__ == "__main__":
    main()

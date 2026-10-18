"""Annotated ARM disassembly of the 0.0.3 libdespicablemefree.so, by symbol name or address.

    python3 tools/armdis.py SO _ZNK3jet5scene7SubMesh12LoadGeometryEv
    python3 tools/armdis.py SO 0x707de0 [length]
    python3 tools/armdis.py SO -s SubMesh          # list symbols matching a substring
    python3 tools/armdis.py SO -r 0x653800 0x655700   # list the symbols of an address range

Branch targets, PLT stubs (imports), literal-pool words, GOT-relative PIC address math
(ldr rX,[pc,#];  add rX,pc,rX) and C strings are resolved to names inline.
"""
import bisect, struct, subprocess, sys
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
from capstone import Cs, CS_ARCH_ARM, CS_MODE_ARM, CS_MODE_THUMB
from capstone.arm import ARM_OP_IMM, ARM_OP_MEM, ARM_REG_PC


class Lib:
    def __init__(self, path):
        self.f = open(path, "rb")
        self.elf = ELFFile(self.f)
        self.segs = [(s["p_vaddr"], s["p_filesz"], s["p_offset"]) for s in self.elf.iter_segments() if s["p_type"] == "PT_LOAD"]
        self.syms = {}
        self.byname = {}
        for secname in (".dynsym", ".symtab"):
            sec = self.elf.get_section_by_name(secname)
            if not sec:
                continue
            for s in sec.iter_symbols():
                v = s["st_value"]
                if v and s.name:
                    self.syms.setdefault(v & ~1, s.name)
                    self.byname[s.name] = (v, s["st_size"])
        self.got = {}
        self.relocated = set()
        for sec in self.elf.iter_sections():
            if isinstance(sec, RelocationSection):
                symtab = self.elf.get_section(sec["sh_link"])
                for r in sec.iter_relocations():
                    self.relocated.add(r["r_offset"])
                    if r["r_info_sym"]:
                        self.got[r["r_offset"]] = symtab.get_symbol(r["r_info_sym"]).name
        self.plt = {}
        plt = self.elf.get_section_by_name(".plt")
        if plt:
            a, data = plt["sh_addr"], plt.data()
            md = Cs(CS_ARCH_ARM, CS_MODE_ARM)
            md.detail = True
            ins = list(md.disasm(data, a))
            for i in range(len(ins) - 2):
                # add ip,pc,#a ; add ip,ip,#b ; ldr pc,[ip,#c]!
                x, y, z = ins[i], ins[i + 1], ins[i + 2]
                if x.mnemonic == "add" and y.mnemonic == "add" and z.mnemonic == "ldr" and "ip" in x.op_str:
                    try:
                        g = x.address + 8 + armimm(x) + armimm(y) + z.operands[1].mem.disp
                        if g in self.got:
                            self.plt[x.address] = self.got[g]
                    except Exception:
                        pass
        self.addrs = sorted(self.syms)
        gotsec = self.elf.get_section_by_name(".got")
        self.gotbase = gotsec["sh_addr"] if gotsec else 0

    def read(self, addr, n):
        for va, sz, off in self.segs:
            if va <= addr < va + sz:
                self.f.seek(off + addr - va)
                return self.f.read(min(n, va + sz - addr))
        return b""

    def u32(self, addr):
        b = self.read(addr, 4)
        return struct.unpack("<I", b)[0] if len(b) == 4 else None

    def cstr(self, addr):
        b = self.read(addr, 200)
        if not b:
            return None
        e = b.find(b"\0")
        s = b[: e if e >= 0 else 80]
        if len(s) >= 2 and all(32 <= c < 127 or c in (9, 10) for c in s):
            return s.decode()
        return None

    def name(self, addr):
        if addr in self.plt:
            return "PLT:" + demangle(self.plt[addr])
        if addr in self.got:
            return "GOT:" + demangle(self.got[addr])
        i = bisect.bisect_right(self.addrs, addr) - 1
        if i >= 0:
            b = self.addrs[i]
            n = self.syms[b]
            size = self.byname.get(n, (0, 0))[1]
            if addr == b:
                return demangle(n)
            if addr - b < max(size, 1) or addr - b < 0x40:
                return "%s+0x%x" % (demangle(n), addr - b)
        return None

    def describe(self, v):
        n = self.name(v)
        s = self.cstr(v)
        out = []
        if n:
            out.append(n)
        if s:
            out.append(repr(s))
        return " ".join(out)


def armimm(i):
    # data-processing immediate straight from the encoding (capstone splits some as "#imm, #rot")
    w = struct.unpack("<I", bytes(i.bytes))[0]
    v, r = w & 0xff, ((w >> 8) & 0xf) * 2
    return ((v >> r) | (v << (32 - r))) & 0xffffffff if r else v


_dm = {}


def demangle(n):
    if n not in _dm:
        try:
            _dm[n] = subprocess.run(["c++filt", n], capture_output=True, text=True).stdout.strip() or n
        except Exception:
            _dm[n] = n
    return _dm[n]


def dis(lib, addr, length=None, thumb=False):
    if length is None:
        n = lib.syms.get(addr)
        length = lib.byname.get(n, (0, 0x200))[1] if n else 0x200
        length = length or 0x200
    code = lib.read(addr, length)
    md = Cs(CS_ARCH_ARM, CS_MODE_THUMB if thumb else CS_MODE_ARM)
    md.detail = True
    md.skipdata = True  # literal pool words in the middle of a function do not end the listing
    regs = {}  # reg -> literal value loaded from pool
    lines = []
    for i in md.disasm(code, addr):
        note = ""
        if i.id == 0:  # skipped data word
            lines.append("%08x  .word    0x%08x" % (i.address, int.from_bytes(i.bytes, "little")))
            continue
        ops = i.operands
        if i.mnemonic.startswith("b") and ops and ops[-1].type == ARM_OP_IMM and i.mnemonic not in ("bic", "bics"):
            note = lib.name(ops[-1].imm) or ""
        elif i.mnemonic.startswith("ldr") and len(ops) >= 2 and ops[1].type == ARM_OP_MEM and ops[1].mem.base == ARM_REG_PC and ops[1].mem.index == 0:
            la = i.address + 8 + ops[1].mem.disp
            v = lib.u32(la)
            if v is not None:
                regs[ops[0].reg] = (v, i.address)
                # an absolute address only if the pool slot is relocated; else a PC/GOT-relative offset
                d = (lib.got.get(la) and "&" + demangle(lib.got[la])) or (lib.describe(v) if la in lib.relocated else "")
                note = "=0x%x %s" % (v, d)
        elif i.mnemonic == "add" and len(ops) == 3 and ops[1].type != ARM_OP_IMM and ops[1].reg == ARM_REG_PC and ops[2].reg in regs:
            v = (regs[ops[2].reg][0] + i.address + 8) & 0xffffffff
            note = "-> 0x%x %s" % (v, lib.describe(v))
            regs[ops[0].reg] = (v, i.address)
        elif i.mnemonic == "add" and len(ops) == 3 and ops[2].type != ARM_OP_IMM and ops[2].reg == ARM_REG_PC and ops[1].reg in regs:
            v = (regs[ops[1].reg][0] + i.address + 8) & 0xffffffff
            note = "-> 0x%x %s" % (v, lib.describe(v))
            regs[ops[0].reg] = (v, i.address)
        elif i.mnemonic == "ldr" and len(ops) == 2 and ops[1].type == ARM_OP_MEM and ops[1].mem.index and ops[1].mem.base in regs and ops[1].mem.index in regs:
            # ldr rD, [rGotBase, rOffset]: a GOT slot (the pool word is the signed offset from the GOT)
            gb, off = regs[ops[1].mem.base][0], regs[ops[1].mem.index][0]
            if gb == lib.byname.get("_GLOBAL_OFFSET_TABLE_", (0xcfe914, 0))[0]:
                slot = (gb + (off - (1 << 32) if off >= 1 << 31 else off)) & 0xffffffff
                tgt = lib.got.get(slot)
                v = lib.u32(slot)
                note = "GOT[0x%x] = %s" % (slot, ("&" + demangle(tgt)) if tgt else (lib.describe(v) if v else "?"))
        lines.append("%08x  %-8s %-34s %s" % (i.address, i.mnemonic, i.op_str, ("; " + note) if note else ""))
    # data words after the code (literal pool) are reported by capstone as garbage; that's fine
    return "\n".join(lines)


def main():
    lib = Lib(sys.argv[1])
    a = sys.argv[2]
    if a == "-s":
        pat = sys.argv[3]
        for n, (v, sz) in sorted(lib.byname.items(), key=lambda x: x[1]):
            dn = demangle(n) if pat.lower() in n.lower() else ""
            if dn:
                print("%08x %5d %s" % (v, sz, dn))
        return
    if a == "-r":
        # every symbol in an address range: armdis SO -r 0x653800 0x655700
        lo, hi = int(sys.argv[3], 16), int(sys.argv[4], 16)
        for n, (v, sz) in sorted(lib.byname.items(), key=lambda x: x[1]):
            if lo <= v < hi:
                print("%08x %5d %s" % (v, sz, demangle(n)))
        return
    if a.startswith("0x"):
        addr = int(a, 16)
    else:
        addr = lib.byname[a][0]
    thumb = bool(addr & 1)
    addr &= ~1
    length = int(sys.argv[3], 0) if len(sys.argv) > 3 else None
    print("; %s" % (lib.name(addr) or hex(addr)))
    print(dis(lib, addr, length, thumb))


if __name__ == "__main__":
    main()

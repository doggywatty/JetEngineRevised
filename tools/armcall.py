"""Call functions of the original ARM .so in an emulator (unicorn), to diff them against the rewrite.

    from armcall import Arm
    arm = Arm("libdespicablemefree.so")
    out = arm.alloc(16)
    arm.call("_ZN4math4quatIfEC1ENS1_6ZAxisZEf", out, 0, arm.f2i(1.0))   # softfp: floats go in core registers
    arm.floats(out, 4)
"""
import math, struct
import lief
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE, UcError
from unicorn.unicorn_const import *
from unicorn.arm_const import *

PAGE = 0x1000
STUB, HEAP, STACK, RET = 0x20000000, 0x30000000, 0x40000000, 0x50000000


def f2i(f):
    return struct.unpack("<I", struct.pack("<f", f))[0]


def i2f(i):
    return struct.unpack("<f", struct.pack("<I", i & 0xffffffff))[0]


def d2i(d):
    return struct.unpack("<II", struct.pack("<d", d))


def i2d(lo, hi):
    return struct.unpack("<d", struct.pack("<II", lo & 0xffffffff, hi & 0xffffffff))[0]


def _f1(fn):
    return lambda a: f2i(fn(i2f(a.r(0))))


def _f2(fn):
    return lambda a: f2i(fn(i2f(a.r(0)), i2f(a.r(1))))


def _d1(fn):
    def g(a):
        lo, hi = d2i(fn(i2d(a.r(0), a.r(1))))
        a.w(1, hi)
        return lo
    return g


def _atan2f(y, x):
    return math.atan2(y, x)


LIBM = {
    "sinf": _f1(math.sin), "cosf": _f1(math.cos), "tanf": _f1(math.tan), "sqrtf": _f1(lambda x: math.sqrt(x) if x >= 0 else float("nan")),
    "acosf": _f1(lambda x: math.acos(max(-1.0, min(1.0, x)))), "asinf": _f1(lambda x: math.asin(max(-1.0, min(1.0, x)))),
    "atanf": _f1(math.atan), "atan2f": _f2(_atan2f), "floorf": _f1(math.floor), "ceilf": _f1(math.ceil),
    "fmodf": _f2(math.fmod), "powf": _f2(math.pow), "expf": _f1(math.exp), "logf": _f1(lambda x: math.log(x) if x > 0 else float("-inf")),
    "fabsf": _f1(abs), "sin": _d1(math.sin), "cos": _d1(math.cos), "sqrt": _d1(math.sqrt), "acos": _d1(math.acos),
    "floor": _d1(math.floor), "ceil": _d1(math.ceil),
}


class Arm:
    def __init__(self, so):
        self.elf = lief.parse(so)
        self.uc = Uc(UC_ARCH_ARM, UC_MODE_ARM)
        self.uc.ctl_set_cpu_model(UC_CPU_ARM_CORTEX_A15)
        self.uc.reg_write(UC_ARM_REG_C1_C0_2, self.uc.reg_read(UC_ARM_REG_C1_C0_2) | (0xf << 20))
        self.uc.reg_write(UC_ARM_REG_FPEXC, 0x40000000)
        self.syms = {}
        for s in self.elf.symbols:
            if s.value and s.name:
                self.syms.setdefault(s.name, s.value)
        for seg in self.elf.segments:
            if seg.type == lief.ELF.Segment.TYPE.LOAD:
                lo = seg.virtual_address & ~(PAGE - 1)
                hi = (seg.virtual_address + seg.virtual_size + PAGE - 1) & ~(PAGE - 1)
                self.uc.mem_map(lo, hi - lo)
                self.uc.mem_write(seg.virtual_address, bytes(seg.content))
        self.uc.mem_map(STUB, 0x100000)
        self.uc.mem_map(HEAP, 0x4000000)
        self.uc.mem_map(STACK - 0x100000, 0x100000)
        self.uc.mem_map(RET, PAGE)
        self.heap = HEAP
        self.stubs = {}
        for r in self.elf.relocations:
            t = int(r.type) & 0xff
            if t == 23:  # R_ARM_RELATIVE: load base is 0, value already right
                continue
            name = r.symbol.name if r.has_symbol else ""
            if name in self.syms and self.syms[name]:
                val = self.syms[name]
            else:
                val = self.stub(name)
            if t in (2, 21, 22):  # ABS32, GLOB_DAT, JUMP_SLOT
                add = struct.unpack("<I", self.uc.mem_read(r.address, 4))[0] if t == 2 else 0
                self.uc.mem_write(r.address, struct.pack("<I", (val + add) & 0xffffffff))
        self.uc.hook_add(UC_HOOK_CODE, self._hook, begin=STUB, end=STUB + 0x100000)

    def stub(self, name):
        if name not in self.stubs:
            addr = STUB + 8 * len(self.stubs)
            self.stubs[name] = addr
            self.uc.mem_write(addr, b"\x1e\xff\x2f\xe1")  # bx lr
        return self.stubs[name]

    def _hook(self, uc, addr, size, _):
        name = next((n for n, a in self.stubs.items() if a == addr), None)
        fn = LIBM.get(name) or getattr(self, "_x_" + (name or ""), None)
        if fn is None:
            raise RuntimeError(f"unimplemented import {name}")
        v = fn(self)
        if v is not None:
            self.w(0, v)

    def _x_memcpy(self):
        d, s, n = self.r(0), self.r(1), self.r(2)
        self.uc.mem_write(d, bytes(self.uc.mem_read(s, n)))
        return d

    _x_memmove = _x_memcpy

    def _x_memset(self):
        d, c, n = self.r(0), self.r(1), self.r(2)
        self.uc.mem_write(d, bytes([c & 0xff]) * n)
        return d

    def _x_malloc(self):
        return self.alloc(self.r(0))

    def _x_calloc(self):
        return self.alloc(self.r(0) * self.r(1))

    def _x_free(self):
        return None

    _x__Znwj = _x_malloc
    _x__Znaj = _x_malloc
    _x__ZdlPv = _x_free
    _x__ZdaPv = _x_free

    def _x_strlen(self):
        p, n = self.r(0), 0
        while self.uc.mem_read(p + n, 1)[0]:
            n += 1
        return n

    def r(self, i):
        return self.uc.reg_read(UC_ARM_REG_R0 + i)

    def w(self, i, v):
        self.uc.reg_write(UC_ARM_REG_R0 + i, v & 0xffffffff)

    f2i, i2f = staticmethod(f2i), staticmethod(i2f)

    def alloc(self, n, data=None):
        p = self.heap
        self.heap += (n + 15) & ~15
        self.uc.mem_write(p, data if data is not None else bytes(n))
        return p

    def floats(self, p, n):
        return list(struct.unpack(f"<{n}f", self.uc.mem_read(p, 4 * n)))

    def put_floats(self, vals):
        return self.alloc(4 * len(vals), struct.pack(f"<{len(vals)}f", *vals))

    def call(self, sym, *args):
        addr = self.syms[sym] if isinstance(sym, str) else sym
        sp = STACK - 0x100
        regs = [a & 0xffffffff for a in args]
        stack = regs[4:]
        if stack:
            sp -= 4 * len(stack)
            self.uc.mem_write(sp, struct.pack(f"<{len(stack)}I", *stack))
        for i, v in enumerate(regs[:4]):
            self.w(i, v)
        self.uc.reg_write(UC_ARM_REG_SP, sp)
        self.uc.reg_write(UC_ARM_REG_LR, RET)
        self.uc.emu_start(addr, RET, count=5_000_000)
        return self.r(0)

"""gui_1_1 (Aurora GUI levels) parser: AuroraLevelData::Load @0x505fc8, LoadElementNames @0x5064c0."""
import struct, sys
TYPES = ['Graph','Area','FillRect','Button','Text','List','ListButtonScroll','Slide','PBar','Grid',
         'DeviceSpecificAnim','ScrollBar','3DElement','3DBodyPart','3DCostumePos','3DTouchInteract']
PARENT = [12, 11, 15, 13, 17, 17, 17, 13, 15, 18, 11, 12, 10, 11, 11, 10]
strings = []; smap = {}

def archive(path):
    d = open(path, 'rb').read()
    n, = struct.unpack_from('<I', d, 0)
    offs = struct.unpack_from('<%dI' % n, d, 4)
    p = 4 + 4 * n; names = []
    for i in range(n):
        l, = struct.unpack_from('<H', d, p); names.append(d[p+2:p+2+l].decode()); p += 2 + l
    ends = sorted(offs) + [len(d)]
    out = {}
    for nm, o in zip(names, offs):
        e = ends[ends.index(o) + 1]
        out[nm] = d[o:e]
    return out

def level(buf):
    elems = []; p = 0
    while p < len(buf):
        if buf[p] not in (4, 5, 8): break          # archive padding
        cnt = struct.unpack_from('<h', buf, p + 0x19)[0] + 9
        e = []
        for _ in range(cnt):
            t = buf[p]
            if t in (4, 5): e.append(struct.unpack_from('<h', buf, p + 1)[0]); p += 3
            elif t == 8:
                l = struct.unpack_from('b', buf, p + 1)[0]; p += 2
                if l > 0:
                    s = buf[p:p+l].decode('latin1'); p += l
                    if s not in smap: smap[s] = len(strings); strings.append(s)
                    e.append(smap[s])
                else: e.append(-1)
            else: raise ValueError('record type %d at %d' % (t, p))
        elems.append(e)
    return elems

def names(path):
    d = open(path, 'rb').read(); p = 4; out = []
    n, = struct.unpack_from('<I', d, 0)
    def rs():
        nonlocal p
        l, = struct.unpack_from('<H', d, p); s = d[p+2:p+2+l].decode('latin1'); p += 2 + l; return s
    for _ in range(n):
        lv = rs(); c, = struct.unpack_from('<i', d, p); p += 4
        out.append((lv, [rs() for _ in range(c)]))
    assert p == len(d), (p, len(d))
    return out

if __name__ == '__main__':
    root = sys.argv[1]
    lv = archive(root + '/gui_1_1')
    nm = names(root + '/gui_1_1_str')
    print(len(lv), 'levels', len(nm), 'name sets')
    for i, (k, v) in enumerate(sorted(lv.items(), key=lambda kv: int(kv[0][5:-7]))):
        els = level(v)
        print(k, len(els), 'elements', nm[i][0] if i < len(nm) else '?', len(nm[i][1]) if i < len(nm) else '')
    want = sys.argv[2] if len(sys.argv) > 2 else None
    if want:
        idx = [n for n, _ in nm].index(want)
        els = level(lv['level%d.layers' % (idx + 1)])
        en = nm[idx][1]
        for j, e in enumerate(els):
            t = e[0]; par = e[PARENT[t]] if t < 16 else None
            print('%3d %-18s id=%-3d pos=(%d,%d) anc=%s par=%s %-30s %s' % (j, TYPES[t], e[1], e[2], e[3], e[4:8], par, en[j] if j < len(en) else '?',
                  [strings[x] if (k >= 9 and 0 <= x < len(strings) and False) else x for k, x in enumerate(e[9:])]))

import struct, sys, os
class R:
    def __init__(s,b): s.b=b; s.p=0
    def u8(s): v=s.b[s.p]; s.p+=1; return v
    def u16(s): v=struct.unpack_from('<H',s.b,s.p)[0]; s.p+=2; return v
    def s16(s): v=struct.unpack_from('<h',s.b,s.p)[0]; s.p+=2; return v
    def i32(s): v=struct.unpack_from('<i',s.b,s.p)[0]; s.p+=4; return v
    def name(s): n=s.u8(); v=s.b[s.p:s.p+n].decode('latin1'); s.p+=n; return v
def load(path):
    r=R(open(path,'rb').read()); sp={}
    sp['version']=r.u16(); f=r.i32(); f2=0
    if f & 0x400000: f2=r.i32()
    sp['flags']=f; sp['flags2']=f2; sp['images']=[]
    if f2 & 0x400:
        for _ in range(r.s16()):
            im={'name':r.name()}
            if f2 & 0x10000: im['size']=(r.s16(), r.s16())
            sp['images'].append(im)
    sp['modules']=[]
    n=r.u16()
    for _ in range(n):
        m={'type':r.u8(),'x':r.u16(),'y':r.u16()}
        if m['type']!=0xfd: m['w']=r.u16(); m['h']=r.u16()
        if f2 & 0x800: m['name']=r.name()
        sp['modules'].append(m)
    sp['fmodules']=[]
    for _ in range(r.u16()):
        fm={'module':r.u16(),'x':r.s16(),'y':r.s16(),'flags':r.u8()}
        if f2 & 1:
            fm['rot']=r.s16(); fm['sx']=r.s16(); fm['sy']=r.s16()
            if f2 & 0x2000: fm['ex']=(r.s16(), r.s16())
        sp['fmodules'].append(fm)
    sp['rects']=[]
    if f & 0x8000:
        for _ in range(r.u16()): sp['rects'].append((r.s16(),r.s16(),r.u16(),r.u16()))
    sp['frames']=[]
    nf=r.u16()
    for _ in range(nf):
        fr={'count': r.u16() if f & 0x100000 else r.u8(), 'first': r.u16()}
        if f & 0x8000: fr['rects']=r.u8()
        if f2 & 0x80: fr['name']=r.name()
        sp['frames'].append(fr)
    if nf and not (f & 0x1000): r.p += nf*8
    sp['aframes']=[]
    for _ in range(r.u16()):
        af={'frame': r.u16() if f & 0x200000 else r.u8(), 'time':r.u8(), 'x':r.s16(),'y':r.s16(),'flags':r.u8()}
        if f2 & 2:
            af['rot']=r.s16(); af['sx']=r.s16(); af['sy']=r.s16()
            if f2 & 0x2000: af['ex']=(r.s16(),r.s16())
        sp['aframes'].append(af)
    sp['anims']=[]
    for _ in range(r.u16()):
        a={'count': r.u16() if f & 0x80000 else r.u8(), 'first': r.u16()}
        if f2 & 0x80: a['name']=r.name()
        sp['anims'].append(a)
    sp['end']=r.p; sp['size']=len(r.b)
    return sp
if __name__=='__main__':
    d=sys.argv[1]; bad=0
    for fn in sorted(os.listdir(d)):
        try:
            s=load(os.path.join(d,fn))
            ok=s['end']==s['size']
            if not ok: bad+=1
            print(('OK ' if ok else 'BAD'), fn, hex(s['flags']), hex(s['flags2']), 'img',[i['name'] for i in s['images']], 'mod',len(s['modules']),'fm',len(s['fmodules']),'fr',len(s['frames']),'af',len(s['aframes']),'an',len(s['anims']), s['end'], s['size'])
        except Exception as e: bad+=1; print('FAIL', fn, e)
    print('bad', bad)

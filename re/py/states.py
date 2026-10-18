import struct, sys
class R:
    def __init__(s,b): s.b=b; s.p=0
    def u8(s): v=s.b[s.p]; s.p+=1; return v
    def u16(s): v=struct.unpack_from('<H',s.b,s.p)[0]; s.p+=2; return v
    def s16(s): v=struct.unpack_from('<h',s.b,s.p)[0]; s.p+=2; return v
    def i32(s): v=struct.unpack_from('<i',s.b,s.p)[0]; s.p+=4; return v
    def f32(s): v=struct.unpack_from('<f',s.b,s.p)[0]; s.p+=4; return v
    def str(s): n=s.u16(); v=s.b[s.p:s.p+n].decode('latin1'); s.p+=n; return v
def trigger(r):
    t={'i':[r.i32() for _ in range(6)]}
    if r.u8(): t['name']=r.str()
    t['cond']=[(r.str(),r.i32()) for _ in range(r.u16())]
    return t
def track(r):
    t={'name':r.str(),'events':[]}
    for _ in range(r.u16()):
        e={'a':r.i32(),'b':r.i32()}; n=r.i32(); e['args']=[r.i32() for _ in range(n)]
        if r.u8(): e['str']=r.str()
        t['events'].append(e)
    t['end']=r.i32(); return t
def state(r):
    s={'name':r.str()}
    sel=r.u8(); s['selector']=sel
    if sel:
        s['entries']=[(r.str(),r.i32(),r.f32(),r.i32()) for _ in range(r.u16())]
        s['params']=[(r.str(),r.i32()) for _ in range(r.u16())]
        s['triggers']=[trigger(r) for _ in range(r.u16())]
        return s
    s['params']=[(r.str(),r.i32()) for _ in range(r.u16())]
    s['triggers']=[trigger(r) for _ in range(r.u16())]
    s['anims']=[]
    for _ in range(r.u16()):
        a={'name':r.str(),'v':r.s16()}; a['tracks']=[track(r) for _ in range(r.u16())]; s['anims'].append(a)
    s['animsets']=[(r.str(),r.s16(),r.s16()) for _ in range(r.u16())]
    return s
def load(path):
    r=R(open(path,'rb').read()); sets=[]
    for _ in range(r.u16()):
        ss={'name':r.str()}
        ss['params']=[]
        for _ in range(r.u16()): ss['params'].append((r.i32(), r.str()))
        ss['states']=[state(r) for _ in range(r.u16())]
        sets.append(ss)
    return sets, r.p, len(r.b)
if __name__=='__main__':
    sets,end,size=load(sys.argv[1]); print('end',end,'size',size,'sets',[ (s['name'],len(s['states'])) for s in sets])

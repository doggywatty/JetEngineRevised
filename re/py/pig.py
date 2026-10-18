import struct, sys, os, math
class R:
    def __init__(s, b): s.b=b; s.p=0
    def u8(s): v=s.b[s.p]; s.p+=1; return v
    def s8(s): v=struct.unpack_from('<b',s.b,s.p)[0]; s.p+=1; return v
    def u16(s): v=struct.unpack_from('<H',s.b,s.p)[0]; s.p+=2; return v
    def s16(s): v=struct.unpack_from('<h',s.b,s.p)[0]; s.p+=2; return v
    def u32(s): v=struct.unpack_from('<I',s.b,s.p)[0]; s.p+=4; return v
    def i32(s): v=struct.unpack_from('<i',s.b,s.p)[0]; s.p+=4; return v
    def f(s): v=struct.unpack_from('<f',s.b,s.p)[0]; s.p+=4; return v
    def v3(s): return (s.f(),s.f(),s.f())
    def raw(s,n): v=s.b[s.p:s.p+n]; s.p+=n; return v
    def str(s):
        n=s.u16(); return s.raw(n).decode('latin1')

def node(r):
    ver=r.i32(); assert ver==100, ver
    n=dict(name=r.str(), flags=r.s8()&0xff, parent=r.s16(), pos=r.v3(), rot=struct.unpack('<4f',r.raw(16)), scale=r.v3(), f34=r.f())
    k=r.u16(); n['props']=[(r.str(),r.str()) for _ in range(k)]
    return n

def submesh(r, skinned, verbose):
    ver=r.i32(); assert ver==100, ('submesh ver',ver)
    sm={}
    sm['fmt']=r.u32(); sm['u0c']=r.u32(); sm['center']=r.v3()
    if sm['fmt']&3: sm['qoff']=r.v3(); sm['qscale']=r.v3()
    sm['u44']=r.u16(); sm['u48']=r.u32()
    sm['mat']=r.str(); nt=r.u16(); sm['tex']=[r.str() for _ in range(nt)]
    sm['u8']=r.u8(); sm['size']=r.u32(); sm['off']=r.p; r.p+=sm['size']
    if skinned:
        sm['s6c']=r.u16(); sm['bones']=r.u16(); sz=r.u32(); sm['skinsize']=sz; r.p+=sz
    return sm

def mesh(r, verbose):
    ver=r.i32(); assert ver==100, ('mesh ver',ver)
    skinned=r.u8(); b=r.u8(); aabb=struct.unpack('<6f',r.raw(24)); n=r.u16()
    return dict(skinned=skinned, b=b, aabb=aabb, subs=[submesh(r,skinned,verbose) for _ in range(n)])

def pig(path, verbose=False):
    r=R(open(path,'rb').read())
    ver=r.i32(); assert ver==100
    nodes=[node(r) for _ in range(r.u16())]
    u8a0=r.u8()
    mrm=[]
    for _ in range(r.u16()):
        ver=r.i32(); assert ver==100
        m=dict(u=r.u32(), lods=[])
        for _ in range(r.u16()):
            lod=r.u8(); m['lods'].append((lod, mesh(r,verbose)))
        mrm.append(m)
    cams=[]
    for _ in range(r.u16()):
        c=node(r); c['target']=r.str(); fovh=r.f(); c['aspect']=r.f(); c['near']=r.f(); c['far']=r.f(); c['ortho']=r.u8()
        c['fovh']=fovh; c['fov']=2*math.atan(math.tan(fovh*math.pi/360)/c['aspect']); cams.append(c)
    ud=[]
    if r.p < len(r.b):
        ud=[(r.str(),r.str()) for _ in range(r.u16())]
    return dict(nodes=nodes, a0=u8a0, mrm=mrm, cams=cams, ud=ud, end=r.p, size=len(r.b))

if __name__=='__main__':
    bad=0
    for p in sys.argv[1:]:
        try:
            m=pig(p)
            if m['end']!=m['size']: print('LEFTOVER',p,m['end'],m['size']); bad+=1
        except Exception as e:
            print('FAIL',p,repr(e)); bad+=1
    print('bad',bad,'of',len(sys.argv)-1)

import struct as _st
def decode(path, sm, skinned=False, hw=False, weights=0):
    b=open(path,'rb').read(); p=sm['off']
    hl=_st.unpack_from('<H',b,p)[0]; p+=2+hl
    fmt=sm['fmt']; mask=sm['u0c']; n=sm['u44']; out={}
    def chan(sz):
        nonlocal p
        d=b[p:p+sz*n]; p+=sz*n; pad=_st.unpack_from('<H',b,p)[0]; p+=2+pad; return d
    q=fmt&3
    if mask&1:
        d=chan({0:12,1:8,2:4}[q])
        if q==0: out['pos']=[_st.unpack_from('<3f',d,i*12) for i in range(n)]
        elif q==1: out['pos']=[tuple(sm['qoff'][k]+sm['qscale'][k]*max(-1,v/32767) for k,v in enumerate(_st.unpack_from('<4h',d,i*8)[:3])) for i in range(n)]
        else: out['pos']=[tuple(sm['qoff'][k]+sm['qscale'][k]*max(-1,v/127) for k,v in enumerate(_st.unpack_from('<4b',d,i*4)[:3])) for i in range(n)]
    if mask&2: out['nrm']=chan({0:12,1:8,2:4}[(fmt>>2)&3])
    for c in range(2,6):
        if mask&(1<<c): chan({0:16,1:8,2:4}[(fmt>>4)&3])
    if mask&0x40: out['col']=chan(4)
    for t in range(4):
        if mask&(1<<(7+t)):
            qz=(fmt>>(6+t))&1
            d=chan(4 if qz else 8)
            if t==0: out['uv0']=(qz,d)
    if skinned:
        chan(4)  # node indices
        if not hw or weights>1: chan(4)  # weights
    out['idx']=_st.unpack_from('<%dH'%sm['u48'],b,p); p+=2*sm['u48']
    return out, p

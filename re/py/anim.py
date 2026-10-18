import struct, sys, os
class R:
    def __init__(s,b): s.b=b; s.p=0
    def u8(s): v=s.b[s.p]; s.p+=1; return v
    def s16(s): v=struct.unpack_from('<h',s.b,s.p)[0]; s.p+=2; return v
    def u16(s): v=struct.unpack_from('<H',s.b,s.p)[0]; s.p+=2; return v
    def u32(s): v=struct.unpack_from('<I',s.b,s.p)[0]; s.p+=4; return v
    def str(s): n=s.u16(); v=s.b[s.p:s.p+n].decode('latin1'); s.p+=n; return v
    def raw(s,n): v=s.b[s.p:s.p+n]; s.p+=n; return v
def load(path):
    r=R(open(path,'rb').read())
    a={'fps':r.u32()}
    nn=r.s16(); nodes=[{'name':r.str(),'flags':r.u8()} for _ in range(nn)]
    nm=r.s16(); mats=[]
    for _ in range(nm): mats.append({'name':r.str(),'ch':r.s16()})
    dur=0
    def seq(keysize, initsize):
        nonlocal dur
        n=r.u16()
        if n:
            t=struct.unpack_from('<%dH'%n,r.raw(2*n))
            dur=max(dur,(t[-1]&0x7fff)<<3); r.raw(keysize*n); return n
        r.raw(initsize); return 0
    for n in nodes: n['pos']=seq(12,12)
    for n in nodes:
        rs=6 if n['flags']&2 else 8
        n['rot']=seq(rs,rs)
    for n in nodes: n['scale']=seq(12,12)
    for m in mats:
        for c in range(m['ch']): seq(8,8)
        for c in range(m['ch']): seq(4,4)
        for c in range(m['ch']): seq(8,8)
    a['tail']=r.u8()
    a['nodes']=nodes; a['mats']=mats; a['dur']=dur; a['end']=r.p; a['size']=len(r.b)
    return a
if __name__=='__main__':
    d=sys.argv[1]; bad=0; flags=set(); tails=set()
    for f in sorted(os.listdir(d)):
        try:
            a=load(os.path.join(d,f))
            for n in a['nodes']: flags.add(n['flags'])
            tails.add(a['tail'])
            if a['end']!=a['size']: bad+=1; print('size mismatch',f,a['end'],a['size'],len(a['mats']))
        except Exception as e: bad+=1; print('fail',f,e)
    print('bad',bad,'flags',flags,'tails',tails)

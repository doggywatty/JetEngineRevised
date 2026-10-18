import sys,math,os; sys.path.insert(0,os.path.join(os.path.dirname(os.path.abspath(__file__)),"..","..","tools"))
from armcall import Arm,f2i
A=Arm("apk003/lib/armeabi-v7a/libdespicablemefree.so")
F=A.floats; P=A.put_floats; o=lambda n: A.alloc(4*n)
out=[]
nq=lambda q:[x/math.sqrt(sum(y*y for y in q)) for x in q]
a=P(nq([.1,.2,.3,.9])); b=P(nq([-.4,.5,.1,.7]))
r=o(4); A.call("_ZNK4math4quatIfEmlERKS1_",r,a,b); out.append(F(r,4))
m=o(9); A.call("_ZNK4math4quatIfE9getAsMat3Ev",m,a); out.append(F(m,9))
q=o(4); A.call("_ZN4math4quatIfE11setFromMat3ERKNS_4mat3IfEE",q,m); out.append(F(q,4))
e=P([.3,-.7,1.1]); x=o(4); A.call("_ZN4math4quatIfE15setFromEulerXYZERKNS_4vec3IfEE",x,e); out.append(F(x,4))
g=o(3); A.call("_ZNK4math4quatIfE13getAsEulerXYZEv",g,x); out.append(F(g,3))
z=o(4); A.call("_ZN4math4quatIfE15setFromEulerZXYERKNS_4vec3IfEE",z,e); out.append(F(z,4))
l=o(4); A.call("_ZN4math4lerpIfEENS_4quatIT_EERKS3_S5_f",l,a,b,f2i(.3)); out.append(F(l,4))
l=o(4); A.call("_ZN4math5nlerpIfEENS_4quatIT_EERKS3_S5_f",l,a,b,f2i(.3)); out.append(F(l,4))
v=P([1.5,-2,.25]); rv=o(3); A.call("_ZN4mathmlIffEENS_4vec3IT0_EERKNS_4quatIT_EERKS3_",rv,a,v); out.append(F(rv,3))
m4=o(16); A.call("_ZNK4math4quatIfE9getAsMat4Ev",m4,a)
mm=F(m4,16); mm[12:15]=[3,4,5]; m4=P(mm); A.call("_ZN4math4mat4IfE9postScaleERKNS_4vec3IfEE",m4,P([2,3,4]))
mi=P(F(m4,16)); A.call("_ZN4math4mat4IfE6invertEv",mi); out.append(F(mi,16))
pp=o(16); A.call("_ZNK4math4mat4IfEmlERKS1_",pp,m4,mi); out.append(F(pp,16))
tv=o(3); A.call("_ZN4math9transformIfEENS_4vec3IT_EERKNS_4mat4IS2_EERKS3_",tv,m4,v); out.append(F(tv,3))
pr=o(16); A.call("_ZN4math11perspectiveIfEENS_4mat4IT_EES2_S2_S2_S2_S2_S2_",pr,f2i(-1),f2i(1),f2i(-.75),f2i(.75),f2i(.1),f2i(100)); out.append(F(pr,16))
up=P([0,1,0]); la=o(9); A.call("_ZN4math6lookAtIfEENS_4mat3IT_EERKNS_4vec3IS2_EES7_",la,v,up); out.append(F(la,9))
ql=o(4); A.call("_ZN4math10quatLookAtIfEENS_4quatIT_EERKNS_4vec3IS2_EES7_",ql,v,up); out.append(F(ql,4))
names="mul mat3 fromMat3 eulerXYZ getEulerXYZ eulerZXY slerp nlerp rotate invert m*inv transform perspective lookAt quatLookAt".split()
ours=[list(map(float,l.split())) for l in open("ours.txt")]
for n,x,y in zip(names,out,ours):
    d=max(abs(i-j) for i,j in zip(x,y))
    print(f"{n:12s} {'OK ' if d<1e-4 else 'DIFF'} {d:.2e}", "" if d<1e-4 else ([round(i,4) for i in x],[round(i,4) for i in y]))

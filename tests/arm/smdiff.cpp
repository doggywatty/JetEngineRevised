#include "smath.h"
#include <cstdio>
using namespace math;
static void p(const float* f, int n) { for (int i = 0; i < n; ++i) printf("%.6g ", f[i]); printf("\n"); }
int main()
{
    quat<float> a(0.1f, 0.2f, 0.3f, 0.9f), b(-0.4f, 0.5f, 0.1f, 0.7f);
    a.normalize(); b.normalize();
    quat<float> r = a * b; p(&r.x, 4);
    mat3<float> m3 = a.getAsMat3(); p(m3.m, 9);
    quat<float> q3(m3); p(&q3.x, 4);
    vec3<float> e(0.3f, -0.7f, 1.1f);
    quat<float> ex; ex.setFromEulerXYZ(e); p(&ex.x, 4);
    vec3<float> ge = ex.getAsEulerXYZ(); p(&ge.x, 3);
    quat<float> ez; ez.setFromEulerZXY(e); p(&ez.x, 4);
    quat<float> l = lerp(a, b, 0.3f); p(&l.x, 4);
    quat<float> nl = nlerp(a, b, 0.3f); p(&nl.x, 4);
    vec3<float> v(1.5f, -2, 0.25f), rv = a * v; p(&rv.x, 3);
    mat4<float> m4 = a.getAsMat4(); m4.setTranslation(vec3<float>(3, 4, 5)); m4.postScale(vec3<float>(2, 3, 4));
    mat4<float> mi = m4; mi.invert(); p(mi.m, 16);
    mat4<float> mm = m4 * mi; p(mm.m, 16);
    vec3<float> tv = transform(m4, v); p(&tv.x, 3);
    mat4<float> pr = perspective(-1.f, 1.f, -0.75f, 0.75f, 0.1f, 100.f); p(pr.m, 16);
    mat3<float> la = lookAt(v, vec3<float>(0, 1, 0)); p(la.m, 9);
    quat<float> ql = quatLookAt(v, vec3<float>(0, 1, 0)); p(&ql.x, 4);
}

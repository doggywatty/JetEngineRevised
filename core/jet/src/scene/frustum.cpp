// core/jet/src/scene/frustum.cpp
#include "scene/frustum.h"


namespace jet::scene {

Frustum::Frustum() : m_cameraPosition(0, 0, 0), m_dirtyPlanes(false), m_dirtyPoints(false) {
    m_viewProjMatrix.setIdentity();
    for (int i = 0; i < 6; i++) m_planes[i] = math::plane<float>(math::vec3<float>(0, 0, 1), 0);
}

Frustum::Frustum(const math::mat4<float> &vp) : Frustum() {
    m_viewProjMatrix = vp;
    m_dirtyPlanes = true;
    m_dirtyPoints = true;
}

Frustum::Frustum(const Frustum &o) : Frustum() { *this = o; }

// @0x674954: the planes and the points only count when they are up to date
Frustum &Frustum::operator=(const Frustum &o) {
    m_viewProjMatrix = o.m_viewProjMatrix;
    m_cameraPosition = o.m_cameraPosition;
    m_dirtyPlanes = o.m_dirtyPlanes;
    for (int i = 0; i < 6; i++) m_planes[i] = o.m_planes[i];
    m_dirtyPoints = o.m_dirtyPoints;
    m_farLeftUp = o.m_farLeftUp;
    m_farLeftDown = o.m_farLeftDown;
    m_farRightUp = o.m_farRightUp;
    m_farRightDown = o.m_farRightDown;
    m_nearLeftUp = o.m_nearLeftUp;
    m_nearLeftDown = o.m_nearLeftDown;
    m_nearRightUp = o.m_nearRightUp;
    m_nearRightDown = o.m_nearRightDown;
    return *this;
}

// @0x6b67ec
void Frustum::SetFrom(const math::mat4<float> &vp) {
    m_viewProjMatrix = vp;
    m_dirtyPlanes = true;
    m_dirtyPoints = true;
}

// @0x6b682c: the box of `box`, but its far plane and far corners are placed `farD` from the camera
// along `dir` (rays from the camera through the near corners for a perspective view, the near
// corners moved along `dir` for an orthogonal one)
void Frustum::SetFrom(const math::mat4<float> &vp, const math::mat4<float> &box, bool orthogonal, const math::vec3<float> &camPos, const math::vec3<float> &dir, float nearD,
                      float farD) {
    m_viewProjMatrix = box;
    ComputePoints();
    ComputePlanesFromPoints();
    math::vec3<float> farPoint = camPos + dir * farD;
    math::plane<float> farPlane(dir, -math::dot(farPoint, dir));
    math::vec3<float> *far4[4] = {&m_farLeftDown, &m_farRightDown, &m_farLeftUp, &m_farRightUp};
    const math::vec3<float> *near4[4] = {&m_nearLeftDown, &m_nearRightDown, &m_nearLeftUp, &m_nearRightUp};
    for (int i = 0; i < 4; i++) {
        if (orthogonal) {
            *far4[i] = *near4[i] + dir * (farD - nearD);
        } else {
            math::vec3<float> ray = *near4[i] - camPos;
            float t;
            math::rayIntersectPlane(camPos, ray, farPlane, t);
            *far4[i] = camPos + ray * t;
        }
    }
    m_planes[PlanesDef::Far] = farPlane;
    m_viewProjMatrix = vp;
    m_dirtyPlanes = false;
    m_dirtyPoints = false;
}

// @0x6b7188
void Frustum::ComputePlanes() const {
    ComputePlanesFromPoints();
    m_dirtyPlanes = false;
}

// @0x6b71a0
void Frustum::ComputePlanesFromPoints() const {
    math::vec3<float> fld = GetFarLeftDown(), flu = GetFarLeftUp(), frd = GetFarRightDown(), fru = GetFarRightUp();
    math::vec3<float> nld = GetNearLeftDown(), nlu = GetNearLeftUp(), nrd = GetNearRightDown(), nru = GetNearRightUp();
    m_planes[PlanesDef::Near].setPlane(nld, nrd, nru);
    m_planes[PlanesDef::Right].setPlane(nrd, frd, fru);
    m_planes[PlanesDef::Top].setPlane(nru, fru, flu);
    m_planes[PlanesDef::Far].setPlane(frd, fld, flu);
    m_planes[PlanesDef::Left].setPlane(fld, nld, nlu);
    m_planes[PlanesDef::Bottom].setPlane(nrd, nld, fld);
}

// @0x6b72b4: the corners of the NDC cube through the inverse of the matrix
void Frustum::ComputePoints() const {
    math::mat4<float> inv = m_viewProjMatrix;
    inv.invert();
    static const float k_ndc[8][3] = {{-1, 1, -1}, {1, 1, -1}, {-1, -1, -1}, {1, -1, -1}, {-1, 1, 1}, {1, 1, 1}, {-1, -1, 1}, {1, -1, 1}};
    math::vec3<float> *out[8] = {&m_nearLeftUp, &m_nearRightUp, &m_nearLeftDown, &m_nearRightDown, &m_farLeftUp, &m_farRightUp, &m_farLeftDown, &m_farRightDown};
    for (int i = 0; i < 8; i++) {
        math::vec4<float> p = math::transform(inv, math::vec4<float>(k_ndc[i][0], k_ndc[i][1], k_ndc[i][2], 1.0f));
        float w = 1.0f / p.w;
        *out[i] = math::vec3<float>(w * p.x, w * p.y, w * p.z);
    }
    m_dirtyPoints = false;
}

const math::plane<float> &Frustum::GetPlane(safe_enum<PlanesDef,PlanesDef::type> p) const {
    if (m_dirtyPlanes) ComputePlanes();
    return m_planes[p];
}

const math::plane<float> *Frustum::GetPlanes() const {
    if (m_dirtyPlanes) ComputePlanes();
    return m_planes;
}

#define POINT_GETTER(name, member) \
    math::vec3<float> Frustum::name() const { \
        if (m_dirtyPoints) ComputePoints(); \
        return member; \
    }
POINT_GETTER(GetFarLeftUp, m_farLeftUp)
POINT_GETTER(GetFarLeftDown, m_farLeftDown)
POINT_GETTER(GetFarRightUp, m_farRightUp)
POINT_GETTER(GetFarRightDown, m_farRightDown)
POINT_GETTER(GetNearLeftUp, m_nearLeftUp)
POINT_GETTER(GetNearLeftDown, m_nearLeftDown)
POINT_GETTER(GetNearRightUp, m_nearRightUp)
POINT_GETTER(GetNearRightDown, m_nearRightDown)

void Frustum::SetCameraPosition(const math::vec3<float> &p) { m_cameraPosition = p; }

// @0x6b6f34: outside when the box's most inward corner is in front of a plane, intersecting when
// its most outward corner is not behind one
safe_enum<CullStatusDef,CullStatusDef::type> Frustum::CullAABB(const math::aabb<float> &box) const {
    if (m_dirtyPlanes) ComputePlanes();
    int status = CullStatusDef::Inside;
    for (int i = 0; i < 6; i++) {
        const math::plane<float> &pl = m_planes[i];
        math::vec3<float> n, p;
        if (pl.normal.x <= 0) { n.x = box.maxPoint.x; p.x = box.minPoint.x; } else { n.x = box.minPoint.x; p.x = box.maxPoint.x; }
        if (pl.normal.y > 0) { n.y = box.minPoint.y; p.y = box.maxPoint.y; } else { n.y = box.maxPoint.y; p.y = box.minPoint.y; }
        if (pl.normal.z > 0) { n.z = box.minPoint.z; p.z = box.maxPoint.z; } else { n.z = box.maxPoint.z; p.z = box.minPoint.z; }
        if (pl.getDistanceTo(n) > 0) return CullStatusDef::Outside;
        if (pl.getDistanceTo(p) >= 0) status = CullStatusDef::Intersect;
    }
    return (CullStatusDef::type)status;
}

// @0x6b7034
safe_enum<CullStatusDef,CullStatusDef::type> Frustum::CullSphere(const math::vec3<float> &c, float r) const {
    if (m_dirtyPlanes) ComputePlanes();
    int status = CullStatusDef::Inside;
    for (int i = 0; i < 6; i++) {
        float d = m_planes[i].getDistanceTo(c);
        if (d > r) return CullStatusDef::Outside;
        if (d > -r) status = CullStatusDef::Intersect;
    }
    return (CullStatusDef::type)status;
}

// @0x6b70ac: without the near and far planes
safe_enum<CullStatusDef,CullStatusDef::type> Frustum::CullSphereAround(const math::vec3<float> &c, float r) const {
    if (m_dirtyPlanes) ComputePlanes();
    int status = CullStatusDef::Inside;
    for (int i = PlanesDef::Top; i >= PlanesDef::Left; i--) {
        float d = m_planes[i].getDistanceTo(c);
        if (d > r) return CullStatusDef::Outside;
        if (d > -r) status = CullStatusDef::Intersect;
    }
    return (CullStatusDef::type)status;
}

// @0x6b7124: the same, never telling intersecting from inside
safe_enum<CullStatusDef,CullStatusDef::type> Frustum::CullSphereAroundFast(const math::vec3<float> &c, float r) const {
    if (m_dirtyPlanes) ComputePlanes();
    for (int i = PlanesDef::Top; i >= PlanesDef::Left; i--)
        if (m_planes[i].getDistanceTo(c) > r) return CullStatusDef::Outside;
    return CullStatusDef::Inside;
}

}

// core/jet/src/scene/camera.cpp
// Camera (jet::scene::Camera, RefreshTransforms @0x6b43ac). World space is 3ds Max style Z-up:
// right (-1,0,0), up (0,0,1), front (0,-1,0); jet2gl turns a camera frame into GL's.
#include "scene/camera.h"
#include "scene/modelbase.h"
#include "video/display.h"
#include "video/driver.h"
#include <algorithm>
#include <cmath>

namespace jet::scene {

static const float k_eps = 1.1920928955078125e-07f;

// @0x6b39e0 / @0x6b3934: the same swap of the 2nd and 3rd columns with the 1st negated
static math::mat4<float> jet2gl(const math::mat4<float> &m) {
    math::mat4<float> r(m);
    for (int i = 0; i < 4; i++) {
        r.m[i] = -m.m[i];
        r.m[4 + i] = m.m[8 + i];
        r.m[8 + i] = m.m[4 + i];
    }
    return r;
}

static math::mat4<float> gl2jet(const math::mat4<float> &m) { return jet2gl(m); }

static math::vec2<unsigned int> RenderTargetSize() {
    return jet::video::Driver::GetCurrent()->GetCrtRenderTarget()->GetSize();
}

// @0x6b3a8c
Camera::Camera(const math::vec3<float> &pos, const math::quat<float> &rot)
    : m_viewport(0, 0, 0, 0), m_cachedViewport(0, 0, 0, 0), m_position(0, 0, 0), m_rotation(0, 0, 0, 1), m_worldDataChanged(false), m_frontVector(0, 0, 0), m_upVector(0, 0, 0),
      m_rightVector(0, 0, 0), m_aspectRatio(0), m_offCenter(-1, -1, 1, 1), m_isOrthogonal(false), m_projDataChanged(true), m_fovy(0.7853982f), m_nearDist(1.0f), m_farDist(10000.0f),
      m_orthoZoom(1.0f), m_cachedOrientation(0) {
    SetTransform(pos, rot);
    m_nearDist = 0.5f;
    m_projDataChanged = true;
    m_orthoZoom = std::max(1.0f, m_nearDist);
}

Camera::Camera(const Camera &o) : Camera(o.m_position, o.m_rotation) { *this = o; }

Camera &Camera::operator=(const Camera &o) {
    m_viewport = o.m_viewport;
    m_cachedViewport = o.m_cachedViewport;
    m_position = o.m_position;
    m_rotation = o.m_rotation;
    m_worldDataChanged = o.m_worldDataChanged;
    m_frontVector = o.m_frontVector;
    m_upVector = o.m_upVector;
    m_rightVector = o.m_rightVector;
    m_aspectRatio = o.m_aspectRatio;
    m_offCenter = o.m_offCenter;
    m_isOrthogonal = o.m_isOrthogonal;
    m_viewTransform = o.m_viewTransform;
    m_projectionTransform = o.m_projectionTransform;
    m_projectionTransformOriented = o.m_projectionTransformOriented;
    m_viewProjectionTransform = o.m_viewProjectionTransform;
    m_projDataChanged = o.m_projDataChanged;
    m_instanceName = o.m_instanceName;
    m_fovy = o.m_fovy;
    m_nearDist = o.m_nearDist;
    m_farDist = o.m_farDist;
    m_orthoZoom = o.m_orthoZoom;
    m_cachedOrientation = o.m_cachedOrientation;
    m_frustum = o.m_frustum;
    m_untransformedFrustum = o.m_untransformedFrustum;
    return *this;
}

Camera::~Camera() {}

// @0x6b4990
void Camera::Load(const ModelBase::CameraData &d) {
    SetTransform(d.position, d.rotation);
    m_instanceName = d.targetName;
    if (m_fovy != d.fov) m_fovy = d.fov;
    m_nearDist = d.nearDist;
    m_farDist = d.farDist;
    m_isOrthogonal = d.orthogonal;
    m_projDataChanged = true;
}

void Camera::SetPosition(const math::vec3<float> &p) {
    if (p == m_position) return;
    m_position = p;
    m_worldDataChanged = true;
}

void Camera::SetRotation(const math::quat<float> &r) {
    if (r == m_rotation) return;
    m_rotation = r;
    m_worldDataChanged = true;
}

void Camera::SetTransform(const math::vec3<float> &p, const math::quat<float> &r) {
    if (!(p == m_position)) {
        m_position = p;
        m_worldDataChanged = true;
    }
    if (r == m_rotation) return;
    m_rotation = r;
    m_worldDataChanged = true;
}

// @0x6b4c24: the matrices are for the render target they were built with
bool Camera::HasViewportChanged() const {
    math::vec2<unsigned int> s = RenderTargetSize();
    return !(m_cachedViewport.upperLeftCorner.x == 0 && m_cachedViewport.upperLeftCorner.y == 0 && m_cachedViewport.lowerRightCorner.x == s.x && m_cachedViewport.lowerRightCorner.y == s.y);
}

// @0x6b4ba0
float Camera::GetAspectRatio() {
    if (m_aspectRatio != 0) return m_aspectRatio;
    math::vec2<unsigned int> s = GetViewportSize();
    return (float)s.x / (float)s.y;
}

// @0x6b4348
void Camera::SetHorizontalFov(float f) {
    float v = 2.0f * std::atan(std::tan(f * 0.5f) / GetAspectRatio());
    if (m_fovy != v) {
        m_fovy = v;
        m_projDataChanged = true;
    }
}

float Camera::GetHorizontalFov() { return 2.0f * std::atan(std::tan(m_fovy * 0.5f) * GetAspectRatio()); }

// @0x6b41d0
void Camera::SetViewport(const math::rect<unsigned int> &v) {
    m_viewport = v;
    if (v.lowerRightCorner.x < v.upperLeftCorner.x) std::swap(m_viewport.upperLeftCorner.x, m_viewport.lowerRightCorner.x);
    if (v.lowerRightCorner.y < v.upperLeftCorner.y) std::swap(m_viewport.upperLeftCorner.y, m_viewport.lowerRightCorner.y);
    m_projDataChanged = true;
}

// @0x6b4218: corners are inclusive; a viewport that is a pixel or less wide or tall is the render target
math::rect<unsigned int> Camera::GetViewport() {
    unsigned w1 = m_viewport.lowerRightCorner.x - m_viewport.upperLeftCorner.x;
    unsigned h1 = m_viewport.lowerRightCorner.y - m_viewport.upperLeftCorner.y;
    if ((w1 > 1 ? h1 : w1) >= 2) return m_viewport;
    math::vec2<unsigned int> s = RenderTargetSize();
    unsigned w = w1 == 0 ? s.x : w1, h = h1 == 0 ? s.y : h1;
    unsigned x0 = m_viewport.upperLeftCorner.x, y0 = m_viewport.upperLeftCorner.y;
    unsigned x1 = ~x0 + w, y1 = ~y0 + h;
    if (x1 < x0) std::swap(x0, x1);
    if (y1 < y0) std::swap(y0, y1);
    return math::rect<unsigned int>(x0, y0, x1, y1);
}

// @0x6b4308
math::vec2<unsigned int> Camera::GetViewportSize() {
    math::rect<unsigned int> v = GetViewport();
    return math::vec2<unsigned int>(v.lowerRightCorner.x + 1 - v.upperLeftCorner.x, v.lowerRightCorner.y + 1 - v.upperLeftCorner.y);
}

// @0x6b4cb0
math::mat4<float> Camera::ComputePerspectiveMatrix(float fov, float aspect, float n, float f) const {
    float t = std::tan(fov * 0.5f) * n;
    return math::perspective(t * aspect * m_offCenter.upperLeftCorner.x, t * aspect * m_offCenter.lowerRightCorner.x, t * m_offCenter.upperLeftCorner.y, t * m_offCenter.lowerRightCorner.y,
                             n, f);
}

// @0x6b4d40
math::mat4<float> Camera::ComputeOrthographicMatrix(float w, float h, float n, float f) const {
    float l = w * 0.5f * m_offCenter.upperLeftCorner.x, r = w * 0.5f * m_offCenter.lowerRightCorner.x;
    float b = h * 0.5f * m_offCenter.upperLeftCorner.y, t = h * 0.5f * m_offCenter.lowerRightCorner.y;
    math::mat4<float> m;
    m.setIdentity();
    for (int i = 0; i < 16; i++) m.m[i] = 0;
    m.m[15] = 1.0f;
    m.m[0] = 2.0f / (r - l);
    m.m[5] = 2.0f / (t - b);
    m.m[10] = -2.0f / (f - n);
    m.m[12] = (r + l) / (l - r);
    m.m[13] = (t + b) / (b - t);
    m.m[14] = (n + f) / (n - f);
    return m;
}

// @0x6b43ac
void Camera::RefreshTransforms() {
    bool viewportChanged = HasViewportChanged();
    if (viewportChanged) {
        math::vec2<unsigned int> s = RenderTargetSize();
        m_cachedViewport = math::rect<unsigned int>(0, 0, s.x, s.y);
    }
    if (m_worldDataChanged || viewportChanged) {
        math::mat4<float> world = m_rotation.getAsMat4();
        world.m[12] = m_position.x;
        world.m[13] = m_position.y;
        world.m[14] = m_position.z;
        m_viewTransform = jet2gl(world);
        m_viewTransform.invert();
        m_frontVector = m_rotation * math::vec3<float>(0, -1, 0);
        m_frontVector.normalize();
        m_upVector = m_rotation * math::vec3<float>(0, 0, 1);
        m_upVector.normalize();
        m_rightVector = m_rotation * math::vec3<float>(-1, 0, 0);
        m_rightVector.normalize();
    }
    unsigned orientation = (unsigned)jet::video::Driver::GetCurrent()->GetDisplay()->GetOrientation();
    if (m_cachedOrientation != orientation) {
        m_projDataChanged = true;
        m_cachedOrientation = orientation;
    }
    if (m_projDataChanged || viewportChanged) {
        float aspect = GetAspectRatio();
        float orientedAspect = (orientation & ~2u) == 1 ? 1.0f / aspect : aspect;
        if (!m_isOrthogonal) {
            m_projectionTransform = ComputePerspectiveMatrix(m_fovy, aspect, m_nearDist, m_farDist);
            m_projectionTransformOriented = ComputePerspectiveMatrix(m_fovy, orientedAspect, m_nearDist, m_farDist);
        } else {
            float h = m_orthoZoom + m_orthoZoom;
            m_projectionTransform = ComputeOrthographicMatrix(aspect * h, h, m_nearDist, m_farDist);
            m_projectionTransformOriented = ComputeOrthographicMatrix(orientedAspect * h, h, m_nearDist, m_farDist);
        }
        m_untransformedFrustum.SetFrom(gl2jet(m_projectionTransform));
    }
    if (m_worldDataChanged || m_projDataChanged || viewportChanged) {
        m_viewProjectionTransform = m_projectionTransform * m_viewTransform;
        m_frustum.SetCameraPosition(m_position);
        // the frustum's sides come from a projection to a near+10 far plane, the far plane and
        // corners are then moved out to the real one
        math::mat4<float> p = !m_isOrthogonal ? ComputePerspectiveMatrix(m_fovy, GetAspectRatio(), m_nearDist, m_nearDist + 10.0f)
                                               : ComputeOrthographicMatrix(GetAspectRatio() * (m_orthoZoom + m_orthoZoom), m_orthoZoom + m_orthoZoom, m_nearDist, m_nearDist + 10.0f);
        m_worldDataChanged = false;
        m_projDataChanged = false;
        m_frustum.SetFrom(m_viewProjectionTransform, p * m_viewTransform, m_isOrthogonal, m_position, m_frontVector, m_nearDist, m_farDist);
    }
    m_worldDataChanged = false;
    m_projDataChanged = false;
}

// @0x6b3c48: the point of the near rectangle at the fraction of the viewport, to the same one on
// the far rectangle
math::line3<float> Camera::GetRayFromScreenCoordinates(const math::vec2<float> &p) {
    math::vec2<unsigned int> s = GetViewportSize();
    float u = p.x / (float)s.x, v = p.y / (float)s.y;
    const Frustum &f = GetFrustum();
    math::vec3<float> nlu = f.GetNearLeftUp();
    math::vec3<float> a = nlu + (f.GetNearRightUp() - nlu) * u + (f.GetNearLeftDown() - nlu) * v;
    math::vec3<float> flu = f.GetFarLeftUp();
    math::vec3<float> b = flu + (f.GetFarRightUp() - flu) * u + (f.GetFarLeftDown() - flu) * v;
    return math::line3<float>(a, b);
}

// @0x6b3ec8
bool Camera::Project(math::vec3<float> &out, const math::vec3<float> &world) {
    math::mat4<float> vp = GetViewProjectionTransform();
    math::rect<unsigned int> v = GetViewport();
    math::vec4<float> c = math::transform(vp, math::vec4<float>(world.x, world.y, world.z, 1.0f));
    if (std::fabs(c.w) > k_eps) {
        float inv = 1.0f / c.w;
        c.x *= inv;
        c.y *= inv;
        c.z *= inv;
    }
    if (!(std::fabs(c.w) > k_eps)) return false;
    float w = (float)(v.lowerRightCorner.x - v.upperLeftCorner.x), h = (float)(v.lowerRightCorner.y - v.upperLeftCorner.y);
    float hx = (w + 1.0f) * 0.5f, hy = (h + 1.0f) * 0.5f;
    out.x = hx + c.x * hx + (float)v.upperLeftCorner.x;
    out.y = ((h + 1.0f) - (float)v.upperLeftCorner.y) - (hy + c.y * hy);
    out.z = c.z * 0.5f + 0.5f;
    return c.w > 0.0f;
}

bool Camera::Project(math::vec2<float> &out, const math::vec3<float> &world) {
    math::vec3<float> p(0, 0, 0);
    Project(p, world);
    out.x = p.x;
    out.y = p.y;
    return true;
}

// @0x6b4028
bool Camera::Unproject(math::vec3<float> &out, const math::vec3<float> &s) {
    math::rect<unsigned int> v = GetViewport();
    float x0 = (float)v.upperLeftCorner.x, y0 = (float)v.upperLeftCorner.y;
    float h = (float)(v.lowerRightCorner.y - v.upperLeftCorner.y), w = (float)(v.lowerRightCorner.x - v.upperLeftCorner.x);
    float sy = ((h + 1.0f) - s.y) - y0;
    math::mat4<float> inv = GetViewProjectionTransform();
    inv.invert();
    math::vec4<float> ndc(((s.x - x0) + (s.x - x0)) / (w + 1.0f) - 1.0f, (sy + sy) / (h + 1.0f) - 1.0f, (s.z + s.z) - 1.0f, 1.0f);
    math::vec4<float> r = math::transform(inv, ndc);
    if (std::fabs(r.w) > k_eps) {
        float k = 1.0f / r.w;
        r.x *= k;
        r.y *= k;
        r.z *= k;
    }
    out = math::vec3<float>(r.x, r.y, r.z);
    return true;
}

// @0x6b49f4: the planes of the part of the view inside a screen rectangle
void Camera::ComputeFrustumPlanes(int x0, int y0, int x1, int y1, math::plane<float> *out) {
    math::vec3<float> a(0, 0, 0), b(0, 0, 0);
    RefreshTransforms();
    Unproject(a, math::vec3<float>((float)x0, (float)y1, 0));
    Unproject(b, math::vec3<float>((float)x1, (float)y0, 0));
    a = math::transform(GetViewTransform(), a);
    b = math::transform(GetViewTransform(), b);
    math::mat4<float> proj = math::perspective(a.x, b.x, b.y, a.y, m_nearDist, m_farDist);
    Frustum f;
    f.SetFrom(proj * GetViewTransform());
    for (int i = 0; i < 6; i++) out[i] = f.GetPlane((PlanesDef::type)i);
}

}

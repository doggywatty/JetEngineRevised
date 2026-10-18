#include <chrono>
#include <cstdlib>
// core/jet/src/video/painter.cpp
#include "video/painter.h"
#include "video/driver.h"
#include "video/renderjob.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <boost/bind.hpp>

namespace jet::video {

std::vector<math::vec2<float> > Painter::s_cosSinTable;

namespace {
const unsigned kCosSinTableSize = 64;
const int kOpNone = -1;
}

Painter::Painter(unsigned short maxVerticesInBatch)
    : m_endFrameCallback(0), m_unloadResourceCallback(0), m_maxVerticesInBatch(maxVerticesInBatch),
      m_hasPreClipTransform(false), m_hasPostClipTransform(false), m_hasCombinedTransform(false),
      m_crtCameraIdx(-1), m_crtMaterialIdx(-1), m_bytecodeIdx(0), m_bytecodeSize(0), m_batchInfosIdx(0),
      m_lastOperation(kOpNone), m_triangleGeometryPool(8), m_lineGeometryPool(8), m_isInitialized(false) {
    Init();
}

Painter::~Painter() { Unload(); }

void Painter::Init() {
    if (s_cosSinTable.empty()) {
        s_cosSinTable.reserve(kCosSinTableSize + 1);
        for (unsigned i = 0; i <= kCosSinTableSize; ++i) {
            float a = (float)i / (float)kCosSinTableSize * 6.283185307179586f;
            s_cosSinTable.push_back(math::vec2<float>(std::cos(a), std::sin(a)));
        }
    }
    m_triangleGeometryPool.CreateGeometry = boost::bind(&Painter::CreateTriangleGeometry, this);
    m_lineGeometryPool.CreateGeometry = boost::bind(&Painter::CreateLineGeometry, this);
    m_isInitialized = true;
}

bool Painter::IsInitialized() { return m_isInitialized; }

boost::shared_ptr<Geometry> Painter::CreateTriangleGeometry() {
    boost::shared_ptr<Geometry> g = Geometry::New();
    g->SetTopology(Geometry::TopologyDef::Triangles);
    g->SetVertexCount(m_maxVerticesInBatch);
    g->AddAttribute("position", Geometry::SemanticDef::Position, Geometry::UsageDef::Stream, Geometry::TypeDef::Float, 3);
    g->AddAttribute("color", Geometry::SemanticDef::Color, Geometry::UsageDef::Stream, Geometry::TypeDef::Float, 4);
    g->AddAttribute("uv", Geometry::SemanticDef::TexCoord0, Geometry::UsageDef::Stream, Geometry::TypeDef::Float, 2);
    return g;
}

boost::shared_ptr<Geometry> Painter::CreateLineGeometry() {
    boost::shared_ptr<Geometry> g = Geometry::New();
    g->SetTopology(Geometry::TopologyDef::Lines);
    g->SetVertexCount(m_maxVerticesInBatch);
    g->AddAttribute("position", Geometry::SemanticDef::Position, Geometry::UsageDef::Stream, Geometry::TypeDef::Float, 3);
    g->AddAttribute("color", Geometry::SemanticDef::Color, Geometry::UsageDef::Stream, Geometry::TypeDef::Float, 4);
    g->AddAttribute("uv", Geometry::SemanticDef::TexCoord0, Geometry::UsageDef::Stream, Geometry::TypeDef::Float, 2);
    return g;
}

// --- Raw batch recording ---------------------------------------------------

// SetTransform / SetPostClipTransform apply to the primitives drawn after them, so positions are
// transformed as they are recorded (@0x6661c4)
Painter::Vertex Painter::TransformVertex(const Vertex &v) {
    if (!m_hasPreClipTransform && !m_hasPostClipTransform) return v;
    if (!m_hasCombinedTransform) {
        // post * pre (SetTransform/SetPostClipTransform @0x6ee240/@0x6ee2ec): the element's own transform first
        m_combinedTransform = m_hasPreClipTransform ? m_preClipTransform : math::mat4<float>();
        if (m_hasPostClipTransform) m_combinedTransform = m_postClipTransform * m_combinedTransform;
        m_hasCombinedTransform = true;
    }
    Vertex r = v;
    r.position = math::transform(m_combinedTransform, v.position);
    return r;
}

// Batches the vertices as given
void Painter::RecordLine(const Vertex &a, const Vertex &b) {
    if (m_lastOperation != Geometry::TopologyDef::Lines) {
        BatchInfo bi;
        bi.operation = Geometry::TopologyDef::Lines;
        bi.byteOffset = static_cast<unsigned>(m_bytecode.size());
        bi.vertexCount = 0;
        bi.materialIdx = m_crtMaterialIdx;
        m_batchInfos.push_back(bi);
        m_lastOperation = Geometry::TopologyDef::Lines;
    }
    Vertex verts[2] = { a, b };
    size_t off = m_bytecode.size();
    m_bytecode.resize(off + sizeof(verts));
    std::memcpy(m_bytecode.data() + off, verts, sizeof(verts));
    m_batchInfos.back().vertexCount += 2;
}

void Painter::RecordTriangle(const Vertex &a, const Vertex &b, const Vertex &c) {
    static auto t0 = std::chrono::steady_clock::now();
    if (getenv("JET_DBGTRI")) {
        const float px = 100, py = 500;
        auto sg = [](float x1, float y1, float x2, float y2, float x3, float y3) { return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3); };
        float d1 = sg(px, py, a.position.x, a.position.y, b.position.x, b.position.y), d2 = sg(px, py, b.position.x, b.position.y, c.position.x, c.position.y), d3 = sg(px, py, c.position.x, c.position.y, a.position.x, a.position.y);
        bool neg = d1 < 0 || d2 < 0 || d3 < 0, pos = d1 > 0 || d2 > 0 || d3 > 0;
        if (!(neg && pos)) fprintf(stderr, "tri %lld mat %d col %08x uv %.2f,%.2f pos %.0f,%.0f %.0f,%.0f %.0f,%.0f\n", (long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count(), m_crtMaterialIdx, (unsigned)a.color.GetPacked(), a.uv.x, a.uv.y, a.position.x, a.position.y, b.position.x, b.position.y, c.position.x, c.position.y);
    }
    if (m_lastOperation != Geometry::TopologyDef::Triangles) {
        BatchInfo bi;
        bi.operation = Geometry::TopologyDef::Triangles;
        bi.byteOffset = static_cast<unsigned>(m_bytecode.size());
        bi.vertexCount = 0;
        bi.materialIdx = m_crtMaterialIdx;
        m_batchInfos.push_back(bi);
        m_lastOperation = Geometry::TopologyDef::Triangles;
    }
    Vertex verts[3] = { a, b, c };
    size_t off = m_bytecode.size();
    m_bytecode.resize(off + sizeof(verts));
    std::memcpy(m_bytecode.data() + off, verts, sizeof(verts));
    m_batchInfos.back().vertexCount += 3;
}

// without clipping both transforms act as one (@0x6661c4)
void Painter::DrawLineRaw(const Vertex &a, const Vertex &b) { RecordLine(TransformVertex(a), TransformVertex(b)); }
void Painter::FillTriangleRaw(const Vertex &a, const Vertex &b, const Vertex &c) { RecordTriangle(TransformVertex(a), TransformVertex(b), TransformVertex(c)); }

math::vec3<float> Painter::PreClip(const math::vec3<float> &p) { return m_hasPreClipTransform ? math::transform(m_preClipTransform, p) : p; }
math::vec3<float> Painter::PostClip(const math::vec3<float> &p) { return m_hasPostClipTransform ? math::transform(m_postClipTransform, p) : p; }

// x y u v r g b a of a vertex, the layout RectClipper interpolates
static void PackVertex(const Painter::Vertex &v, const math::vec3<float> &pos, float *o) {
    o[0] = pos.x; o[1] = pos.y;
    o[2] = v.uv.x; o[3] = v.uv.y;
    o[4] = v.color.r; o[5] = v.color.g; o[6] = v.color.b; o[7] = v.color.a;
}

static Painter::Vertex UnpackVertex(const float *o, float z, const Painter::Vertex &like) {
    Painter::Vertex v = like;
    v.position = math::vec3<float>(o[0], o[1], z);
    v.uv = math::vec2<float>(o[2], o[3]);
    v.color = Color(o[4], o[5], o[6], o[7]);
    return v;
}

// --- Public drawing primitives ---------------------------------------------

// @0x6657fc: lines crossing the clip rect are cut, the pieces then take the post-clip transform
void Painter::DrawLine(const Vertex &a, const Vertex &b) {
    if (!m_clippingParameters.enabled) { DrawLineRaw(a, b); return; }
    float pts[16];
    PackVertex(a, PreClip(a.position), pts);
    PackVertex(b, PreClip(b.position), pts + 8);
    if (m_clippingParameters.rectMode) m_clippingParameters.rectClipper.SetRect(m_clippingParameters.clipStack.GetTop());
    jet::util::RectClipper &rc = m_clippingParameters.rectClipper;
    rc.ClipPrimitive(pts, 2, 8);
    unsigned n = rc.m_numberOfOutputPoints;
    for (unsigned i = 0; n && i < n - 1; i++) {
        const float *p = &rc.m_output[i * 8], *q = &rc.m_output[((i + 1) % n) * 8];
        Vertex va = UnpackVertex(p, a.position.z, a), vb = UnpackVertex(q, a.position.z, b);
        va.position = PostClip(va.position);
        vb.position = PostClip(vb.position);
        RecordLine(va, vb);
    }
}

// @0x6653c8 / @0x665488: the other two corners take the first vertex's colour, the middle depth and the mixed texture coordinates
static void RectangleCorners(const Painter::Vertex &a, const Painter::Vertex &b, Painter::Vertex &tr, Painter::Vertex &bl) {
    float z = (a.position.z + b.position.z) * 0.5f;
    tr = Painter::Vertex(math::vec3<float>(b.position.x, a.position.y, z), a.color, math::vec2<float>(b.uv.x, a.uv.y));
    bl = Painter::Vertex(math::vec3<float>(a.position.x, b.position.y, z), a.color, math::vec2<float>(a.uv.x, b.uv.y));
}

void Painter::DrawRectangle(const Vertex &a, const Vertex &b) {
    Vertex tr, bl;
    RectangleCorners(a, b, tr, bl);
    DrawQuad(a, tr, b, bl);
}

void Painter::FillRectangle(const Vertex &a, const Vertex &b) {
    Vertex tr, bl;
    RectangleCorners(a, b, tr, bl);
    FillQuad(a, tr, b, bl);
}

void Painter::DrawQuad(const Vertex &a, const Vertex &b, const Vertex &c, const Vertex &d) {
    DrawLine(a, b); DrawLine(b, c); DrawLine(c, d); DrawLine(d, a);
}

// @0x6661c4: a quad inside the clip rect is batched whole, one crossing it is clipped as two triangles
void Painter::FillQuad(const Vertex &a, const Vertex &b, const Vertex &c, const Vertex &d) {
    if (m_clippingParameters.enabled && m_clippingParameters.rectMode) {
        const math::rect<float> clip = m_clippingParameters.clipStack.GetTop();
        math::vec3<float> p[4] = { PreClip(a.position), PreClip(b.position), PreClip(c.position), PreClip(d.position) };
        float minX = p[0].x, maxX = p[0].x, minY = p[0].y, maxY = p[0].y;
        for (int i = 1; i < 4; i++) {
            minX = std::min(minX, p[i].x); maxX = std::max(maxX, p[i].x);
            minY = std::min(minY, p[i].y); maxY = std::max(maxY, p[i].y);
        }
        if (maxX < clip.upperLeftCorner.x || clip.lowerRightCorner.x < minX) return;
        if (maxY < clip.upperLeftCorner.y || clip.lowerRightCorner.y < minY) return;
        if (!(clip.upperLeftCorner.x < minX && maxX < clip.lowerRightCorner.x && clip.upperLeftCorner.y < minY && maxY < clip.lowerRightCorner.y)) {
            FillTriangle(a, b, c);
            FillTriangle(a, c, d);
            return;
        }
    } else if (m_clippingParameters.enabled) {
        FillTriangle(a, b, c);
        FillTriangle(a, c, d);
        return;
    }
    FillTriangleRaw(a, b, c);
    FillTriangleRaw(a, c, d);
}

void Painter::DrawTriangle(const Vertex &a, const Vertex &b, const Vertex &c) {
    DrawLine(a, b); DrawLine(b, c); DrawLine(c, a);
}

// @0x6667b4: the clipped polygon is fanned from its first point
void Painter::FillTriangle(const Vertex &a, const Vertex &b, const Vertex &c) {
    if (!m_clippingParameters.enabled) { FillTriangleRaw(a, b, c); return; }
    if (m_clippingParameters.rectMode && m_clippingParameters.clipStack.GetSize() == 0) return;
    float pts[24];
    PackVertex(a, PreClip(a.position), pts);
    PackVertex(b, PreClip(b.position), pts + 8);
    PackVertex(c, PreClip(c.position), pts + 16);
    if (m_clippingParameters.rectMode) m_clippingParameters.rectClipper.SetRect(m_clippingParameters.clipStack.GetTop());
    jet::util::RectClipper &rc = m_clippingParameters.rectClipper;
    rc.ClipPrimitive(pts, 3, 8);
    unsigned n = rc.m_numberOfOutputPoints;
    for (unsigned i = 0; n && i + 2 < n; i++) {
        const float *p0 = &rc.m_output[0], *p1 = &rc.m_output[((i + 2) % n) * 8], *p2 = &rc.m_output[((i + 1) % n) * 8];
        Vertex v0 = UnpackVertex(p0, a.position.z, a), v1 = UnpackVertex(p1, a.position.z, b), v2 = UnpackVertex(p2, a.position.z, c);
        v0.position = PostClip(v0.position);
        v1.position = PostClip(v1.position);
        v2.position = PostClip(v2.position);
        RecordTriangle(v0, v1, v2);
    }
}

void Painter::DrawTriangleList(const Vertex *v, unsigned nv, const unsigned *idx, unsigned ni) {
    for (unsigned i = 0; i + 2 < ni; i += 3) DrawTriangle(v[idx[i]], v[idx[i + 1]], v[idx[i + 2]]);
    (void)nv;
}

void Painter::FillTriangleList(const Vertex *v, unsigned nv, const unsigned *idx, unsigned ni) {
    for (unsigned i = 0; i + 2 < ni; i += 3) FillTriangle(v[idx[i]], v[idx[i + 1]], v[idx[i + 2]]);
    (void)nv;
}

void Painter::DrawQuadList(const Vertex *v, unsigned nv, const unsigned *idx, unsigned ni) {
    for (unsigned i = 0; i + 3 < ni; i += 4) DrawQuad(v[idx[i]], v[idx[i + 1]], v[idx[i + 2]], v[idx[i + 3]]);
    (void)nv;
}

void Painter::FillQuadList(const Vertex *v, unsigned nv, const unsigned *idx, unsigned ni) {
    for (unsigned i = 0; i + 3 < ni; i += 4) FillQuad(v[idx[i]], v[idx[i + 1]], v[idx[i + 2]], v[idx[i + 3]]);
    (void)nv;
}

static Painter::Vertex EllipsePoint(const Painter::Vertex &center, const math::vec2<float> &radius, const math::vec2<float> &cs) {
    return Painter::Vertex(math::vec2<float>(center.position.x + radius.x * cs.x, center.position.y + radius.y * cs.y), center.color);
}

void Painter::DrawEllipse(const Vertex &center, const math::vec2<float> &radius) {
    std::vector<Vertex> pts;
    pts.reserve(s_cosSinTable.size());
    for (size_t i = 0; i < s_cosSinTable.size(); ++i) pts.push_back(EllipsePoint(center, radius, s_cosSinTable[i]));
    DrawPolyline(pts);
}

void Painter::FillEllipse(const Vertex &center, const math::vec2<float> &radius) {
    std::vector<Vertex> pts;
    pts.reserve(s_cosSinTable.size());
    for (size_t i = 0; i < s_cosSinTable.size(); ++i) pts.push_back(EllipsePoint(center, radius, s_cosSinTable[i]));
    FillTriangleFan(pts);
}

void Painter::DrawCircle(const Vertex &center, float radius) { DrawEllipse(center, math::vec2<float>(radius, radius)); }
void Painter::FillCircle(const Vertex &center, float radius) { FillEllipse(center, math::vec2<float>(radius, radius)); }

void Painter::DrawArc(const Vertex &center, float radius, float startAngle, float endAngle) {
    std::vector<Vertex> pts;
    unsigned steps = kCosSinTableSize;
    for (unsigned i = 0; i <= steps; ++i) {
        float t = startAngle + (endAngle - startAngle) * (float)i / (float)steps;
        pts.push_back(Vertex(math::vec2<float>(center.position.x + radius * std::cos(t), center.position.y + radius * std::sin(t)), center.color));
    }
    DrawPolyline(pts);
}

void Painter::FillArc(const Vertex &center, float radius, float startAngle, float endAngle) {
    std::vector<Vertex> pts;
    pts.push_back(center);
    unsigned steps = kCosSinTableSize;
    for (unsigned i = 0; i <= steps; ++i) {
        float t = startAngle + (endAngle - startAngle) * (float)i / (float)steps;
        pts.push_back(Vertex(math::vec2<float>(center.position.x + radius * std::cos(t), center.position.y + radius * std::sin(t)), center.color));
    }
    FillTriangleFan(pts);
}

void Painter::DrawPolyline(const Vertex *v, unsigned n) {
    for (unsigned i = 0; i + 1 < n; ++i) DrawLine(v[i], v[i + 1]);
}
void Painter::DrawPolyline(const std::vector<Vertex> &v) { DrawPolyline(v.data(), static_cast<unsigned>(v.size())); }

void Painter::FillTriangleFan(const Vertex *v, unsigned n) {
    for (unsigned i = 1; i + 1 < n; ++i) FillTriangle(v[0], v[i], v[i + 1]);
}
void Painter::FillTriangleFan(const std::vector<Vertex> &v) { FillTriangleFan(v.data(), static_cast<unsigned>(v.size())); }

void Painter::DrawBox(const math::aabb<float> &box, unsigned packedColor) {
    Color c(packedColor);
    math::vec3<float> mn = box.minPoint, mx = box.maxPoint;
    math::vec3<float> p[8] = {
        math::vec3<float>(mn.x, mn.y, mn.z), math::vec3<float>(mx.x, mn.y, mn.z),
        math::vec3<float>(mx.x, mx.y, mn.z), math::vec3<float>(mn.x, mx.y, mn.z),
        math::vec3<float>(mn.x, mn.y, mx.z), math::vec3<float>(mx.x, mn.y, mx.z),
        math::vec3<float>(mx.x, mx.y, mx.z), math::vec3<float>(mn.x, mx.y, mx.z),
    };
    static const int edges[12][2] = { {0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7} };
    for (int i = 0; i < 12; ++i)
        DrawLineRaw(Vertex(p[edges[i][0]], c), Vertex(p[edges[i][1]], c));
}

void Painter::DrawAxis(const math::quat<float> &rot, const math::vec3<float> &pos) {
    math::vec3<float> x = rot * math::vec3<float>(1, 0, 0);
    math::vec3<float> y = rot * math::vec3<float>(0, 1, 0);
    math::vec3<float> z = rot * math::vec3<float>(0, 0, 1);
    DrawLineRaw(Vertex(pos, Color(1, 0, 0, 1)), Vertex(pos + x, Color(1, 0, 0, 1)));
    DrawLineRaw(Vertex(pos, Color(0, 1, 0, 1)), Vertex(pos + y, Color(0, 1, 0, 1)));
    DrawLineRaw(Vertex(pos, Color(0, 0, 1, 1)), Vertex(pos + z, Color(0, 0, 1, 1)));
}

void Painter::DrawAxis(const math::mat4<float> &m) {
    math::vec3<float> pos = m.getTranslation();
    DrawAxis(math::quat<float>(m), pos);
}

void Painter::FillFullScreenRectangle(unsigned packedColor) {
    Color c(packedColor);
    FillRectangle(Vertex(math::vec2<float>(-1, -1), c), Vertex(math::vec2<float>(1, 1), c));
}

// --- State -------------------------------------------------------------

// Each material change starts a new batch that keeps its own copy of the material until Flush.
void Painter::SetMaterial(const Material &m) {
    m_material = m;
    m_materials.push_back(m);
    m_crtMaterialIdx = (int)m_materials.size() - 1;
    m_lastOperation = kOpNone;
}
const Material &Painter::GetMaterial() { return m_material; }

void Painter::SetTransform(const math::mat4<float> &m) { m_preClipTransform = m; m_hasPreClipTransform = true; m_hasCombinedTransform = false; }
const math::mat4<float> &Painter::GetTransform() { return m_preClipTransform; }
void Painter::SetPostClipTransform(const math::mat4<float> &m) { m_postClipTransform = m; m_hasPostClipTransform = true; m_hasCombinedTransform = false; }
const math::mat4<float> &Painter::GetPostClipTransform() { return m_postClipTransform; }

void Painter::SetCamera(const jet::scene::Camera *cam) {
    if (!cam) { m_crtCameraIdx = -1; return; }
    if (m_cameras.empty()) m_cameras.push_back(*cam); else m_cameras[0] = *cam;
    m_crtCameraIdx = 0;
}

bool Painter::IsClippingEnabled() { return m_clippingParameters.enabled; }
void Painter::SetClippingEnabled(bool e) { m_clippingParameters.enabled = e; m_clippingParameters.rectMode = true; }
math::rect<float> Painter::GetClippingRectangle() { return m_clippingParameters.clipStack.GetTop(); }
// @0x668c70: the top of the stack is replaced
void Painter::SetClippingRectangle(const math::rect<float> &r) {
    m_clippingParameters.rectMode = true;
    if (m_clippingParameters.clipStack.GetSize() == 0) m_clippingParameters.clipStack.Push(r, true);
    else m_clippingParameters.clipStack.GetTop() = r;
}
void Painter::SetClippingShape(const math::vec2<float> *pts, unsigned n, bool close) {
    m_clippingParameters.rectMode = false;
    m_clippingParameters.rectClipper.SetShape(pts, n, close);
}
const jet::util::ClipStack &Painter::GetClipStack() const { return m_clippingParameters.clipStack; }
jet::util::ClipStack &Painter::GetClipStack() { return m_clippingParameters.clipStack; }

// --- Flush ---------------------------------------------------------------

void Painter::AddLines() {
    for (size_t bi = 0; bi < m_batchInfos.size(); ++bi) {
        const BatchInfo &b = m_batchInfos[bi];
        if (b.operation != Geometry::TopologyDef::Lines) continue;
        unsigned nVerts = b.vertexCount;
        const Vertex *src = reinterpret_cast<const Vertex*>(m_bytecode.data() + b.byteOffset);
        for (unsigned i = 0; i < nVerts; i += m_maxVerticesInBatch ? m_maxVerticesInBatch : nVerts) {
            unsigned count = std::min<unsigned>(m_maxVerticesInBatch ? m_maxVerticesInBatch : nVerts, nVerts - i);
            RenderJobData job;
            job.operation = Geometry::TopologyDef::Lines;
            job.materialIdx = b.materialIdx;
            job.vertices.assign(src + i, src + i + count);
            m_renderJobs.push_back(job);
        }
    }
}

void Painter::AddQuadsAndTris() {
    for (size_t bi = 0; bi < m_batchInfos.size(); ++bi) {
        const BatchInfo &b = m_batchInfos[bi];
        if (b.operation != Geometry::TopologyDef::Triangles) continue;
        unsigned nVerts = b.vertexCount;
        const Vertex *src = reinterpret_cast<const Vertex*>(m_bytecode.data() + b.byteOffset);
        unsigned step = m_maxVerticesInBatch ? (m_maxVerticesInBatch - m_maxVerticesInBatch % 3) : nVerts;
        if (step == 0) step = nVerts;
        for (unsigned i = 0; i < nVerts; i += step) {
            unsigned count = std::min<unsigned>(step, nVerts - i);
            RenderJobData job;
            job.operation = Geometry::TopologyDef::Triangles;
            job.materialIdx = b.materialIdx;
            job.vertices.assign(src + i, src + i + count);
            m_renderJobs.push_back(job);
        }
    }
}

void Painter::FlushList(std::vector<RenderJobData> &jobs, unsigned /*maxVerticesInBatch*/) {
    Driver *driver = Driver::GetCurrent();
    if (!driver) { jobs.clear(); return; }

    for (size_t i = 0; i < jobs.size(); ++i) {
        RenderJobData &jd = jobs[i];
        if (jd.vertices.empty()) continue;
        if (const char *sk = getenv("JET_JOBSKIP")) if ((int)i == atoi(sk) || atoi(sk) < 0) continue;
        if (const char *mx = getenv("JET_JOBMAX")) if ((int)i >= atoi(mx)) continue;
        if (getenv("JET_JOBLOG")) fprintf(stderr, "job %zu/%zu verts %zu mat %d\n", i, jobs.size(), jd.vertices.size(), jd.materialIdx);
        bool isLines = jd.operation == Geometry::TopologyDef::Lines;
        boost::shared_ptr<Geometry> geom = isLines ? m_lineGeometryPool.GetGeometry() : m_triangleGeometryPool.GetGeometry();
        if (!geom) continue;
        geom->Compile();
        int posIdx = geom->FindAttributeIdxBySemantic(Geometry::SemanticDef::Position);
        int colIdx = geom->FindAttributeIdxBySemantic(Geometry::SemanticDef::Color);
        int uvIdx = geom->FindAttributeIdxBySemantic(Geometry::SemanticDef::TexCoord0);
        if (posIdx >= 0) {
            float *p = geom->MapAttributeFloatWrite(posIdx);
            unsigned strideF = geom->GetAttributeStride(posIdx) / sizeof(float);
            for (size_t v = 0; v < jd.vertices.size(); ++v) { float *o = p + v*strideF; o[0] = jd.vertices[v].position.x; o[1] = jd.vertices[v].position.y; o[2] = jd.vertices[v].position.z; }
            geom->UnmapAttribute(posIdx);
        }
        if (colIdx >= 0) {
            float *p = geom->MapAttributeFloatWrite(colIdx);
            unsigned strideF = geom->GetAttributeStride(colIdx) / sizeof(float);
            for (size_t v = 0; v < jd.vertices.size(); ++v) { float *o = p + v*strideF; o[0] = jd.vertices[v].color.r; o[1] = jd.vertices[v].color.g; o[2] = jd.vertices[v].color.b; o[3] = jd.vertices[v].color.a; }
            geom->UnmapAttribute(colIdx);
        }
        if (uvIdx >= 0) {
            float *p = geom->MapAttributeFloatWrite(uvIdx);
            unsigned strideF = geom->GetAttributeStride(uvIdx) / sizeof(float);
            for (size_t v = 0; v < jd.vertices.size(); ++v) { float *o = p + v*strideF; o[0] = jd.vertices[v].uv.x; o[1] = jd.vertices[v].uv.y; }
            geom->UnmapAttribute(uvIdx);
        }

        const Material &mat = (jd.materialIdx >= 0 && jd.materialIdx < (int)m_materials.size()) ? m_materials[jd.materialIdx] : m_material;
        RenderJob job;
        job.SetMaterial(&mat);
        job.SetGeometry(geom);
        job.SetTransform(math::mat4<float>(), true);
        job.SetRange(0, static_cast<unsigned>(jd.vertices.size()));
        driver->DrawPrimitives(job);
    }
    jobs.clear();
}

void Painter::Flush() {
    AddLines();
    AddQuadsAndTris();
    FlushList(m_renderJobs, m_maxVerticesInBatch);
    m_bytecode.clear();
    m_batchInfos.clear();
    m_lastOperation = kOpNone;
    if (!m_materials.empty()) {
        m_materials.erase(m_materials.begin(), m_materials.end() - 1);
        m_crtMaterialIdx = 0;
    }
    m_triangleGeometryPool.Reset();
    m_lineGeometryPool.Reset();
}

void Painter::Unload() {
    Flush();
}

}

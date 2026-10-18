// core/jet/include/video/painter.h
#ifndef CORE_JET_INCLUDE_VIDEO_PAINTER_H
#define CORE_JET_INCLUDE_VIDEO_PAINTER_H

#include <vector>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "jenum.h"
#include "video/color.h"
#include "video/material.h"
#include "video/geometry.h"
#include "video/geometrypool.h"
#include "util/clipstack.h"
#include "util/rectclipper.h"
#include "scene/camera.h"

namespace jet::video {
// Painter's PDB layout (member offsets below are real: bytecode buffer + batch-info list +
// render-job list + two geometry pools) implies immediate-mode 2D drawing that serializes
// vertices into a byte buffer per running batch, then decodes/uploads/draws them on Flush().
// That architecture is reproduced for real here; what's simplified from the original (same
// spirit as GLES20Driver's own documented simplification) is that indices aren't used --
// Lines/Triangles topologies are drawn as sequential non-indexed vertex runs, since Painter's
// primitives never need shared/indexed vertices. Vertex/BatchInfo/RenderJobData/
// ClippingParameters/EndFrameCallback/UnloadResourceCallback aren't declared in the PDB stub
// header at all (only used, never defined) so their field layouts are original to this rebuild.
class Painter {  // sizeof 0x418
public:
    struct Vertex {
        math::vec3<float> position;
        Color color;
        math::vec2<float> uv;
        Vertex() {}
        Vertex(const math::vec2<float> &p, const Color &c) : position(p.x, p.y, 0.0f), color(c) {}
        Vertex(const math::vec3<float> &p, const Color &c, const math::vec2<float> &t = math::vec2<float>(0, 0))
            : position(p), color(c), uv(t) {}
    };
    struct ClippingParameters {
        bool enabled;
        bool rectMode;  // clip against the stack's top rect (else against the shape set by SetClippingShape)
        jet::util::ClipStack clipStack;
        jet::util::RectClipper rectClipper;
        ClippingParameters() : enabled(false), rectMode(true) {}
    };
    struct BatchInfo {
        safe_enum<Geometry::TopologyDef,Geometry::TopologyDef::type> operation;
        unsigned byteOffset;
        unsigned vertexCount;
        int materialIdx;
    };
    struct RenderJobData {
        std::vector<Vertex> vertices;
        safe_enum<Geometry::TopologyDef,Geometry::TopologyDef::type> operation;
        int materialIdx;
        RenderJobData() : operation(Geometry::TopologyDef::Triangles), materialIdx(-1) {}
    };
    typedef boost::function<void()> EndFrameCallback;
    typedef boost::function<void()> UnloadResourceCallback;

    Painter(unsigned short);
    ~Painter();
    bool IsInitialized();
    void DrawLine(const Vertex&, const Vertex&);
    void DrawRectangle(const Vertex&, const Vertex&);
    void FillRectangle(const Vertex&, const Vertex&);
    void DrawQuad(const Vertex&, const Vertex&, const Vertex&, const Vertex&);
    void FillQuad(const Vertex&, const Vertex&, const Vertex&, const Vertex&);
    void DrawTriangle(const Vertex&, const Vertex&, const Vertex&);
    void FillTriangle(const Vertex&, const Vertex&, const Vertex&);
    void DrawTriangleList(const Vertex*, unsigned, const unsigned*, unsigned);
    void FillTriangleList(const Vertex*, unsigned, const unsigned*, unsigned);
    void DrawQuadList(const Vertex*, unsigned, const unsigned*, unsigned);
    void FillQuadList(const Vertex*, unsigned, const unsigned*, unsigned);
    void DrawEllipse(const Vertex&, const math::vec2<float>&);
    void FillEllipse(const Vertex&, const math::vec2<float>&);
    void DrawCircle(const Vertex&, float);
    void FillCircle(const Vertex&, float);
    void DrawArc(const Vertex&, float, float, float);
    void FillArc(const Vertex&, float, float, float);
    void DrawPolyline(const Vertex*, unsigned);
    void DrawPolyline(const std::vector<Vertex>&);
    void FillTriangleFan(const Vertex*, unsigned);
    void FillTriangleFan(const std::vector<Vertex>&);
    void DrawBox(const math::aabb<float>&, unsigned);
    void DrawAxis(const math::quat<float>&, const math::vec3<float>&);
    void DrawAxis(const math::mat4<float>&);
    void FillFullScreenRectangle(unsigned);
    void SetMaterial(const Material&);
    const Material &GetMaterial();
    void SetTransform(const math::mat4<float>&);
    const math::mat4<float> &GetTransform();
    void SetPostClipTransform(const math::mat4<float>&);
    const math::mat4<float> &GetPostClipTransform();
    void SetCamera(const jet::scene::Camera*);
    bool IsClippingEnabled();
    void SetClippingEnabled(bool);
    math::rect<float> GetClippingRectangle();
    void SetClippingRectangle(const math::rect<float>&);
    void SetClippingShape(const math::vec2<float>*, unsigned, bool);
    const jet::util::ClipStack &GetClipStack() const;
    jet::util::ClipStack &GetClipStack();
    void Flush();
    void Unload();
private:
    void DrawLineRaw(const Vertex&, const Vertex&);
    void RecordLine(const Vertex&, const Vertex&);
    void RecordTriangle(const Vertex&, const Vertex&, const Vertex&);
    math::vec3<float> PreClip(const math::vec3<float>&);
    math::vec3<float> PostClip(const math::vec3<float>&);
    void FillTriangleRaw(const Vertex&, const Vertex&, const Vertex&);
    void Init();
    Vertex TransformVertex(const Vertex&);
    void AddLines();
    void AddQuadsAndTris();
    void FlushList(std::vector<RenderJobData>&, unsigned);
    boost::shared_ptr<Geometry> CreateTriangleGeometry();
    boost::shared_ptr<Geometry> CreateLineGeometry();
    ClippingParameters m_clippingParameters;  // 0x0
    EndFrameCallback *m_endFrameCallback;  // 0x154
    UnloadResourceCallback *m_unloadResourceCallback;  // 0x158
    unsigned short m_maxVerticesInBatch;  // 0x15C
    math::mat4<float> m_preClipTransform;  // 0x160
    math::mat4<float> m_postClipTransform;  // 0x1A0
    math::mat4<float> m_combinedTransform;  // 0x1E0
    bool m_hasPreClipTransform;  // 0x220
    bool m_hasPostClipTransform;  // 0x221
    bool m_hasCombinedTransform;  // 0x222
    std::vector<jet::scene::Camera> m_cameras;  // 0x224
    signed char m_crtCameraIdx;  // 0x238
    Material m_material;  // 0x23C
    std::vector<Material> m_materials;  // 0x32C
    int m_crtMaterialIdx;  // 0x340 (a signed char in the original)
    std::vector<unsigned char> m_bytecode;  // 0x344
    unsigned m_bytecodeIdx;  // 0x358
    unsigned m_bytecodeSize;  // 0x35C
    std::vector<BatchInfo> m_batchInfos;  // 0x360
    unsigned m_batchInfosIdx;  // 0x374
    int m_lastOperation;  // 0x378
    std::vector<RenderJobData> m_renderJobs;  // 0x37C
    GeometryPool m_triangleGeometryPool;  // 0x390
    GeometryPool m_lineGeometryPool;  // 0x3D0
    bool m_isInitialized;  // 0x410
    static std::vector<math::vec2<float> > s_cosSinTable;
};
}

#endif

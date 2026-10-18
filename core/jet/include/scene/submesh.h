// core/jet/include/scene/submesh.h
#ifndef CORE_JET_INCLUDE_SCENE_SUBMESH_H
#define CORE_JET_INCLUDE_SCENE_SUBMESH_H

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "core/pstring.h"
#include "stream/istream.h"
#include "video/geometry.h"

namespace jet::scene {
class Mesh;

// A draw call's worth of a Mesh: material name, texture names, and a vertex/index blob kept in
// the model stream and turned into a Geometry on demand (LoadGeometry), after SubMesh in the
// 0.0.3 .so (LoadV100 @0x706f08, LoadAttributes @0x707130).
//
// The blob: u16 header size + header (skipped), then one array per attribute present in the
// attribute mask, each followed by a u16 pad size + padding; then (skinned meshes) node indices
// and weights the same way; then u16 indices. Attribute mask bits follow Geometry::SemanticDef:
// 0 positions, 1 normals, 2..5 tangents, 6 colors, 7..10 texcoords. The format word holds the
// quantization: bits 0-1 positions, 2-3 normals, 4-5 tangents (0 float, 1 S16N x4, 2 S8N x4),
// bits 6..9 texcoord set quantized (S16N x2 instead of float x2), bit 31 hardware skinning.
class SubMesh : private boost::noncopyable {
public:
    struct NodeInfo {  // 0x1c bytes each in the skin blob
        unsigned char data[0x1c];
    };

    explicit SubMesh(Mesh*);
    virtual ~SubMesh();

    const Mesh &GetMesh() const { return *m_mesh; }
    const math::vec3<float> &GetCenter() const { return m_center; }
    unsigned GetTextureCount() const { return (unsigned)m_textureNames.size(); }
    const jet::String &GetTextureName(unsigned i) const { return m_textureNames[i]; }
    const jet::String &GetMaterialName() const { return m_materialName; }
    unsigned GetVertexCount() const { return m_vertexCount; }
    unsigned GetIndexCount() const { return m_indexCount; }
    unsigned GetTriangleCount() const { return m_indexCount / 3; }
    unsigned GetAttributeMask() const { return m_attributeMask; }
    unsigned GetFormat() const { return m_format; }

    unsigned GetPositionsQuantization() const { return m_format & 3; }
    unsigned GetNormalsQuantization() const { return (m_format >> 2) & 3; }
    unsigned GetTangentsQuantization() const { return (m_format >> 4) & 3; }
    bool AreTexCoordsQuantized(unsigned set) const { return (m_format & (1u << (set + 6))) != 0; }
    const math::vec3<float> &GetPositionsQuantizationOffset() const { return m_posQuantOffset; }
    const math::vec3<float> &GetPositionsQuantizationScale() const { return m_posQuantScale; }
    bool IsHWSkinned() const;
    bool IsSWSkinned() const;
    unsigned GetWeightCount() const { return m_weightCount; }
    unsigned GetSkinNodeCount() const { return m_skinNodeCount; }
    const NodeInfo *GetSkinNodeInfos() const { return m_skinNodeInfos; }
    const unsigned char *GetSkinData() const { return m_skinData; }

    // Software skinning (LoadSWSkinData @0x708948, Skin @0x70852c, ComputeVertexSkinning
    // @0x734434): the bind pose decoded to floats (dequantized positions) with the per vertex
    // node indices / weights; the geometry of a software skinned submesh has float position,
    // normal and tangent streams that Skin rewrites in world space every frame.
    struct SWSkinData {
        std::vector<math::vec3<float> > positions, normals;
        std::vector<math::vec4<float> > tangents;
        std::vector<unsigned char> indices;   // 4 per vertex
        std::vector<float> weights;           // 4 per vertex
    };
    const SWSkinData &GetSWSkinData() const { return m_swSkinData; }
    const math::mat4<float> *GetInverseBindMatrices() const {
        return reinterpret_cast<const math::mat4<float>*>(m_skinData + sizeof(NodeInfo) * m_skinNodeCount);
    }
    void Skin(const math::mat4<float> *nodeAbsolute) const;

    const boost::shared_ptr<jet::video::Geometry> &GetGeometry() const;
    bool IsGeometryLoaded() const { return (bool)m_geometry; }
    int GetGeometryVersion() const { return m_geometryVersion; }
    void LoadGeometry() const;
    void UnloadGeometry();

    virtual int Load(jet::stream::IStream&);
    virtual void SetAttributeMask(unsigned);

protected:
    virtual int LoadV100(jet::stream::IStream&);
    void LoadAttributes(unsigned mask) const;
    void LoadSWSkinData(jet::video::Geometry&, const unsigned char *pos, const unsigned char *normal, const unsigned char *tangent,
                        const unsigned char *indices, const unsigned char *weights, unsigned requested) const;
    mutable SWSkinData m_swSkinData;

    Mesh *m_mesh;
    unsigned m_format;
    unsigned m_attributeMask;
    jet::stream::IStream *m_stream;
    unsigned m_dataOffset;
    unsigned m_dataSize;
    unsigned m_requestedMask;
    math::vec3<float> m_center;
    math::vec3<float> m_posQuantOffset;
    math::vec3<float> m_posQuantScale;
    unsigned m_vertexCount;
    unsigned m_indexCount;
    jet::String m_materialName;
    std::vector<jet::String> m_textureNames;
    unsigned short m_weightCount;
    unsigned short m_skinNodeCount;
    unsigned char *m_skinData;
    const NodeInfo *m_skinNodeInfos;
    const unsigned char *m_skinNodeData;
    mutable boost::shared_ptr<jet::video::Geometry> m_geometry;
    mutable int m_geometryVersion;
};
}

#endif

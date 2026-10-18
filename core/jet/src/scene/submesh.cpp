// core/jet/src/scene/submesh.cpp
#include "scene/submesh.h"
#include "scene/mesh.h"
#include "scene/modelbase.h"
#include "mem/memorymanager.h"
#include <cstring>

namespace jet::scene {

using jet::video::Geometry;
typedef Geometry::SemanticDef Sem;
typedef Geometry::TypeDef Type;

// Attribute names, from the static initializer at 0x3d66f8
static const jet::String &AttrName(unsigned semantic) {
    static const jet::String names[] = {
        jet::String("positions"), jet::String("normals"), jet::String("tangents0"), jet::String("tangents1"),
        jet::String("tangents2"), jet::String("tangents3"), jet::String("colors"), jet::String("uv0"),
        jet::String("uv1"), jet::String("uv2"), jet::String("uv3"), jet::String("nodeindices"),
        jet::String("nodeweights") };
    return names[semantic];
}

// ModelBase::s_hwSkinningBoneLimit (0xd13ce0)
unsigned ModelBase::s_hwSkinningBoneLimit = 0;

SubMesh::SubMesh(Mesh *mesh)
    : m_mesh(mesh), m_format(0), m_attributeMask(0), m_stream(0), m_dataOffset(0), m_dataSize(0),
      m_requestedMask(0xffffffff), m_center(0, 0, 0), m_posQuantOffset(0, 0, 0), m_posQuantScale(1, 1, 1),
      m_vertexCount(0), m_indexCount(0), m_weightCount(0), m_skinNodeCount(0), m_skinData(0),
      m_skinNodeInfos(0), m_skinNodeData(0), m_geometryVersion(0) {}

SubMesh::~SubMesh() {
    if (m_skinData) jet::mem::Free_S(m_skinData);
}

// IsHWSkinned @0x708b78 / IsSWSkinned @0x708b94
bool SubMesh::IsHWSkinned() const { return m_mesh->IsSkinned() && (m_format >> 31) != 0; }
bool SubMesh::IsSWSkinned() const { return m_mesh->IsSkinned() && (m_format >> 31) == 0; }

int SubMesh::Load(jet::stream::IStream &s) {
    int version = 0;
    s >> version;
    if (version != 100) return 0;
    return LoadV100(s);
}

// LoadV100 @0x706f08
int SubMesh::LoadV100(jet::stream::IStream &s) {
    m_stream = &s;
    s >> m_format;
    m_format &= ~0x80000000u;
    s >> m_attributeMask;
    s >> m_center;
    if (GetPositionsQuantization()) {
        s >> m_posQuantOffset;
        s >> m_posQuantScale;
    }
    unsigned short vertexCount = 0;
    s >> vertexCount;
    m_vertexCount = vertexCount;
    s >> m_indexCount;
    s >> m_materialName;
    unsigned short textureCount = 0;
    s >> textureCount;
    m_textureNames.resize(textureCount);
    for (unsigned i = 0; i < textureCount; i++) s >> m_textureNames[i];
    unsigned char unused = 0;
    s >> unused;
    s >> m_dataSize;
    m_dataOffset = s.Tell();
    s.SeekBeg(m_dataOffset + m_dataSize);
    if (m_mesh->IsSkinned()) {
        s >> m_weightCount;
        s >> m_skinNodeCount;
        // Hardware skinning when the skeleton fits the shader's bone palette; "meena" models are
        // always skinned in software.
        if (m_skinNodeCount < ModelBase::s_hwSkinningBoneLimit &&
            !strstr(m_mesh->GetModelBase().GetResourceName().c_str(), "meena"))
            m_format |= 0x80000000u;
        unsigned size = 0;
        s >> size;
        m_skinData = (unsigned char*)jet::mem::Malloc_NZ_S(size);
        s.Read(m_skinData, size);
        m_skinNodeInfos = reinterpret_cast<const NodeInfo*>(m_skinData);
        m_skinNodeData = m_skinData + sizeof(NodeInfo) * m_skinNodeCount;
    }
    return 0;
}

void SubMesh::SetAttributeMask(unsigned mask) {
    if (mask == m_requestedMask) return;
    m_requestedMask = mask;
    UnloadGeometry();
}

const boost::shared_ptr<Geometry> &SubMesh::GetGeometry() const {
    LoadGeometry();
    return m_geometry;
}

// LoadGeometry @0x707de0
void SubMesh::LoadGeometry() const {
    if (m_geometry || !m_requestedMask) return;
    LoadAttributes(m_requestedMask);
    ++m_geometryVersion;
}

void SubMesh::UnloadGeometry() { m_geometry.reset(); }

// Copies count elements of elemSize bytes from a packed array into an interleaved buffer.
static void StrideCopy(unsigned char *dst, unsigned dstStride, const unsigned char *src, unsigned srcStride,
                       unsigned elemSize, unsigned count) {
    for (unsigned i = 0; i < count; i++) memcpy(dst + i * dstStride, src + i * srcStride, elemSize);
}

// Each attribute array is followed by a u16 padding size and the padding.
static const unsigned char *SkipArray(const unsigned char *p, unsigned bytes) {
    unsigned short pad;
    memcpy(&pad, p + bytes, 2);
    return p + bytes + 2 + pad;
}

// One component of a quantized array: 0 float, 1 S16N, 2 S8N (x4)
static float Decode(const unsigned char *p, unsigned q, unsigned i) {
    if (q == 0) { float f; memcpy(&f, p + i * 4, 4); return f; }
    if (q == 1) { short v; memcpy(&v, p + i * 2, 2); return v < -32767 ? -1.0f : v / 32767.0f; }
    signed char v = (signed char)p[i];
    return v < -127 ? -1.0f : v / 127.0f;
}

// LoadSWSkinData @0x708948
void SubMesh::LoadSWSkinData(jet::video::Geometry &g, const unsigned char *pos, const unsigned char *normal,
                             const unsigned char *tangent, const unsigned char *indices, const unsigned char *weights,
                             unsigned requested) const {
    const unsigned n = m_vertexCount;
    static const unsigned posSize[] = { 12, 8, 4 }, tanSize[] = { 16, 8, 4 };
    SWSkinData &d = m_swSkinData;
    d.positions.assign(pos ? n : 0, math::vec3<float>(0, 0, 0));
    d.normals.assign(normal ? n : 0, math::vec3<float>(0, 0, 1));
    d.tangents.assign(tangent ? n : 0, math::vec4<float>(1, 0, 0, 1));
    for (unsigned v = 0; pos && v < n; v++) {
        unsigned q = GetPositionsQuantization();
        const unsigned char *e = pos + v * posSize[q];
        d.positions[v] = math::vec3<float>(m_posQuantOffset.x + Decode(e, q, 0) * m_posQuantScale.x,
                                           m_posQuantOffset.y + Decode(e, q, 1) * m_posQuantScale.y,
                                           m_posQuantOffset.z + Decode(e, q, 2) * m_posQuantScale.z);
    }
    for (unsigned v = 0; normal && v < n; v++) {
        unsigned q = GetNormalsQuantization();
        const unsigned char *e = normal + v * posSize[q];
        d.normals[v] = math::vec3<float>(Decode(e, q, 0), Decode(e, q, 1), Decode(e, q, 2));
    }
    for (unsigned v = 0; tangent && v < n; v++) {
        unsigned q = GetTangentsQuantization();
        const unsigned char *e = tangent + v * tanSize[q];
        d.tangents[v] = math::vec4<float>(Decode(e, q, 0), Decode(e, q, 1), Decode(e, q, 2), Decode(e, q, 3));
    }
    d.indices.assign(indices ? indices : (const unsigned char*)0, indices ? indices + 4 * n : (const unsigned char*)0);
    d.weights.resize(4 * n);
    for (unsigned i = 0; i < 4 * n; i++) d.weights[i] = weights ? weights[i] / 255.0f : (i % 4 == 0 ? 1.0f : 0.0f);
    if (m_weightCount <= 1)
        for (unsigned v = 0; v < n; v++) { d.weights[v * 4] = 1; d.weights[v * 4 + 1] = d.weights[v * 4 + 2] = d.weights[v * 4 + 3] = 0; }
    // the bind pose until the first skinning
    int pi = g.FindAttributeIdxBySemantic(Sem::Position);
    if (pi >= 0 && pos && (requested & 1)) {
        unsigned char *dst = g.MapAttributeU8Write(pi);
        for (unsigned v = 0; v < n; v++) memcpy(dst + v * g.GetAttributeStride(pi), &d.positions[v], 12);
        g.UnmapAttribute(pi);
    }
    int ni = g.FindAttributeIdxBySemantic(Sem::Normal);
    if (ni >= 0 && normal && (requested & 2)) {
        unsigned char *dst = g.MapAttributeU8Write(ni);
        for (unsigned v = 0; v < n; v++) memcpy(dst + v * g.GetAttributeStride(ni), &d.normals[v], 12);
        g.UnmapAttribute(ni);
    }
    int ti = g.FindAttributeIdxBySemantic(Sem::Tangent);
    if (ti >= 0 && tangent && (requested & 4)) {
        unsigned char *dst = g.MapAttributeU8Write(ti);
        for (unsigned v = 0; v < n; v++) memcpy(dst + v * g.GetAttributeStride(ti), &d.tangents[v], 16);
        g.UnmapAttribute(ti);
    }
}

// Skin @0x70852c / ComputeVertexSkinning @0x734434: skin matrices = node absolute * inverse
// bind, each vertex the weighted sum of its nodes' transforms of the bind pose
void SubMesh::Skin(const math::mat4<float> *nodeAbsolute) const {
    if (!m_geometry || m_swSkinData.positions.empty()) return;
    std::vector<math::mat4<float> > mats(m_skinNodeCount);
    const math::mat4<float> *invBind = GetInverseBindMatrices();
    for (unsigned i = 0; i < m_skinNodeCount; i++) mats[i] = nodeAbsolute[i] * invBind[i];
    jet::video::Geometry &g = *m_geometry;
    const SWSkinData &d = m_swSkinData;
    const unsigned n = m_vertexCount;
    int pi = g.FindAttributeIdxBySemantic(Sem::Position), ni = g.FindAttributeIdxBySemantic(Sem::Normal),
        ti = g.FindAttributeIdxBySemantic(Sem::Tangent);
    unsigned char *pd = pi >= 0 ? g.MapAttributeU8Write(pi) : 0;
    unsigned char *nd = ni >= 0 && !d.normals.empty() ? g.MapAttributeU8Write(ni) : 0;
    unsigned char *td = ti >= 0 && !d.tangents.empty() ? g.MapAttributeU8Write(ti) : 0;
    const unsigned ps = pd ? g.GetAttributeStride(pi) : 0, ns = nd ? g.GetAttributeStride(ni) : 0, ts = td ? g.GetAttributeStride(ti) : 0;
    for (unsigned v = 0; v < n; v++) {
        float m[16] = { 0 };
        for (unsigned k = 0; k < 4; k++) {
            float w = d.weights[v * 4 + k];
            if (w <= 0) continue;
            unsigned idx = d.indices.empty() ? 0 : d.indices[v * 4 + k];
            if (idx >= m_skinNodeCount) continue;
            const float *s = mats[idx].m;
            for (int e = 0; e < 16; e++) m[e] += s[e] * w;
        }
        if (pd) {
            const math::vec3<float> &p = d.positions[v];
            float o[3] = { m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12], m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13],
                           m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14] };
            memcpy(pd + v * ps, o, 12);
        }
        if (nd) {
            const math::vec3<float> &p = d.normals[v];
            float o[3] = { m[0] * p.x + m[4] * p.y + m[8] * p.z, m[1] * p.x + m[5] * p.y + m[9] * p.z, m[2] * p.x + m[6] * p.y + m[10] * p.z };
            memcpy(nd + v * ns, o, 12);
        }
        if (td) {
            const math::vec4<float> &p = d.tangents[v];
            float o[4] = { m[0] * p.x + m[4] * p.y + m[8] * p.z, m[1] * p.x + m[5] * p.y + m[9] * p.z, m[2] * p.x + m[6] * p.y + m[10] * p.z, p.w };
            memcpy(td + v * ts, o, 16);
        }
    }
    if (pd) g.UnmapAttribute(pi);
    if (nd) g.UnmapAttribute(ni);
    if (td) g.UnmapAttribute(ti);
}

// LoadAttributes @0x707130
void SubMesh::LoadAttributes(unsigned requested) const {
    jet::stream::IStream &s = *m_stream;
    bool started = s.IsStarted();
    if (!started) s.Start();
    s.SeekBeg(m_dataOffset);
    unsigned short headerSize = 0;
    s >> headerSize;
    s.SeekCur(headerSize);
    const unsigned char *p = static_cast<const unsigned char*>(s.Map(m_dataSize - 2 - headerSize));

    boost::shared_ptr<Geometry> g = Geometry::New();
    g->SetVertexCount(m_vertexCount);
    g->SetIndexCount(m_indexCount);
    g->SetTopology(Geometry::TopologyDef::Triangles);
    g->SetIndexUsage(Geometry::UsageDef::Static);

    const unsigned mask = requested & m_attributeMask;
    const bool swSkinned = IsSWSkinned();
    const bool skinned = m_mesh->IsSkinned();
    // software-skinned meshes rewrite positions/normals/tangents every frame from the skin data
    const safe_enum<Geometry::UsageDef,Geometry::UsageDef::type> dynUsage = swSkinned ? Geometry::UsageDef::Stream : Geometry::UsageDef::Static;

    static const unsigned quantType[] = { Type::Float, Type::S16N, Type::S8N };
    static const unsigned quantComponents[] = { 3, 4, 4 };
    // software skinned streams hold the skinned floats (AcquireDynamicGeometryData @0x707ebc)
    if (mask & (1u << Sem::Position)) {
        unsigned q = GetPositionsQuantization();
        if (swSkinned) g->AddAttribute(AttrName(Sem::Position), Sem::Position, dynUsage, Type::Float, 3);
        else g->AddAttribute(AttrName(Sem::Position), Sem::Position, dynUsage, (Type::type)quantType[q], quantComponents[q]);
    }
    if (mask & (1u << Sem::Normal)) {
        unsigned q = GetNormalsQuantization();
        if (swSkinned) g->AddAttribute(AttrName(Sem::Normal), Sem::Normal, dynUsage, Type::Float, 3);
        else g->AddAttribute(AttrName(Sem::Normal), Sem::Normal, dynUsage, (Type::type)quantType[q], quantComponents[q]);
    }
    for (unsigned t = Sem::Tangent; t <= Sem::Tangent3; t++)
        if (mask & (1u << t)) {
            unsigned q = GetTangentsQuantization();
            g->AddAttribute(AttrName(t), (Sem::type)t, dynUsage, swSkinned ? Type::Float : (Type::type)quantType[q], 4);
        }
    if (mask & (1u << Sem::Color))
        g->AddAttribute(AttrName(Sem::Color), Sem::Color, Geometry::UsageDef::Static, Type::U8N, 4);
    for (unsigned t = Sem::TexCoord0; t <= Sem::TexCoord3; t++)
        if (mask & (1u << t))
            g->AddAttribute(AttrName(t), (Sem::type)t, Geometry::UsageDef::Static,
                            AreTexCoordsQuantized(t - Sem::TexCoord0) ? Type::S16N : Type::Float, 2);
    if (IsHWSkinned()) {
        g->AddAttribute(AttrName(Sem::NodeIndices), Sem::NodeIndices, Geometry::UsageDef::Static, Type::U8, 4);
        if (m_weightCount > 1)
            g->AddAttribute(AttrName(Sem::NodeWeights), Sem::NodeWeights, Geometry::UsageDef::Static, Type::U8N, 4);
    }
    g->Compile();

    const unsigned n = m_vertexCount;
    static const unsigned posSize[] = { 12, 8, 4 };
    static const unsigned tanSize[] = { 16, 8, 4 };
    // Copy one attribute array into the geometry, then step over it. Arrays present in the file
    // but not requested are skipped.
    auto take = [&](unsigned semantic, unsigned elemSize, bool wanted) {
        if (wanted) {
            int idx = g->FindAttributeIdxBySemantic((Sem::type)semantic);
            unsigned char *dst = g->MapAttributeU8Write(idx);
            StrideCopy(dst, g->GetAttributeStride(idx), p, elemSize, elemSize, n);
            g->UnmapAttribute(idx);
        }
        p = SkipArray(p, elemSize * n);
    };

    const unsigned char *posData = 0, *normalData = 0, *tangentData = 0;
    if (m_attributeMask & (1u << Sem::Position)) {
        posData = p;
        take(Sem::Position, posSize[GetPositionsQuantization()], (requested & 1) != 0);
    }
    if (m_attributeMask & (1u << Sem::Normal)) {
        normalData = p;
        take(Sem::Normal, posSize[GetNormalsQuantization()], (requested & 2) != 0);
    }
    for (unsigned t = Sem::Tangent; t <= Sem::Tangent3; t++)
        if (m_attributeMask & (1u << t)) {
            if (t == Sem::Tangent) tangentData = p;
            take(t, tanSize[GetTangentsQuantization()], (requested & (1u << t)) != 0);
        }
    if (m_attributeMask & (1u << Sem::Color)) take(Sem::Color, 4, (requested & (1u << Sem::Color)) != 0);
    for (unsigned t = Sem::TexCoord0; t <= Sem::TexCoord3; t++)
        if (m_attributeMask & (1u << t))
            take(t, AreTexCoordsQuantized(t - Sem::TexCoord0) ? 4 : 8, (requested & (1u << t)) != 0);

    const unsigned char *indexData = 0, *weightData = 0;
    if (IsHWSkinned()) {
        take(Sem::NodeIndices, 4, true);
        if (m_weightCount > 1) take(Sem::NodeWeights, 4, true);
        else p = SkipArray(p, 4 * n);
    } else if (skinned) {
        indexData = p;
        p = SkipArray(p, 4 * n);
        weightData = p;
        p = SkipArray(p, 4 * n);
    }
    if (swSkinned) LoadSWSkinData(*g, posData, normalData, tangentData, indexData, weightData, requested);

    unsigned short *idx = g->MapIndicesWrite();
    memcpy(idx, p, m_indexCount * 2);
    g->UnmapAll();

    s.Unmap();
    if (!started) s.Finish();
    m_geometry = g;
}

}

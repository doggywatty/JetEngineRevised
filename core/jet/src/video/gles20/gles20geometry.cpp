// core/jet/src/video/gles20/gles20geometry.cpp
#include "video/gles20/gles20geometry.h"
#include "video/renderjob.h"
#include "video/material.h"
#include "video/rendertechnique.h"
#include "video/gles20/gles20shaderprogramflavor.h"
#include <cstring>
#include <algorithm>

namespace jet::video {

// AddAttribute's element size table (0xc95b8e in the 0.0.3 .so)
static unsigned ElementSize(safe_enum<Geometry::TypeDef,Geometry::TypeDef::type> t) {
    static const unsigned char sizes[] = { 4, 1, 2, 1, 1, 2, 2 };
    unsigned i = (unsigned char)(Geometry::TypeDef::type)t;
    return i < 7 ? sizes[i] : 0;
}
// _BindAttributes' switch (0x6bb7f4): GL type and normalized flag per TypeDef
static unsigned GLType(safe_enum<Geometry::TypeDef,Geometry::TypeDef::type> t) {
    static const unsigned types[] = { 0x1406 /*FLOAT*/, 0x1401 /*UNSIGNED_BYTE*/, 0x1402 /*SHORT*/, 0x1400 /*BYTE*/,
                                      0x1401 /*UNSIGNED_BYTE*/, 0x1402 /*SHORT*/, 0x1403 /*UNSIGNED_SHORT*/ };
    unsigned i = (unsigned char)(Geometry::TypeDef::type)t;
    return i < 7 ? types[i] : 0x1406;
}
static bool GLNormalized(safe_enum<Geometry::TypeDef,Geometry::TypeDef::type> t) {
    return (unsigned char)(Geometry::TypeDef::type)t >= Geometry::TypeDef::S8N;
}
// Verified against GLES20Geometry::Draw's mode table (0x6322dc in the 0.0.3 .so).
static unsigned GLTopology(safe_enum<Geometry::TopologyDef,Geometry::TopologyDef::type> t) {
    static const unsigned table[] = { 0x1 /*LINES*/, 0x2 /*LINE_LOOP*/, 0x3 /*LINE_STRIP*/,
                                       0x4 /*TRIANGLES*/, 0x5 /*TRIANGLE_STRIP*/, 0x6 /*TRIANGLE_FAN*/ };
    return table[(unsigned char)(Geometry::TopologyDef::type)t];
}

unsigned GLES20Geometry::s_nextUID = 1;

GLES20Geometry::GLES20Geometry()
    : m_uid(s_nextUID++), m_topology(Geometry::TopologyDef::Triangles), m_vertexCount(0), m_indexCount(0),
      m_indexUsage(Geometry::UsageDef::Static), m_hasStaticAttributes(false), m_hasStreamedAttributes(false),
      m_isCompiled(false), m_staticStride(0), m_streamStride(0), m_staticVbo(0), m_streamVbo(0),
      m_staticDirty(false), m_streamDirty(false), m_indexVbo(0), m_indexDirty(false),
      m_lastRenderedFrameIdx(0xffffffff), m_lockCount(0) {
    for (unsigned i = 0; i < sizeof(m_semanticAttributeIdx) / sizeof(int); i++) m_semanticAttributeIdx[i] = -1;
}

GLES20Geometry::~GLES20Geometry() {
    gles::Interface gl;
    if (m_staticVbo) gl.iglDeleteBuffers(1, &m_staticVbo);
    if (m_streamVbo) gl.iglDeleteBuffers(1, &m_streamVbo);
    if (m_indexVbo) gl.iglDeleteBuffers(1, &m_indexVbo);
}

unsigned GLES20Geometry::GetUID() { return m_uid; }

void GLES20Geometry::SetTopology(safe_enum<Geometry::TopologyDef,Geometry::TopologyDef::type> t) { if (!m_isCompiled) m_topology = t; }
safe_enum<Geometry::TopologyDef,Geometry::TopologyDef::type> GLES20Geometry::GetTopology() { return m_topology; }

// a compiled geometry can only use less of its vertices (the dynamic geometries of the particles: only the used ones are uploaded)
void GLES20Geometry::SetVertexCount(unsigned c) { if (!m_isCompiled || c <= m_allocatedVertexCount) m_vertexCount = c; }
unsigned GLES20Geometry::GetVertexCount() { return m_vertexCount; }
void GLES20Geometry::SetIndexCount(unsigned c) { if (!m_isCompiled) m_indexCount = c; }
unsigned GLES20Geometry::GetIndexCount() { return m_indexCount; }

unsigned GLES20Geometry::GetPrimitiveCount(unsigned indexCount) {
    switch ((Geometry::TopologyDef::type)m_topology) {
    case Geometry::TopologyDef::Lines: return indexCount / 2;
    case Geometry::TopologyDef::LineLoop: return indexCount;
    case Geometry::TopologyDef::LineStrip: return indexCount ? indexCount - 1 : 0;
    case Geometry::TopologyDef::Triangles: return indexCount / 3;
    case Geometry::TopologyDef::TriangleStrip: case Geometry::TopologyDef::TriangleFan:
        return indexCount >= 2 ? indexCount - 2 : 0;
    default: return 0;
    }
}
unsigned GLES20Geometry::GetPrimitiveCount() { return GetPrimitiveCount(m_indexCount ? m_indexCount : m_vertexCount); }

void GLES20Geometry::SetIndexUsage(safe_enum<Geometry::UsageDef,Geometry::UsageDef::type> u) { m_indexUsage = u; }
safe_enum<Geometry::UsageDef,Geometry::UsageDef::type> GLES20Geometry::GetIndexUsage() { return m_indexUsage; }

void GLES20Geometry::LinkIndices(boost::shared_ptr<Geometry> g) { m_linkedIndices = g; }
void GLES20Geometry::UnlinkIndices() { m_linkedIndices.reset(); }

int GLES20Geometry::AddAttribute(const jet::String &name, safe_enum<Geometry::SemanticDef,Geometry::SemanticDef::type> sem,
                                  safe_enum<Geometry::UsageDef,Geometry::UsageDef::type> usage,
                                  safe_enum<Geometry::TypeDef,Geometry::TypeDef::type> type, unsigned components) {
    if (FindAttributeIdxByName(name) >= 0 || components == 0) return -1;
    AttributeData ad;
    ad.attr.name = name;
    ad.attr.semantic = sem;
    ad.attr.usage = usage;
    ad.attr.type = type;
    ad.attr.components = components;
    ad.attr.enabled = true;
    ad.attr.texCoordUsedAreaHint = 1.0f;
    ad.elementSize = ElementSize(type);
    ad.offset = 0;
    if (usage == Geometry::UsageDef::Static) m_hasStaticAttributes = true;
    else m_hasStreamedAttributes = true;
    m_attributeData.push_back(ad);
    unsigned idx = (unsigned)m_attributeData.size() - 1;
    m_semanticAttributeIdx[(unsigned char)(Geometry::SemanticDef::type)sem] = idx;
    return (int)idx;
}

void GLES20Geometry::LinkAttribute(const jet::String &name, safe_enum<Geometry::SemanticDef,Geometry::SemanticDef::type> sem,
                                    boost::shared_ptr<Geometry> other, const jet::String &otherName) {
    AttributeData ad;
    ad.attr.name = name;
    ad.attr.semantic = sem;
    ad.attr.enabled = true;
    ad.attr.texCoordUsedAreaHint = 1.0f;
    ad.linked = other;
    ad.linkedName = otherName;
    if (other) {
        int idx = other->FindAttributeIdxByName(otherName);
        if (idx >= 0) {
            const Geometry::Attribute &src = other->GetAttribute(idx);
            ad.attr.usage = src.usage;
            ad.attr.type = src.type;
            ad.attr.components = src.components;
            ad.elementSize = ElementSize(src.type);
        }
    }
    m_attributeData.push_back(ad);
    unsigned idx = (unsigned)m_attributeData.size() - 1;
    m_semanticAttributeIdx[(unsigned char)(Geometry::SemanticDef::type)sem] = idx;
}

int GLES20Geometry::FindAttributeIdxByName(const jet::String &name) {
    for (unsigned i = 0; i < m_attributeData.size(); i++)
        if (m_attributeData[i].attr.name.Equals(name)) return (int)i;
    return -1;
}
int GLES20Geometry::FindAttributeIdxBySemantic(safe_enum<Geometry::SemanticDef,Geometry::SemanticDef::type> sem) {
    return m_semanticAttributeIdx[(unsigned char)(Geometry::SemanticDef::type)sem];
}

void GLES20Geometry::SetAttributeEnabled(unsigned i, bool v) { m_attributeData[i].attr.enabled = v; }
bool GLES20Geometry::HasStaticAttributes() { return m_hasStaticAttributes; }
bool GLES20Geometry::HasStreamedAttributes() { return m_hasStreamedAttributes; }
void GLES20Geometry::SetTexCoordAttributeUsedAreaHint(unsigned i, float v) { m_attributeData[i].attr.texCoordUsedAreaHint = v; }
float GLES20Geometry::GetTexCoordAttributeUsedAreaHint(unsigned i) { return m_attributeData[i].attr.texCoordUsedAreaHint; }

void GLES20Geometry::Compile() {
    if (m_isCompiled) return;
    m_staticStride = 0;
    m_streamStride = 0;
    for (unsigned i = 0; i < m_attributeData.size(); i++) {
        AttributeData &ad = m_attributeData[i];
        if (ad.linked) continue;
        unsigned size = ad.elementSize * ad.attr.components;
        if (ad.attr.usage == Geometry::UsageDef::Static) { ad.offset = m_staticStride; m_staticStride += size; }
        else { ad.offset = m_streamStride; m_streamStride += size; }
    }
    if (m_hasStaticAttributes) m_staticData.resize((size_t)m_staticStride * m_vertexCount, 0);
    if (m_hasStreamedAttributes) m_streamData.resize((size_t)m_streamStride * m_vertexCount, 0);
    if (m_indexCount && !m_linkedIndices) m_indexData.resize((size_t)m_indexCount * 2, 0);
    m_allocatedVertexCount = m_vertexCount;
    m_isCompiled = true;
    m_staticDirty = m_hasStaticAttributes;
    m_streamDirty = m_hasStreamedAttributes;
    m_indexDirty = m_indexCount != 0;
}

void GLES20Geometry::EnsureBuffer(bool stream, unsigned bytes) {
    gles::Interface gl;
    unsigned &vbo = stream ? m_streamVbo : m_staticVbo;
    if (!vbo) gl.iglGenBuffers(1, &vbo);
    gl.iglBindBuffer(0x8892 /*GL_ARRAY_BUFFER*/, vbo);
    gl.iglBufferData(0x8892, bytes, 0, stream ? 0x88E0 /*GL_STREAM_DRAW*/ : 0x88E4 /*GL_STATIC_DRAW*/);
}

const GLES20Geometry::AttributeData &GLES20Geometry::RetrieveAttributeData(unsigned i) {
    const AttributeData *ad = &m_attributeData[i];
    while (ad->linked) {
        GLES20Geometry *g = static_cast<GLES20Geometry*>(ad->linked.get());
        int idx = g->FindAttributeIdxByName(ad->linkedName);
        ad = idx >= 0 ? &g->m_attributeData[idx] : ad;
        if (idx < 0) break;
        if (!g->m_attributeData[idx].linked) return g->m_attributeData[idx];
    }
    return *ad;
}

unsigned char *GLES20Geometry::MapAttributeInternal(unsigned i, bool write) {
    Compile();
    const AttributeData &ad = m_attributeData[i];
    if (ad.linked) {
        GLES20Geometry *g = static_cast<GLES20Geometry*>(ad.linked.get());
        int idx = g->FindAttributeIdxByName(ad.linkedName);
        return idx >= 0 ? g->MapAttributeInternal(idx, write) : 0;
    }
    std::vector<unsigned char> &buf = ad.attr.usage == Geometry::UsageDef::Static ? m_staticData : m_streamData;
    unsigned stride = ad.attr.usage == Geometry::UsageDef::Static ? m_staticStride : m_streamStride;
    if (write) { if (ad.attr.usage == Geometry::UsageDef::Static) m_staticDirty = true; else m_streamDirty = true; }
    (void)stride;
    return buf.empty() ? 0 : &buf[ad.offset];
}

bool GLES20Geometry::UnmapAttributeInternal(unsigned i) {
    const AttributeData &ad = m_attributeData[i];
    if (ad.linked) return true;
    gles::Interface gl;
    if (ad.attr.usage == Geometry::UsageDef::Static && m_staticDirty) {
        EnsureBuffer(false, (unsigned)m_staticData.size());
        gl.iglBufferSubData(0x8892, 0, (int)m_staticData.size(), m_staticData.empty() ? 0 : &m_staticData[0]);
        m_staticDirty = false;
    } else if (ad.attr.usage != Geometry::UsageDef::Static && m_streamDirty) {
        const unsigned used = std::min((unsigned)m_streamData.size(), m_streamStride * m_vertexCount);
        EnsureBuffer(true, used);
        gl.iglBufferSubData(0x8892, 0, (int)used, m_streamData.empty() ? 0 : &m_streamData[0]);
        m_streamDirty = false;
    }
    return true;
}

unsigned short *GLES20Geometry::MapIndicesInternal(bool write) {
    Compile();
    if (m_linkedIndices) {
        return write ? 0 : const_cast<unsigned short*>(m_linkedIndices->MapIndicesRead());
    }
    if (write) m_indexDirty = true;
    return m_indexData.empty() ? 0 : reinterpret_cast<unsigned short*>(&m_indexData[0]);
}
bool GLES20Geometry::UnmapIndicesInternal() {
    if (m_linkedIndices || !m_indexDirty) return true;
    gles::Interface gl;
    if (!m_indexVbo) gl.iglGenBuffers(1, &m_indexVbo);
    gl.iglBindBuffer(0x8893 /*GL_ELEMENT_ARRAY_BUFFER*/, m_indexVbo);
    gl.iglBufferData(0x8893, (int)m_indexData.size(), m_indexData.empty() ? 0 : &m_indexData[0], 0x88E4);
    m_indexDirty = false;
    return true;
}

const unsigned short *GLES20Geometry::MapIndicesRead() { return MapIndicesInternal(false); }
unsigned short *GLES20Geometry::MapIndicesWrite() { return MapIndicesInternal(true); }
void GLES20Geometry::UnmapIndices() { UnmapIndicesInternal(); }

const float *GLES20Geometry::MapAttributeFloatRead(unsigned i) { return reinterpret_cast<const float*>(MapAttributeInternal(i, false)); }
float *GLES20Geometry::MapAttributeFloatWrite(unsigned i) { return reinterpret_cast<float*>(MapAttributeInternal(i, true)); }
const signed char *GLES20Geometry::MapAttributeS8Read(unsigned i) { return reinterpret_cast<const signed char*>(MapAttributeInternal(i, false)); }
signed char *GLES20Geometry::MapAttributeS8Write(unsigned i) { return reinterpret_cast<signed char*>(MapAttributeInternal(i, true)); }
const unsigned char *GLES20Geometry::MapAttributeU8Read(unsigned i) { return MapAttributeInternal(i, false); }
unsigned char *GLES20Geometry::MapAttributeU8Write(unsigned i) { return MapAttributeInternal(i, true); }
const short *GLES20Geometry::MapAttributeS16Read(unsigned i) { return reinterpret_cast<const short*>(MapAttributeInternal(i, false)); }
short *GLES20Geometry::MapAttributeS16Write(unsigned i) { return reinterpret_cast<short*>(MapAttributeInternal(i, true)); }
const unsigned short *GLES20Geometry::MapAttributeU16Read(unsigned i) { return reinterpret_cast<const unsigned short*>(MapAttributeInternal(i, false)); }
unsigned short *GLES20Geometry::MapAttributeU16Write(unsigned i) { return reinterpret_cast<unsigned short*>(MapAttributeInternal(i, true)); }
void GLES20Geometry::UnmapAttribute(unsigned i) { UnmapAttributeInternal(i); }

void GLES20Geometry::UnmapAll() {
    for (unsigned i = 0; i < m_attributeData.size(); i++) UnmapAttributeInternal(i);
    UnmapIndicesInternal();
}

unsigned GLES20Geometry::GetAttributeCount() { return (unsigned)m_attributeData.size(); }
const Geometry::Attribute &GLES20Geometry::GetAttribute(unsigned i) { return m_attributeData[i].attr; }

unsigned GLES20Geometry::GetLastRenderedFrameIdx() { return m_lastRenderedFrameIdx; }
bool GLES20Geometry::IsLocked() { return m_lockCount > 0; }
unsigned GLES20Geometry::GetAttributeStride(unsigned i) {
    Compile();
    const AttributeData &ad = RetrieveAttributeData(i);
    return ad.attr.usage == Geometry::UsageDef::Static ? m_staticStride : m_streamStride;
}
void GLES20Geometry::SetLocked(bool v) { m_lockCount += v ? 1 : -1; }

// After GLES20Geometry::_BindAttributes (0x6bb670): walk the program's attributes (location i is
// bound to the pass's i-th attribute at link time) and find the geometry attribute for each by
// semantic, Custom ones by name. A missing Tangent1..3 falls back to the first tangent set and a
// missing TexCoord1..3 to the first texcoord set.
bool GLES20Geometry::BindAttributes(gles::Interface &gl, const GLES20ShaderProgramFlavor &flavor) {
    Compile();
    UnmapAll();
    GLES20ShaderProgramFlavor &fl = const_cast<GLES20ShaderProgramFlavor&>(flavor);
    for (unsigned loc = 0; loc < fl.GetAttributeCount(); loc++) {
        const ShaderAttribute &sa = fl.GetAttribute(loc);
        unsigned sem = (unsigned char)(Geometry::SemanticDef::type)sa.GetType();
        int idx;
        if (sem == Geometry::SemanticDef::Custom) idx = FindAttributeIdxByName(sa.GetAttributeName());
        else {
            idx = m_semanticAttributeIdx[sem];
            if (idx < 0 && sem > Geometry::SemanticDef::Tangent && sem <= Geometry::SemanticDef::Tangent3)
                idx = m_semanticAttributeIdx[Geometry::SemanticDef::Tangent];
            else if (idx < 0 && sem > Geometry::SemanticDef::TexCoord0 && sem <= Geometry::SemanticDef::TexCoord3)
                idx = m_semanticAttributeIdx[Geometry::SemanticDef::TexCoord0];
        }
        if (idx < 0 || !m_attributeData[idx].attr.enabled) { gl.iglDisableVertexAttribArray(loc); continue; }
        const AttributeData &ad = RetrieveAttributeData(idx);
        const GLES20Geometry *owner = m_attributeData[idx].linked ? static_cast<GLES20Geometry*>(m_attributeData[idx].linked.get()) : this;
        unsigned vbo = ad.attr.usage == Geometry::UsageDef::Static ? owner->m_staticVbo : owner->m_streamVbo;
        unsigned stride = ad.attr.usage == Geometry::UsageDef::Static ? owner->m_staticStride : owner->m_streamStride;
        if (!vbo) { gl.iglDisableVertexAttribArray(loc); continue; }
        gl.iglBindBuffer(0x8892, vbo);
        gl.iglEnableVertexAttribArray(loc);
        gl.iglVertexAttribPointer(loc, ad.attr.components, GLType(ad.attr.type), GLNormalized(ad.attr.type), stride,
                                   reinterpret_cast<const void*>((size_t)ad.offset));
    }
    return true;
}
void GLES20Geometry::UnbindAttributes(gles::Interface &gl, const GLES20ShaderProgramFlavor &flavor) {
    GLES20ShaderProgramFlavor &fl = const_cast<GLES20ShaderProgramFlavor&>(flavor);
    for (unsigned i = 0; i < fl.GetAttributeCount(); i++) gl.iglDisableVertexAttribArray(i);
}

void GLES20Geometry::Draw(const RenderJob &job, const Material&, unsigned, const GLES20ShaderProgramFlavor &flavor) {
    gles::Interface gl;
    BindAttributes(gl, flavor);
    unsigned mode = GLTopology(m_topology);
    if (m_indexCount) {
        if (m_linkedIndices) m_linkedIndices->MapIndicesRead();
        else UnmapIndicesInternal();
        unsigned vbo = m_linkedIndices ? static_cast<GLES20Geometry*>(m_linkedIndices.get())->m_indexVbo : m_indexVbo;
        gl.iglBindBuffer(0x8893, vbo);
        unsigned count = job.GetRangeCount() ? job.GetRangeCount() : m_indexCount;
        gl.iglDrawElements(mode, count, 0x1403 /*GL_UNSIGNED_SHORT*/, (const void*)(uintptr_t)(job.GetRangeStart() * 2));
    } else {
        gl.iglDrawArrays(mode, job.GetRangeStart(), job.GetRangeCount() ? job.GetRangeCount() : m_vertexCount);
    }
    UnbindAttributes(gl, flavor);
    m_lastRenderedFrameIdx = 0;  // set from the driver's frame counter once it exists
}

}

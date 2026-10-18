// core/jet/include/video/gles20/gles20geometry.h
#ifndef CORE_JET_INCLUDE_VIDEO_GLES20_GLES20GEOMETRY_H
#define CORE_JET_INCLUDE_VIDEO_GLES20_GLES20GEOMETRY_H

#include <vector>
#include "video/geometry.h"
#include "video/gles20/gles20interface.h"

namespace jet::video {
class RenderJob;
class Material;
class RenderTechnique;
class GLES20ShaderProgramFlavor;

// A real (not simulated) GLES2 vertex/index buffer geometry. Static and streamed attributes are
// each packed into their own CPU shadow buffer at Compile() time and uploaded to their own VBO;
// Map*/Unmap* read or write that shadow buffer and re-upload via glBufferSubData on unmap. This
// is a functionally complete rewrite of GLES20Geometry's buffer management, not a byte-for-byte
// port of the original's adaptive client-array-vs-glMapBuffer heuristic (0x64131c in the 0.0.3
// .so) -- that heuristic only affects performance, not what ends up bound and drawn.
class GLES20Geometry : public Geometry {
public:
    struct AttributeData {
        Geometry::Attribute attr;
        unsigned offset;       // byte offset of this attribute within its VBO's vertex stride
        unsigned elementSize;  // bytes for one component value (1/2/4)
        boost::shared_ptr<Geometry> linked;  // set when LinkAttribute'd to another geometry
        jet::String linkedName;
    };

    GLES20Geometry();
    virtual ~GLES20Geometry();
    virtual unsigned GetUID();
    virtual void SetTopology(safe_enum<Geometry::TopologyDef,Geometry::TopologyDef::type>);
    virtual safe_enum<Geometry::TopologyDef,Geometry::TopologyDef::type> GetTopology();
    virtual void SetVertexCount(unsigned);
    virtual unsigned GetVertexCount();
    virtual void SetIndexCount(unsigned);
    virtual unsigned GetIndexCount();
    unsigned GetPrimitiveCount(unsigned indexCount);
    virtual unsigned GetPrimitiveCount();
    virtual void SetIndexUsage(safe_enum<Geometry::UsageDef,Geometry::UsageDef::type>);
    virtual safe_enum<Geometry::UsageDef,Geometry::UsageDef::type> GetIndexUsage();
    virtual void LinkIndices(boost::shared_ptr<Geometry>);
    virtual void UnlinkIndices();
    virtual int AddAttribute(const jet::String&, safe_enum<Geometry::SemanticDef,Geometry::SemanticDef::type>, safe_enum<Geometry::UsageDef,Geometry::UsageDef::type>, safe_enum<Geometry::TypeDef,Geometry::TypeDef::type>, unsigned);
    virtual void LinkAttribute(const jet::String&, safe_enum<Geometry::SemanticDef,Geometry::SemanticDef::type>, boost::shared_ptr<Geometry>, const jet::String&);
    virtual int FindAttributeIdxByName(const jet::String&);
    virtual int FindAttributeIdxBySemantic(safe_enum<Geometry::SemanticDef,Geometry::SemanticDef::type>);
    virtual void SetAttributeEnabled(unsigned, bool);
    virtual bool HasStaticAttributes();
    virtual bool HasStreamedAttributes();
    virtual void SetTexCoordAttributeUsedAreaHint(unsigned, float);
    virtual float GetTexCoordAttributeUsedAreaHint(unsigned);
    virtual void Compile();
    virtual unsigned GetAttributeCount();
    virtual const Geometry::Attribute &GetAttribute(unsigned);
    virtual const unsigned short *MapIndicesRead();
    virtual unsigned short* MapIndicesWrite();
    virtual void UnmapIndices();
    virtual const float *MapAttributeFloatRead(unsigned);
    virtual float* MapAttributeFloatWrite(unsigned);
    virtual const signed char *MapAttributeS8Read(unsigned);
    virtual signed char* MapAttributeS8Write(unsigned);
    virtual const unsigned char *MapAttributeU8Read(unsigned);
    virtual unsigned char* MapAttributeU8Write(unsigned);
    virtual const short *MapAttributeS16Read(unsigned);
    virtual short* MapAttributeS16Write(unsigned);
    virtual const unsigned short *MapAttributeU16Read(unsigned);
    virtual unsigned short* MapAttributeU16Write(unsigned);
    virtual void UnmapAttribute(unsigned);
    virtual void UnmapAll();
    virtual unsigned GetLastRenderedFrameIdx();
    virtual bool IsLocked();
    virtual unsigned GetAttributeStride(unsigned);
protected:
    virtual void SetLocked(bool);
public:
    void Draw(const RenderJob&, const Material&, unsigned, const GLES20ShaderProgramFlavor&);
private:
    bool BindAttributes(gles::Interface&, const GLES20ShaderProgramFlavor&);
    void UnbindAttributes(gles::Interface&, const GLES20ShaderProgramFlavor&);
    unsigned char* MapAttributeInternal(unsigned, bool write);
    bool UnmapAttributeInternal(unsigned);
    unsigned short* MapIndicesInternal(bool write);
    bool UnmapIndicesInternal();
    const AttributeData &RetrieveAttributeData(unsigned);
    void EnsureBuffer(bool stream, unsigned bytes);

    static unsigned s_nextUID;
    unsigned m_uid;
    safe_enum<Geometry::TopologyDef,Geometry::TopologyDef::type> m_topology;
    unsigned m_vertexCount;
    unsigned m_allocatedVertexCount = 0;
    unsigned m_indexCount;
    safe_enum<Geometry::UsageDef,Geometry::UsageDef::type> m_indexUsage;
    std::vector<AttributeData> m_attributeData;
    bool m_hasStaticAttributes;
    bool m_hasStreamedAttributes;
    bool m_isCompiled;
    unsigned m_staticStride;
    unsigned m_streamStride;
    std::vector<unsigned char> m_staticData;
    std::vector<unsigned char> m_streamData;
    unsigned m_staticVbo;
    unsigned m_streamVbo;
    bool m_staticDirty;
    bool m_streamDirty;
    std::vector<unsigned char> m_indexData;
    unsigned m_indexVbo;
    boost::shared_ptr<Geometry> m_linkedIndices;
    bool m_indexDirty;
    int m_semanticAttributeIdx[Geometry::SemanticDef::Custom + 1];
    unsigned m_lastRenderedFrameIdx;
    int m_lockCount;
};
}

#endif

// core/jet/include/video/geometry.h
#ifndef CORE_JET_INCLUDE_VIDEO_GEOMETRY_H
#define CORE_JET_INCLUDE_VIDEO_GEOMETRY_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "jenum.h"
#include "core/pstring.h"

namespace jet::video {
class Geometry : public boost::noncopyable {
public:
    // Verified against libdespicablemefree.so 0.0.3 armeabi-v7a:
    // ShaderAttribute::ParseValue's "mesh.<name>" string table (0x61e7ac) gives this exact order,
    // and GLES20Geometry::Draw's mode table (0x6322dc) confirms TopologyDef -> GL_LINES..GL_TRIANGLE_FAN.
    struct SemanticDef {
        enum type { Position, Normal, Tangent, Tangent1, Tangent2, Tangent3, Color,
                    TexCoord0, TexCoord1, TexCoord2, TexCoord3, NodeIndices, NodeWeights, Custom };
    };
    struct TopologyDef {
        enum type { Lines, LineLoop, LineStrip, Triangles, TriangleStrip, TriangleFan };
    };
    // From GLES20Geometry::AddAttribute's element size table (0xc95b8e: 4,1,2,1,1,2,2) and
    // _BindAttributes' switch (0x6bb7f4), which picks the GL type and the normalized flag:
    // U8/S16 are integer, the *N ones normalized. SubMesh::LoadAttributes uses Float, U8 (node
    // indices), S8N/S16N (quantized positions, normals, tangents, texcoords), U8N (colors, weights).
    struct TypeDef {
        enum type { Float, U8, S16, S8N, U8N, S16N, U16N };
    };
    struct UsageDef {
        enum type { Static, Dynamic, Stream };
    };

    struct Attribute {
        jet::String name;
        safe_enum<SemanticDef, SemanticDef::type> semantic;
        safe_enum<UsageDef, UsageDef::type> usage;
        safe_enum<TypeDef, TypeDef::type> type;
        unsigned components;
        bool enabled;
        float texCoordUsedAreaHint;
    };

    static boost::shared_ptr<Geometry> New();
    static void SetAttributeCPUAccess(unsigned);
    static unsigned GetAttributeCPUAccess();
    static void SetIndexCPUAccess(unsigned);
    static unsigned GetIndexCPUAccess();
    static void SetUseVAO(bool);
    static bool GetUseVAO();
    static bool GetUseMapBufferData();
    static void SetUseMapBufferData(bool);
    virtual ~Geometry();
    virtual unsigned GetUID() = 0;
    virtual void SetTopology(safe_enum<TopologyDef,TopologyDef::type>) = 0;
    virtual safe_enum<TopologyDef,TopologyDef::type> GetTopology() = 0;
    virtual void SetVertexCount(unsigned) = 0;
    virtual unsigned GetVertexCount() = 0;
    virtual void SetIndexCount(unsigned) = 0;
    virtual unsigned GetIndexCount() = 0;
    virtual unsigned GetPrimitiveCount() = 0;
    virtual void SetIndexUsage(safe_enum<UsageDef,UsageDef::type>) = 0;
    virtual safe_enum<UsageDef,UsageDef::type> GetIndexUsage() = 0;
    virtual void LinkIndices(boost::shared_ptr<Geometry>) = 0;
    virtual void UnlinkIndices() = 0;
    virtual int AddAttribute(const jet::String&, safe_enum<SemanticDef,SemanticDef::type>, safe_enum<UsageDef,UsageDef::type>, safe_enum<TypeDef,TypeDef::type>, unsigned) = 0;
    virtual void LinkAttribute(const jet::String&, safe_enum<SemanticDef,SemanticDef::type>, boost::shared_ptr<Geometry>, const jet::String&) = 0;
    virtual int FindAttributeIdxByName(const jet::String&) = 0;
    virtual int FindAttributeIdxBySemantic(safe_enum<SemanticDef,SemanticDef::type>) = 0;
    virtual void SetAttributeEnabled(unsigned, bool) = 0;
    virtual void SetTexCoordAttributeUsedAreaHint(unsigned, float) = 0;
    virtual float GetTexCoordAttributeUsedAreaHint(unsigned) = 0;
    virtual bool HasStaticAttributes() = 0;
    virtual bool HasStreamedAttributes() = 0;
    virtual void Compile() = 0;
    virtual unsigned GetAttributeCount() = 0;
    virtual const Attribute &GetAttribute(unsigned) = 0;
    virtual const unsigned short *MapIndicesRead() = 0;
    virtual unsigned short* MapIndicesWrite() = 0;
    virtual void UnmapIndices() = 0;
    virtual const float *MapAttributeFloatRead(unsigned) = 0;
    virtual float* MapAttributeFloatWrite(unsigned) = 0;
    virtual const signed char *MapAttributeS8Read(unsigned) = 0;
    virtual signed char* MapAttributeS8Write(unsigned) = 0;
    virtual const unsigned char *MapAttributeU8Read(unsigned) = 0;
    virtual unsigned char* MapAttributeU8Write(unsigned) = 0;
    virtual const short *MapAttributeS16Read(unsigned) = 0;
    virtual short* MapAttributeS16Write(unsigned) = 0;
    virtual const unsigned short *MapAttributeU16Read(unsigned) = 0;
    virtual unsigned short* MapAttributeU16Write(unsigned) = 0;
    virtual void UnmapAttribute(unsigned) = 0;
    virtual void UnmapAll() = 0;
    virtual unsigned GetLastRenderedFrameIdx() = 0;
    virtual bool IsLocked() = 0;
    // Bytes between one vertex's value for this attribute and the next, in whichever buffer
    // (static/stream) it lives in -- callers writing through Map*Write must advance by this, not
    // by the attribute's own componentCount*elementSize, since attributes of the same usage are
    // interleaved into one buffer. Not in the PDB stub; added because nothing else here otherwise
    // tells a caller how the attributes it just declared ended up packed after Compile().
    virtual unsigned GetAttributeStride(unsigned) = 0;
protected:
    virtual void SetLocked(bool) = 0;
private:
    static unsigned ms_attributeCPUAccessVertexLimit;
    static unsigned ms_indexCPUAccessVertexLimit;
    static bool ms_useMapBufferData;
    static bool ms_useVAO;
};
}

#endif

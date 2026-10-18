// core/jet/include/video/texture.h
#ifndef CORE_JET_INCLUDE_VIDEO_TEXTURE_H
#define CORE_JET_INCLUDE_VIDEO_TEXTURE_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "jenum.h"
#include "smath.h"
#include "res/resource.h"

namespace jet::video {
class Texture : public jet::res::Resource {
protected:
    Texture();
public:
    // WrapDef/FilterDef verified against libdespicablemefree.so 0.0.3 (GLES20Texture::Bind's
    // GL_TEXTURE_WRAP_S/T constant table at 0x6461bc, and the m_filtering==0 branch at 0x646210).
    struct WrapDef { enum type { Repeat, Clamp }; };
    struct FilterDef { enum type { Nearest, Bilinear, Trilinear }; };
    // Byte-per-pixel widths (0-7) verified via GetBytesPerPixel's branch table at 0x646ad4;
    // compressed formats (8+, width 0 there) are ordered by GLES20Texture::SupportsFormat's
    // case list but not individually confirmed yet.
    struct FormatDef {
        // GLES20Texture::InitPVR's format assignment (0x6cdfb4)
        enum type { RGBA4444, RGBA5551, RGBA8888, RGB565, RGB888, Luminance8, Alpha8,
                    LuminanceAlpha88, PVRTC2, PVRTC4, ETC1, DXT1, DXT3, DXT5 };
    };

    static boost::shared_ptr<Texture> New();
    virtual ~Texture();
    virtual bool Init(jet::stream::IStream*) = 0;
    virtual bool Load() = 0;
    virtual void Unload() = 0;
    virtual bool Create(const jet::String&, const jet::String&, const math::vec2<unsigned int>&) = 0;
    virtual bool CreateDepth(const jet::String&, const jet::String&, const math::vec2<unsigned int>&) = 0;
    safe_enum<FormatDef,unsigned char> GetFormat();
    virtual bool SupportsFormat(safe_enum<FormatDef,unsigned char>) = 0;
    virtual bool UploadData(unsigned, const unsigned char*) = 0;
    virtual unsigned GetDataSize() = 0;
    virtual unsigned GetApproximatedDataSize(unsigned) = 0;
    virtual void SwapContent(const boost::shared_ptr<Texture>&);
    bool CanBeStreamed();
    bool CanBeUnloaded();
    bool IsCubeMap();
    bool HasAlphaChannel();
    bool HasMipMaps();
    const math::vec2<unsigned int> &GetLod0Size();
    const math::vec2<unsigned int> &GetSize();
    float GetMinViewDistance();
    void SetCanBeStreamed(bool);
    unsigned GetLod();
    void SetLod(unsigned);
    virtual bool GetData(std::vector<unsigned char>&, unsigned);
    // set by TextureLoader and by samplers when binding
    void SetWrapS(safe_enum<WrapDef,unsigned char>);
    safe_enum<WrapDef,unsigned char> GetWrapS();
    void SetWrapT(safe_enum<WrapDef,unsigned char>);
    safe_enum<WrapDef,unsigned char> GetWrapT();
    void SetFiltering(safe_enum<FilterDef,unsigned char>);
    safe_enum<FilterDef,unsigned char> GetFiltering();
    void SetAnisotropy(unsigned);
    unsigned GetAnisotropy();
    void SetFormat(safe_enum<FormatDef,unsigned char>);
    void SetMipMap(bool);
    bool GetMipMap();
    void SetViewDistance(float);
    void SetCanBeUnloaded(bool);
public:
    virtual void Bind(int);
protected:
    void SetIsCubeMap(bool);
    void SetHasMipMaps(bool);
    void SetHasAlphaChannel(bool);
    void SetSize(const math::vec2<unsigned int>&);
    void SetLod0Size(const math::vec2<unsigned int>&);
    float ComputeStreamViewDistance();
    void SetMinViewDistance(float);
    int GetLastUsedFrameIdx();
    void SetLastUsedFrameIdx(int);
protected:
    bool m_canBeStreamed;
    bool m_canBeUnloaded;
    bool m_hasMipMaps;
    bool m_isCubeMap;
    bool m_hasAlphaChannel;
    bool m_useMipMap;
    safe_enum<WrapDef,unsigned char> m_wrapS;
    safe_enum<WrapDef,unsigned char> m_wrapT;
    safe_enum<FilterDef,unsigned char> m_filtering;
    safe_enum<FormatDef,unsigned char> m_format;
    unsigned m_anisotropy;
    int m_lastUsedFrameIdx;
    unsigned m_crtLod;
    float m_minViewDistance;
    float m_streamViewDistance;
    math::vec2<unsigned int> m_size;
    math::vec2<unsigned int> m_lod0Size;
};
}

#endif

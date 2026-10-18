// core/jet/include/video/gles20/gles20texture.h
#ifndef CORE_JET_INCLUDE_VIDEO_GLES20_GLES20TEXTURE_H
#define CORE_JET_INCLUDE_VIDEO_GLES20_GLES20TEXTURE_H

#include <vector>
#include "video/texture.h"
#include "video/gles20/gles20interface.h"

namespace jet::video {
class GLES20Texture : public Texture {
    GLES20Texture(const GLES20Texture&);
public:
    // Verified against libdespicablemefree.so 0.0.3, GLES20Texture::DetectFileFormat (0x643f2c):
    // 1=PVR ("PVR!" tag at a fixed offset once header size==0x34), 2=a single-magic-word format
    // (KTX, not independently confirmed), then stb_image's own test functions in this order:
    // 3=TGA, 4=BMP, 5=PNG, 6=JPEG, 7=DDS, 8=PSD. Only PVR/TGA/BMP are decoded here (see LoadNonPVR);
    // no PNG/JPEG/DDS/PSD decoder is vendored in this rebuild (the original's "astbi_*" test/decode
    // functions are a customized stb_image this repo doesn't have a copy of) -- Load() reports
    // those formats as unsupported rather than faking pixel data.
    struct FileFormatDef { enum type { None, PVR, PVR3, TGA, BMP, PNG, JPEG, DDS, PSD }; };

    GLES20Texture();
    virtual ~GLES20Texture();
    virtual bool Init(jet::stream::IStream*);
    virtual bool Load();
    virtual void Unload();
    virtual bool SupportsFormat(safe_enum<Texture::FormatDef,unsigned char>);
    virtual bool Create(const jet::String&, const jet::String&, const math::vec2<unsigned int>&);
    virtual bool CreateDepth(const jet::String&, const jet::String&, const math::vec2<unsigned int>&);
    virtual bool UploadData(unsigned, const unsigned char*);
    unsigned GetGLId();
    virtual unsigned GetDataSize();
    virtual unsigned GetApproximatedDataSize(unsigned);
    virtual void SwapContent(const boost::shared_ptr<Texture>&);
    virtual bool GetData(std::vector<unsigned char>&, unsigned);
public:
    void Bind(gles::Interface&, int unit);
protected:
    void UnloadData();
    safe_enum<FileFormatDef,FileFormatDef::type> DetectFileFormat(const unsigned char*, unsigned);
    bool InitPVR(const unsigned char*, unsigned);
    bool InitNonPVR(const unsigned char*, unsigned);
    bool LoadPVR(const unsigned char*, unsigned);
    bool LoadNonPVR(const unsigned char*, unsigned);
    void ResetSettings();
    safe_enum<FileFormatDef,FileFormatDef::type> m_fileFormat;
    safe_enum<Texture::WrapDef,unsigned char> m_oldWrapS;
    safe_enum<Texture::WrapDef,unsigned char> m_oldWrapT;
    safe_enum<Texture::FilterDef,unsigned char> m_oldFiltering;
    bool m_oldUseMipMap;
    unsigned m_oldAnisotropy;
    unsigned m_glId;
    int m_dataSize;
    unsigned m_glFormat;
};
}

#endif

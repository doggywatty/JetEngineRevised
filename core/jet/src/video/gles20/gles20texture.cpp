// core/jet/src/video/gles20/gles20texture.cpp
#include <vector>
#include "video/gles20/gles20texture.h"
#include "stream/istream.h"
#include <cstring>
#include <stdexcept>
#include <cstdio>
#include <algorithm>

// GL compressed-format tokens (GL_IMG_texture_compression_pvrtc / GL_OES_compressed_ETC1_RGB8_texture).
#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#endif
#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES 0x8D64
#endif

namespace jet::video {

// Legacy PVR (v2) container header, as produced by PVRTexTool for this engine's era.
struct PVRHeader {
    unsigned headerSize;
    unsigned height, width;
    unsigned mipmapCount;
    unsigned flags;
    unsigned dataSize;
    unsigned bpp;
    unsigned bitmaskRed, bitmaskGreen, bitmaskBlue, bitmaskAlpha;
    unsigned pvrTag;
    unsigned numSurfaces;
};

// PVR v3 container ("PVR\x03"), what textures_android/*.tga are
struct PVR3Header {
    unsigned version;
    unsigned flags;
    unsigned pixelFormatLo, pixelFormatHi;   // < 2^32: a compressed format id; else channel names + bit counts
    unsigned colourSpace;
    unsigned channelType;
    unsigned height, width, depth;
    unsigned surfaces, faces, mipmapCount;
    unsigned metaDataSize;
};

static unsigned FormatToGLInternal(safe_enum<Texture::FormatDef,unsigned char> f) {
    using F = Texture::FormatDef;
    switch ((F::type)f) {
    case F::RGBA4444: case F::RGBA5551: case F::RGBA8888: return 0x1908; // GL_RGBA
    case F::RGB565: case F::RGB888: return 0x1907;                      // GL_RGB
    case F::Alpha8: return 0x1906;                                       // GL_ALPHA
    case F::Luminance8: return 0x1909;                                   // GL_LUMINANCE
    case F::LuminanceAlpha88: return 0x190A;                             // GL_LUMINANCE_ALPHA
    case F::PVRTC2: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
    case F::PVRTC4: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
    case F::ETC1: return GL_ETC1_RGB8_OES;
    case F::DXT1: return 0x83F0;   // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    case F::DXT3: return 0x83F2;
    case F::DXT5: return 0x83F3;
    default: return 0x1908;
    }
}
static unsigned FormatToGLType(safe_enum<Texture::FormatDef,unsigned char> f) {
    using F = Texture::FormatDef;
    switch ((F::type)f) {
    case F::RGBA4444: return 0x8033; // GL_UNSIGNED_SHORT_4_4_4_4
    case F::RGBA5551: return 0x8034; // GL_UNSIGNED_SHORT_5_5_5_1
    case F::RGB565: return 0x8363;   // GL_UNSIGNED_SHORT_5_6_5
    default: return 0x1401;          // GL_UNSIGNED_BYTE
    }
}
static bool IsCompressed(safe_enum<Texture::FormatDef,unsigned char> f) {
    using F = Texture::FormatDef;
    F::type t = (F::type)f;
    return t >= F::PVRTC2;
}

// bytes of one mip level
static unsigned LevelSize(safe_enum<Texture::FormatDef,unsigned char> f, unsigned w, unsigned h) {
    using F = Texture::FormatDef;
    switch ((F::type)f) {
    case F::PVRTC2: return std::max(w, 16u) * std::max(h, 8u) / 4;
    case F::PVRTC4: return std::max(w, 8u) * std::max(h, 8u) / 2;
    case F::ETC1: case F::DXT1: return ((w + 3) / 4) * ((h + 3) / 4) * 8;
    case F::DXT3: case F::DXT5: return ((w + 3) / 4) * ((h + 3) / 4) * 16;
    case F::RGBA8888: return w * h * 4;
    case F::RGB888: return w * h * 3;
    case F::Luminance8: case F::Alpha8: return w * h;
    default: return w * h * 2;
    }
}

GLES20Texture::GLES20Texture()
    : m_fileFormat(FileFormatDef::None), m_oldWrapS(WrapDef::Repeat), m_oldWrapT(WrapDef::Repeat),
      m_oldFiltering(FilterDef::Bilinear), m_oldUseMipMap(true), m_oldAnisotropy(0), m_glId(0),
      m_dataSize(0), m_glFormat(0) {
}
GLES20Texture::~GLES20Texture() { UnloadData(); }

void GLES20Texture::ResetSettings() {
    m_oldWrapS = GetWrapS(); m_oldWrapT = GetWrapT();
    m_oldFiltering = GetFiltering(); m_oldUseMipMap = GetMipMap(); m_oldAnisotropy = GetAnisotropy();
}

unsigned GLES20Texture::GetGLId() { return m_glId; }
unsigned GLES20Texture::GetDataSize() { return m_dataSize; }
unsigned GLES20Texture::GetApproximatedDataSize(unsigned lod) {
    unsigned w = std::max(1u, GetSize().x >> lod), h = std::max(1u, GetSize().y >> lod);
    return LevelSize(GetFormat(), w, h);
}

void GLES20Texture::UnloadData() {
    if (m_glId) { gles::Interface gl; gl.iglDeleteTextures(1, &m_glId); m_glId = 0; }
}
void GLES20Texture::Unload() { UnloadData(); SetLoaded(false); }

void GLES20Texture::Bind(gles::Interface &gl, int unit) {
    gl.iglActiveTexture(0x84C0 + unit); // GL_TEXTURE0 + unit
    gl.iglBindTexture(0x0DE1 /*GL_TEXTURE_2D*/, m_glId);
    if (m_oldWrapS != GetWrapS() || m_oldWrapT != GetWrapT() || m_oldFiltering != GetFiltering() ||
        m_oldUseMipMap != GetMipMap() || m_oldAnisotropy != GetAnisotropy()) {
        unsigned wraps[] = { 0x2901 /*GL_REPEAT*/, 0x812F /*GL_CLAMP_TO_EDGE*/ };
        gl.iglTexParameteri(0x0DE1, 0x2802 /*GL_TEXTURE_WRAP_S*/, wraps[(unsigned char)(WrapDef::type)GetWrapS()]);
        gl.iglTexParameteri(0x0DE1, 0x2803 /*GL_TEXTURE_WRAP_T*/, wraps[(unsigned char)(WrapDef::type)GetWrapT()]);
        bool mip = GetMipMap() && HasMipMaps();
        unsigned minFilter = GetFiltering() == FilterDef::Nearest ? (mip ? 0x2700u /*NEAREST_MIPMAP_NEAREST*/ : 0x2600u /*NEAREST*/)
                                                                    : (mip ? 0x2703u /*LINEAR_MIPMAP_LINEAR*/ : 0x2601u /*LINEAR*/);
        unsigned magFilter = GetFiltering() == FilterDef::Nearest ? 0x2600u : 0x2601u;
        gl.iglTexParameteri(0x0DE1, 0x2801 /*GL_TEXTURE_MIN_FILTER*/, minFilter);
        gl.iglTexParameteri(0x0DE1, 0x2800 /*GL_TEXTURE_MAG_FILTER*/, magFilter);
        ResetSettings();
    }
    Texture::Bind(unit);
}

bool GLES20Texture::Create(const jet::String &name, const jet::String &fmt, const math::vec2<unsigned int> &size) {
    // @0x646338 format names
    using F = Texture::FormatDef;
    if (fmt.Equals("8888")) SetFormat(F::RGBA8888);
    else if (fmt.Equals("888")) SetFormat(F::RGB888);
    else if (fmt.Equals("565")) SetFormat(F::RGB565);
    else if (fmt.Equals("4444")) SetFormat(F::RGBA4444);
    else if (fmt.Equals("8")) SetFormat(F::Alpha8);
    UnloadData();
    gles::Interface gl;
    gl.iglGenTextures(1, &m_glId);
    gl.iglBindTexture(0x0DE1, m_glId);
    unsigned internalFmt = FormatToGLInternal(GetFormat());
    gl.iglTexImage2D(0x0DE1, 0, internalFmt, size.x, size.y, 0, internalFmt, FormatToGLType(GetFormat()), 0);
    SetSize(size); SetLod0Size(size);
    SetResourceName(name);
    SetLoaded(true);
    ResetSettings();
    m_oldFiltering = safe_enum<FilterDef,unsigned char>((FilterDef::type)0xff);  // force parameters on first bind
    return true;
}
bool GLES20Texture::CreateDepth(const jet::String &name, const jet::String&, const math::vec2<unsigned int> &size) {
    UnloadData();
    gles::Interface gl;
    gl.iglGenTextures(1, &m_glId);
    gl.iglBindTexture(0x0DE1, m_glId);
    gl.iglTexImage2D(0x0DE1, 0, 0x1902 /*GL_DEPTH_COMPONENT*/, size.x, size.y, 0, 0x1902, 0x1400 /*GL_UNSIGNED_BYTE*/, 0);
    SetSize(size); SetLod0Size(size);
    SetResourceName(name);
    SetLoaded(true);
    return true;
}

static bool NativeETC1() {
#if defined(__ANDROID__) || defined(__linux__)
    return true;
#else
    return false;
#endif
}

// ETC1 -> RGB888 for drivers without GL_OES_compressed_ETC1_RGB8_texture (desktop macOS)
static void DecodeETC1(const unsigned char *src, unsigned w, unsigned h, std::vector<unsigned char> &out) {
    static const int mods[8][2] = {{2,8},{5,17},{9,29},{13,42},{18,60},{24,80},{33,106},{47,183}};
    out.assign((size_t)w * h * 3, 0);
    auto sign3 = [](int v) { return v >= 4 ? v - 8 : v; };
    auto clamp = [](int v) { return (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v); };
    for (unsigned by = 0; by < (h + 3) / 4; by++)
        for (unsigned bx = 0; bx < (w + 3) / 4; bx++, src += 8) {
            int c[2][3];
            bool diff = src[3] & 2, flip = src[3] & 1;
            if (diff) {
                for (int k = 0; k < 3; k++) {
                    int a = src[k] >> 3, b = a + sign3(src[k] & 7);
                    c[0][k] = (a << 3) | (a >> 2); c[1][k] = (b << 3) | (b >> 2);
                }
            } else {
                for (int k = 0; k < 3; k++) { c[0][k] = (src[k] >> 4) * 17; c[1][k] = (src[k] & 15) * 17; }
            }
            int tbl[2] = { (src[3] >> 5) & 7, (src[3] >> 2) & 7 };
            unsigned msb = (src[4] << 8) | src[5], lsb = (src[6] << 8) | src[7];
            for (unsigned x = 0; x < 4; x++)
                for (unsigned y = 0; y < 4; y++) {
                    unsigned px = bx * 4 + x, py = by * 4 + y;
                    if (px >= w || py >= h) continue;
                    int sub = flip ? (y >= 2) : (x >= 2);
                    unsigned k = x * 4 + y;
                    int idx = (((msb >> k) & 1) << 1) | ((lsb >> k) & 1);
                    int m = mods[tbl[sub]][idx & 1];
                    if (idx & 2) m = -m;
                    unsigned char *o = &out[((size_t)py * w + px) * 3];
                    for (int q = 0; q < 3; q++) o[q] = clamp(c[sub][q] + m);
                }
        }
}

// uploads one mip level, decoding ETC1 in software when the driver lacks it
static void UploadLevel(gles::Interface &gl, safe_enum<Texture::FormatDef,unsigned char> fmt, unsigned lod,
                        unsigned w, unsigned h, unsigned n, const unsigned char *p) {
    unsigned internalFmt = FormatToGLInternal(fmt);
    if (fmt == Texture::FormatDef::ETC1 && !NativeETC1()) {
        std::vector<unsigned char> rgb;
        DecodeETC1(p, w, h, rgb);
        gl.iglPixelStorei(0x0CF5 /*GL_UNPACK_ALIGNMENT*/, 1);
        gl.iglTexImage2D(0x0DE1, lod, 0x1907, w, h, 0, 0x1907, 0x1401, rgb.data());
    } else if (IsCompressed(fmt))
        gl.iglCompressedTexImage2D(0x0DE1, lod, internalFmt, w, h, 0, n, p);
    else
        gl.iglTexImage2D(0x0DE1, lod, internalFmt, w, h, 0, internalFmt, FormatToGLType(fmt), p);
}

bool GLES20Texture::UploadData(unsigned lod, const unsigned char *data) {
    if (!m_glId) return false;
    gles::Interface gl;
    gl.iglBindTexture(0x0DE1, m_glId);
    unsigned w = std::max(1u, GetSize().x >> lod), h = std::max(1u, GetSize().y >> lod);
    UploadLevel(gl, GetFormat(), lod, w, h, GetApproximatedDataSize(lod), data);
    return true;
}

bool GLES20Texture::SupportsFormat(safe_enum<Texture::FormatDef,unsigned char> f) {
    using F = Texture::FormatDef;
    switch ((F::type)f) {
    case F::ETC1:
        return true;   // decoded in software where the driver lacks GL_OES_compressed_ETC1_RGB8_texture
    case F::DXT1: case F::DXT3: case F::DXT5:
        return false;
    case F::PVRTC2: case F::PVRTC4:
#if defined(__APPLE__)
        return true;
#else
        return false;
#endif
    default: return true;
    }
}

safe_enum<GLES20Texture::FileFormatDef,GLES20Texture::FileFormatDef::type> GLES20Texture::DetectFileFormat(const unsigned char *data, unsigned size) {
    if (!data) return FileFormatDef::None;
    if (size > 0x34) {
        const PVRHeader *h = reinterpret_cast<const PVRHeader*>(data);
        if (h->headerSize == 0x34 && memcmp(&h->pvrTag, "PVR!", 4) == 0) return FileFormatDef::PVR;
        if (memcmp(data, "PVR\x03", 4) == 0) return FileFormatDef::PVR3;
    }
    if (size >= 18 && data[2] == 2) return FileFormatDef::TGA;  // uncompressed truecolor TGA
    if (size >= 2 && data[0] == 'B' && data[1] == 'M') return FileFormatDef::BMP;
    if (size >= 8 && memcmp(data, "\x89PNG\r\n\x1a\n", 8) == 0) return FileFormatDef::PNG;
    if (size >= 2 && data[0] == 0xFF && data[1] == 0xD8) return FileFormatDef::JPEG;
    return FileFormatDef::None;
}

// InitPVR @0x6cdfb4 (v3 header path): format from the pixel format word (channel names and bit
// counts for uncompressed data, 0..11 for compressed ones), size, mipmaps, cubemap (faces > 1).
bool GLES20Texture::InitPVR(const unsigned char *data, unsigned size) {
    using F = Texture::FormatDef;
    if (m_fileFormat != FileFormatDef::PVR3 || size < sizeof(PVR3Header)) return false;
    const PVR3Header *h = reinterpret_cast<const PVR3Header*>(data);
    const unsigned lo = h->pixelFormatLo, hi = h->pixelFormatHi;
    F::type fmt;
    bool alpha = false;
    if (lo == 0x61626772 /*'rgba'*/ && hi == 0x04040404) { fmt = F::RGBA4444; alpha = true; }
    else if (lo == 0x61626772 && hi == 0x01050505) { fmt = F::RGBA5551; alpha = true; }
    else if (lo == 0x61626772 && hi == 0x08080808) { fmt = F::RGBA8888; alpha = true; }
    else if (lo == 0x00626772 /*'rgb'*/ && hi == 0x00050605) fmt = F::RGB565;
    else if (lo == 0x00626772 && hi == 0x00080808) fmt = F::RGB888;
    else if (lo == 0x69 /*'i'*/ && hi == 8) fmt = F::Luminance8;
    else if (lo == 0x61 /*'a'*/ && hi == 8) { fmt = F::Alpha8; alpha = true; }
    else if (lo == 0x6961 /*'ia'*/ && hi == 0x0808) { fmt = F::LuminanceAlpha88; alpha = true; }
    else if (hi == 0 && lo <= 1) { fmt = F::PVRTC2; alpha = lo == 1; }
    else if (hi == 0 && lo <= 3) { fmt = F::PVRTC4; alpha = lo == 3; }
    else if (hi == 0 && lo == 6) fmt = F::ETC1;
    else if (hi == 0 && lo == 7) { fmt = F::DXT1; alpha = true; }
    else if (hi == 0 && lo == 9) { fmt = F::DXT3; alpha = true; }
    else if (hi == 0 && lo == 11) { fmt = F::DXT5; alpha = true; }
    else { printf("WARNING: unknown texture format\n"); return false; }
    SetFormat(safe_enum<F,unsigned char>(fmt));
    m_hasAlphaChannel = alpha;
    m_isCubeMap = h->faces > 1;
    SetHasMipMaps(h->mipmapCount > 1);
    // the quality setting drops the largest levels (Texture::SetLod)
    unsigned lod = h->mipmapCount > 1 ? std::min(GetLod(), h->mipmapCount - 1) : 0;
    SetLod(lod);
    SetLod0Size(math::vec2<unsigned>(h->width, h->height));
    SetSize(math::vec2<unsigned>(std::max(1u, h->width >> lod), std::max(1u, h->height >> lod)));
    return true;
}

// LoadPVR @0x6ce7e0 (v3): levels follow the header and its metadata, largest first
bool GLES20Texture::LoadPVR(const unsigned char *data, unsigned size) {
    const PVR3Header *h = reinterpret_cast<const PVR3Header*>(data);
    unsigned offset = sizeof(PVR3Header) + h->metaDataSize;
    gles::Interface gl;
    if (!m_glId) gl.iglGenTextures(1, &m_glId);
    gl.iglBindTexture(0x0DE1, m_glId);
    const unsigned levels = std::max(1u, h->mipmapCount);
    const unsigned skip = GetLod();
    unsigned uploaded = 0, bytes = 0;
    for (unsigned level = 0; level < levels; level++) {
        unsigned lw = std::max(1u, h->width >> level), lh = std::max(1u, h->height >> level);
        unsigned n = LevelSize(GetFormat(), lw, lh) * h->faces * h->surfaces;
        if (offset + n > size) break;
        if (level >= skip) {
            const unsigned char *p = data + offset;
            UploadLevel(gl, GetFormat(), uploaded, lw, lh, n, p);
            ++uploaded;
            bytes += n;
        }
        offset += n;
    }
    SetHasMipMaps(uploaded > 1);
    m_dataSize = (int)bytes;
    ResetSettings();
    m_oldFiltering = safe_enum<FilterDef,unsigned char>((FilterDef::type)0xff);  // force parameters on first bind
    return uploaded > 0;
}

// Only the uncompressed-truecolor TGA and simple 24/32bpp BMP cases are handled; PNG/JPEG/DDS/PSD
// need a real image codec this rebuild doesn't vendor (see the header comment).
bool GLES20Texture::InitNonPVR(const unsigned char *data, unsigned size) {
    if (m_fileFormat == FileFormatDef::TGA && size >= 18) {
        unsigned w = data[12] | (data[13] << 8);
        unsigned h = data[14] | (data[15] << 8);
        unsigned bpp = data[16];
        SetFormat(safe_enum<Texture::FormatDef,unsigned char>(bpp == 32 ? Texture::FormatDef::RGBA8888 : Texture::FormatDef::RGB888));
        SetSize(math::vec2<unsigned>(w, h));
        SetLod0Size(math::vec2<unsigned>(w, h));
        return true;
    }
    throw std::logic_error("GLES20Texture::InitNonPVR: this file format has no decoder vendored in this rebuild yet");
}
bool GLES20Texture::LoadNonPVR(const unsigned char *data, unsigned size) {
    if (m_fileFormat != FileFormatDef::TGA) return false;
    unsigned idLen = data[0];
    unsigned bpp = data[16];
    unsigned bytesPerPixel = bpp / 8;
    const unsigned char *pixels = data + 18 + idLen;
    unsigned w = GetSize().x, h = GetSize().y;
    unsigned needed = w * h * bytesPerPixel;
    if (18 + idLen + needed > size) return false;

    // TGA stores BGR(A) bottom-up; convert to top-down RGB(A) for GL.
    std::vector<unsigned char> converted(needed);
    for (unsigned y = 0; y < h; y++) {
        const unsigned char *src = pixels + (h - 1 - y) * w * bytesPerPixel;
        unsigned char *dst = &converted[y * w * bytesPerPixel];
        for (unsigned x = 0; x < w; x++) {
            dst[x * bytesPerPixel + 0] = src[x * bytesPerPixel + 2];
            dst[x * bytesPerPixel + 1] = src[x * bytesPerPixel + 1];
            dst[x * bytesPerPixel + 2] = src[x * bytesPerPixel + 0];
            if (bytesPerPixel == 4) dst[x * bytesPerPixel + 3] = src[x * bytesPerPixel + 3];
        }
    }
    gles::Interface gl;
    if (!m_glId) gl.iglGenTextures(1, &m_glId);
    gl.iglBindTexture(0x0DE1, m_glId);
    unsigned internalFmt = FormatToGLInternal(GetFormat());
    gl.iglTexImage2D(0x0DE1, 0, internalFmt, w, h, 0, internalFmt, 0x1401, &converted[0]);
    if (GetMipMap()) { gl.iglGenerateMipmap(0x0DE1); SetHasMipMaps(true); }
    m_dataSize = (int)needed;
    return true;
}

bool GLES20Texture::Init(jet::stream::IStream *s) {
    SetStream(s);
    s->Start();
    unsigned size = s->Size();
    if (size == 0) { s->Finish(); return false; }
    std::vector<unsigned char> buf(std::min(size, 0x1000u));
    s->Read(&buf[0], (unsigned)buf.size());
    s->Finish();
    m_fileFormat = DetectFileFormat(&buf[0], (unsigned)buf.size());
    if (m_fileFormat == FileFormatDef::None) return false;
    bool ok = m_fileFormat == FileFormatDef::PVR || m_fileFormat == FileFormatDef::PVR3 ? InitPVR(&buf[0], (unsigned)buf.size())
                                                  : InitNonPVR(&buf[0], (unsigned)buf.size());
    return ok;
}

bool GLES20Texture::Load() {
    jet::stream::IStream *s = GetStream();
    if (!s) return false;
    s->Start();
    s->SeekBeg(0);  // the format was detected from the first bytes; a memory mapped stream does not rewind on Start
    unsigned size = s->Size();
    std::vector<unsigned char> buf(size);
    if (size) s->Read(&buf[0], size);
    s->Finish();
    bool ok = m_fileFormat == FileFormatDef::PVR || m_fileFormat == FileFormatDef::PVR3 ? LoadPVR(&buf[0], size) : LoadNonPVR(&buf[0], size);
    SetLoaded(ok);
    return ok;
}

void GLES20Texture::SwapContent(const boost::shared_ptr<Texture> &o) {
    Texture::SwapContent(o);
    if (!o) return;
    GLES20Texture *t = static_cast<GLES20Texture*>(o.get());
    std::swap(m_glId, t->m_glId);
    std::swap(m_dataSize, t->m_dataSize);
    std::swap(m_glFormat, t->m_glFormat);
    std::swap(m_fileFormat, t->m_fileFormat);
}

bool GLES20Texture::GetData(std::vector<unsigned char> &out, unsigned lod) {
    if (!m_glId) return false;
    gles::Interface gl;
    gl.iglBindTexture(0x0DE1, m_glId);
    unsigned w = std::max(1u, GetSize().x >> lod), h = std::max(1u, GetSize().y >> lod);
    out.resize((size_t)w * h * 4);
    gl.iglReadPixels(0, 0, w, h, 0x1908 /*GL_RGBA*/, 0x1401, &out[0]);
    return true;
}

}

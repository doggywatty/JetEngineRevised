// core/jet/src/video/gles20/gles20rendertarget.cpp
#include "video/gles20/gles20rendertarget.h"
#include "video/gles20/gles20texture.h"
#include <cstdlib>
#include <string>

namespace jet::video {

GLES20RenderTarget::GLES20RenderTarget() : GLES20RenderTargetWrapper(0), m_colorBufferGLId(0), m_depthBufferGLId(0) {}
GLES20RenderTarget::~GLES20RenderTarget() { Unload(); }

// CreateTask::Run @0x6c0320: the format is space separated tokens, "T<fmt>" a colour texture,
// "<fmt>" a colour renderbuffer (fmt 565, 888, 8888, 4444, 5551), "D16"/"D24"/"D32" depth,
// "S8" stencil, "A<n>" multisampling (e.g. the post-effect targets' "T565 D16").
void GLES20RenderTarget::Create(const jet::String &name, const jet::String &format, const math::vec2<unsigned int> &size) {
    GLES20RenderTargetWrapper::Create(name, format, size);
    typedef Texture::FormatDef F;
    bool colorTexture = false, colorBuffer = false, stencil = false;
    F::type colorFormat = F::RGBA8888;
    unsigned depthBits = 0;
    std::string f(format.c_str());
    if (f == "rgba8888") f = "T8888";  // older callers
    size_t pos = 0;
    while (pos < f.size()) {
        size_t end = f.find(' ', pos);
        std::string tok = f.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
        pos = end == std::string::npos ? f.size() : end + 1;
        if (tok.empty()) continue;
        bool tex = tok[0] == 'T';
        std::string body = tex ? tok.substr(1) : tok;
        if (body == "565" || body == "888" || body == "8888" || body == "4444" || body == "5551") {
            colorFormat = body == "565" ? F::RGB565 : body == "888" ? F::RGB888 : body == "4444" ? F::RGBA4444 : body == "5551" ? F::RGBA5551 : F::RGBA8888;
            (tex ? colorTexture : colorBuffer) = true;
        } else if (tok[0] == 'D') depthBits = atoi(tok.c_str() + 1);
        else if (tok == "S8") stencil = true;
    }
    gles::Interface gl;
    unsigned fbo = 0;
    gl.iglGenFramebuffers(1, &fbo);
    SetFBOId(fbo);
    gl.iglBindFramebuffer(0x8D40 /*GL_FRAMEBUFFER*/, fbo);
    if (colorTexture) {
        boost::shared_ptr<Texture> color = Texture::New();
        color->SetFormat(colorFormat);
        color->SetMipMap(false);
        color->SetFiltering(Texture::FilterDef::Bilinear);
        color->SetWrapS(Texture::WrapDef::Clamp);
        color->SetWrapT(Texture::WrapDef::Clamp);
        color->Create(name + "_color", format, size);
        gl.iglFramebufferTexture2D(0x8D40, 0x8CE0 /*GL_COLOR_ATTACHMENT0*/, 0x0DE1 /*GL_TEXTURE_2D*/, static_cast<GLES20Texture*>(color.get())->GetGLId(), 0);
        SetColorTexture(color);
    } else if (colorBuffer) {
        gl.iglGenRenderbuffers(1, &m_colorBufferGLId);
        gl.iglBindRenderbuffer(0x8D41 /*GL_RENDERBUFFER*/, m_colorBufferGLId);
        gl.iglRenderbufferStorage(0x8D41, colorFormat == F::RGB565 ? 0x8D62 /*RGB565*/ : 0x8058 /*RGBA8_OES*/, size.x, size.y);
        gl.iglFramebufferRenderbuffer(0x8D40, 0x8CE0, 0x8D41, m_colorBufferGLId);
    }
    SetHasColorTarget(colorTexture || colorBuffer);
    if (depthBits || stencil) {
        gl.iglGenRenderbuffers(1, &m_depthBufferGLId);
        gl.iglBindRenderbuffer(0x8D41, m_depthBufferGLId);
        unsigned internal = stencil ? 0x88F0 /*DEPTH24_STENCIL8_OES*/ : depthBits > 16 ? 0x81A6 /*DEPTH_COMPONENT24_OES*/ : 0x81A5 /*DEPTH_COMPONENT16*/;
        gl.iglRenderbufferStorage(0x8D41, internal, size.x, size.y);
        if (depthBits) gl.iglFramebufferRenderbuffer(0x8D40, 0x8D00 /*GL_DEPTH_ATTACHMENT*/, 0x8D41, m_depthBufferGLId);
        if (stencil) gl.iglFramebufferRenderbuffer(0x8D40, 0x8D20 /*GL_STENCIL_ATTACHMENT*/, 0x8D41, m_depthBufferGLId);
    }
    SetHasDepthTarget(depthBits != 0);
    SetHasStencilTarget(stencil);
}

void GLES20RenderTarget::Destroy() { Unload(); }
void GLES20RenderTarget::Unload() {
    gles::Interface gl;
    if (m_colorBufferGLId) { gl.iglDeleteRenderbuffers(1, &m_colorBufferGLId); m_colorBufferGLId = 0; }
    if (m_depthBufferGLId) { gl.iglDeleteRenderbuffers(1, &m_depthBufferGLId); m_depthBufferGLId = 0; }
    GLES20RenderTargetWrapper::Unload();
}
bool GLES20RenderTarget::IsValid() { return GetFBOId() != 0; }

const boost::shared_ptr<Texture> &GLES20RenderTarget::GetColorTexture() { return GLES20RenderTargetWrapper::GetColorTexture(); }
void GLES20RenderTarget::SetColorTexture(boost::shared_ptr<Texture> t) { GLES20RenderTargetWrapper::SetColorTexture(t); }

}

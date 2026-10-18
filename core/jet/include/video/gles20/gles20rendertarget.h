// core/jet/include/video/gles20/gles20rendertarget.h
#ifndef CORE_JET_INCLUDE_VIDEO_GLES20_GLES20RENDERTARGET_H
#define CORE_JET_INCLUDE_VIDEO_GLES20_GLES20RENDERTARGET_H

#include "video/gles20/gles20rendertargetwrapper.h"

namespace jet::video {
// An offscreen render target: owns its own FBO plus a color (and optionally depth) texture,
// unlike GLES20RenderTargetWrapper's base case of wrapping an externally-owned FBO (id 0, the
// default framebuffer/screen).
class GLES20RenderTarget : public GLES20RenderTargetWrapper {
public:
    GLES20RenderTarget();
    virtual ~GLES20RenderTarget();
    virtual void Create(const jet::String&, const jet::String&, const math::vec2<unsigned int>&);
    virtual void Destroy();
    virtual void Unload();
    virtual bool IsValid();
    virtual const boost::shared_ptr<Texture> &GetColorTexture();
    virtual void SetColorTexture(boost::shared_ptr<Texture>);
private:
    unsigned m_colorBufferGLId;
    unsigned m_depthBufferGLId;
};
}

#endif

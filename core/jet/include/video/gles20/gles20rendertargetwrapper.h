// core/jet/include/video/gles20/gles20rendertargetwrapper.h
#ifndef CORE_JET_INCLUDE_VIDEO_GLES20_GLES20RENDERTARGETWRAPPER_H
#define CORE_JET_INCLUDE_VIDEO_GLES20_GLES20RENDERTARGETWRAPPER_H

#include "video/rendertarget.h"
#include "video/gles20/gles20interface.h"

namespace jet::video {

// A synchronous (not task-queued) rewrite of GLES20RenderTargetWrapper: FBO create/bind/clear
// happen directly on the calling thread instead of going through the original's CreateTask/
// BindTask/ClearTask/UnloadTask (a multi-threaded GL-submission scheme built on jet::thread::Task
// -- not implemented here since nothing yet drives a separate render thread). MSAA resolve is
// likewise not implemented (m_msaaFboId is tracked but never allocated/blitted).
class GLES20RenderTargetWrapper : public RenderTarget {
public:
    explicit GLES20RenderTargetWrapper(unsigned fboId);
    virtual ~GLES20RenderTargetWrapper();
    virtual void Create(const jet::String&, const jet::String&, const math::vec2<unsigned int>&);
    virtual void Unload();
    virtual void Destroy();
    virtual bool IsValid();
    virtual const jet::String &GetName();
    virtual const boost::shared_ptr<Texture> &GetColorTexture();
    virtual void SetColorTexture(boost::shared_ptr<Texture>);
    virtual const boost::shared_ptr<Texture> &GetDepthTexture();
    virtual const jet::String &GetFormat();
    virtual bool HasColorTarget();
    virtual bool HasDepthTarget();
    virtual void SetClearColor(const math::vec4<float>&);
    virtual const math::vec4<float> &GetClearColor();
    virtual void SetClearDepthBuffer(bool);
    virtual bool GetClearDepthBuffer();
    virtual void SetClearColorBuffer(bool);
    virtual bool GetClearColorBuffer();
    virtual void SetClearPerFrame(bool);
    virtual bool GetClearPerFrame();
    virtual const math::vec2<unsigned int> &GetSize();
    virtual const math::vec2<unsigned int> &GetAllocatedSize();
    virtual void SetScissorTest(bool);
    virtual void SetScissorRect(const math::rect<unsigned int>&);
    virtual bool GetScissorTest();
    virtual const math::rect<unsigned int> &GetScissorRect();
    // Binds this target's FBO, sets the viewport/scissor, and clears it -- what a Driver needs
    // to do whenever SetCrtRenderTarget() switches the active target (see
    // GLES20Driver::SetCrtRenderTarget). Not in the PDB's RenderTarget interface; added because
    // nothing else in this rebuild currently drives that per-frame FBO activation.
    void Activate();
    void SetFBOId(unsigned);
    unsigned GetFBOId();
    void SetMSAAFBOId(unsigned);
    unsigned GetMSAAFBOId();
    void SetHasColorTarget(bool);
    void SetHasDepthTarget(bool);
    void SetHasStencilTarget(bool);
    void SetAllocatedSize(const math::vec2<unsigned int>&);
    void SetSize(const math::vec2<unsigned int>&);
    unsigned GetLastRenderedFrameIdx();
    void Clear();
protected:
    void Bind();
    void SetLastRenderedFrameIdx(unsigned);
private:
    math::rect<unsigned int> GetHWScissorRect();
    void ApplyScissorIfBound();
    unsigned m_fboId;
    unsigned m_msaaFboId;
    bool m_wasRenderedInto;
    jet::String m_name;
    jet::String m_format;
    math::vec2<unsigned int> m_size;
    math::vec2<unsigned int> m_allocatedSize;
    math::vec4<float> m_clearColor;
    bool m_clearColorBuffer;
    bool m_clearDepthBuffer;
    bool m_clearPerFrame;
    bool m_hasColorTarget;
    bool m_hasDepthTarget;
    bool m_hasStencilTarget;
    boost::shared_ptr<Texture> m_colorTexture;
    boost::shared_ptr<Texture> m_depthTexture;
    math::rect<unsigned int> m_scissorRect;
    bool m_scissorTestEnabled;
    unsigned m_lastRenderedFrameIdx;
};
}

#endif

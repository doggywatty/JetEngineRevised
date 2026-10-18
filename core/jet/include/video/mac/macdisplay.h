// core/jet/include/video/mac/macdisplay.h
#ifndef CORE_JET_INCLUDE_VIDEO_MAC_MACDISPLAY_H
#define CORE_JET_INCLUDE_VIDEO_MAC_MACDISPLAY_H

#include <vector>
#include "video/display.h"

namespace jet::video {
// Real macOS Display: creates a CGL context (desktop GL, matching gles20interface.cpp's
// OpenGL/gl.h path for non-iOS Apple targets) and an offscreen GLES20RenderTarget as its
// framebuffer, since there's no NSView/window layer in this rebuild yet -- this Display is
// usable headless (render-to-texture, read back with glReadPixels) or as the backing context
// for a future windowed view. Swap() is a glFlush (no real swap-chain without a CAOpenGLLayer/
// NSView, which belongs to the still-unimplemented windowing/App layer).
class MacDisplay : public Display {
public:
    MacDisplay();
    virtual ~MacDisplay();
    virtual unsigned GetClassId();
    virtual void* GetContext();
    virtual void* CreateSharedContext();
    virtual bool SetThreadContext(void*);
    virtual void Swap();
    virtual safe_enum<StateDef,StateDef::type> GetState();
    virtual bool Init(const DisplayMode&);
    virtual void Cleanup();
    virtual DisplayMode GetMode();
    virtual void SetOrientation(safe_enum<OrientationDef,OrientationDef::type>);
    virtual safe_enum<OrientationDef,OrientationDef::type> GetOrientation();
    virtual math::vec2<unsigned int> GetSize();
    virtual math::vec2<unsigned int> GetNativeSize();
    virtual const boost::shared_ptr<RenderTarget> &GetRenderTarget();
    virtual boost::shared_ptr<RenderTarget> CreateCompatibleRenderTarget();
    virtual const boost::shared_ptr<RenderTarget> &CreatePostEffectRenderTarget(unsigned, unsigned);
    virtual boost::shared_ptr<RenderTarget> GetPostEffectRenderTarget(unsigned);
    virtual bool IsLocked();
    // Makes the render target this size, for a window that has been resized. Its FBO is recreated in place, so the
    // pointers to the target stay valid. The contents are not kept.
    void Resize(math::vec2<unsigned int> size);
protected:
    virtual void SetLocked(bool);
    virtual bool MakeCurrent();
    virtual void Disconnect();
private:
    void *m_cglContext;
    void *m_cglPixelFormat;
    DisplayMode m_mode;
    bool m_locked;
    boost::shared_ptr<RenderTarget> m_renderTarget;
    std::vector<boost::shared_ptr<RenderTarget> > m_postEffectTargets;
};
}

#endif

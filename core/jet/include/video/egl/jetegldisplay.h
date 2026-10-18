// core/jet/include/video/egl/jetegldisplay.h
#ifndef CORE_JET_INCLUDE_VIDEO_EGL_JETEGLDISPLAY_H
#define CORE_JET_INCLUDE_VIDEO_EGL_JETEGLDISPLAY_H

#include <vector>
#include "video/display.h"

namespace jet::video {
// EGL Display, after jet::video::JetEGLDisplay (0x727964..0x7284xx in the 0.0.3 .so): eglGetDisplay,
// eglInitialize, eglChooseConfig for the mode's RGB/depth, pick the config whose attributes match
// exactly, create a GLES2 context and a surface, and wrap the surface's framebuffer (FBO 0) in a
// GLES20RenderTargetWrapper. The original's surface is the ANativeWindow; with no native window
// (desktop Linux, headless) a pbuffer of the requested size is used instead.
class JetEGLDisplay : public Display {
public:
    JetEGLDisplay(void *nativeWindow = 0, JetEGLDisplay *shared = 0);
    virtual ~JetEGLDisplay();
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
protected:
    virtual void SetLocked(bool);
    virtual bool MakeCurrent();
    virtual void Disconnect();
private:
    // Config, surface and context for one client version (3 or 2); false (nothing left behind) if this version
    // is not available on the device.
    bool CreateSurfaceAndContext(unsigned version, int r, int g, int b, int d, int s);
    void DropSurfaceAndContext();
    void *m_nativeWindow;
    JetEGLDisplay *m_shared;
    void *m_display;
    void *m_config;
    void *m_surface;
    void *m_context;
    unsigned m_clientVersion;
    DisplayMode m_mode;
    math::vec2<unsigned int> m_size;
    bool m_locked;
    boost::shared_ptr<RenderTarget> m_renderTarget;
    std::vector<boost::shared_ptr<RenderTarget> > m_postEffectTargets;
};
}

#endif

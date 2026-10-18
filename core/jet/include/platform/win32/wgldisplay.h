// core/jet/include/platform/win32/wgldisplay.h
#ifndef CORE_JET_INCLUDE_PLATFORM_WIN32_WGLDISPLAY_H
#define CORE_JET_INCLUDE_PLATFORM_WIN32_WGLDISPLAY_H

namespace jet::video {
class WGLDisplay : public jet::video::Display {  // sizeof 0x68
public:
    WGLDisplay(void*, jet::video::WGLDisplay*, bool);
    virtual ~WGLDisplay();
    void SetAutoWindowResize(bool);
    virtual unsigned GetClassId();
    virtual void* GetContext();
    virtual void* CreateSharedContext();
    virtual bool SetThreadContext(void*);
    virtual bool MakeCurrent();
    virtual void Swap();
    virtual safe_enum<jet::video::Display::StateDef,enum jet::video::Display::StateDef::type> GetState();
    virtual bool Init(const jet::video::DisplayMode&);
    virtual void Cleanup();
    virtual jet::video::DisplayMode GetMode();
    virtual void SetOrientation(safe_enum<jet::video::Display::OrientationDef,enum jet::video::Display::OrientationDef::type>);
    virtual safe_enum<jet::video::Display::OrientationDef,enum jet::video::Display::OrientationDef::type> GetOrientation();
    virtual math::vec2<unsigned int> GetSize();
    virtual math::vec2<unsigned int> GetNativeSize();
    virtual const boost::shared_ptr<jet::video::RenderTarget> &GetRenderTarget();
    virtual boost::shared_ptr<jet::video::RenderTarget> CreateCompatibleRenderTarget();
    virtual boost::shared_ptr<jet::video::RenderTarget> GetPostEffectRenderTarget(unsigned);
    virtual const boost::shared_ptr<jet::video::RenderTarget> &CreatePostEffectRenderTarget(unsigned, unsigned);
    bool IsProfileGLES20();
protected:
    virtual void SetLocked(bool);
    virtual bool IsLocked();
    virtual void Disconnect();
private:
    jet::String GetRTFormat();
    void* CreateCompatibleContext(void*, void*);
    bool m_moveWindow;  // 0x4
    void* m_context;  // 0x8
    safe_enum<jet::video::Display::StateDef,enum jet::video::Display::StateDef::type> m_state;  // 0xC
    jet::video::DisplayMode m_mode;  // 0x10
    safe_enum<jet::video::Display::OrientationDef,enum jet::video::Display::OrientationDef::type> m_orientation;  // 0x28
    math::vec2<unsigned int> m_size;  // 0x2C
    boost::shared_ptr<jet::video::RenderTarget> m_renderTarget;  // 0x34
    boost::shared_ptr<jet::video::RenderTarget> m_pfxRenderTargets[3];  // 0x3C
    bool m_isLocked;  // 0x54
    jet::video::WGLDisplay *m_parent;  // 0x58
    void* m_window;  // 0x5C
    void* m_dc;  // 0x60
    bool m_profileGLES20;  // 0x64
};
}

#endif

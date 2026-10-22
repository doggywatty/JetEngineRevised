// core/jet/include/video/win32display.h
#ifndef CORE_JET_INCLUDE_VIDEO_WIN32DISPLAY_H
#define CORE_JET_INCLUDE_VIDEO_WIN32DISPLAY_H

#include <boost/shared_ptr.hpp>
#include <cstdio>
#include "smath.h"
#include "video/display.h"
#include "video/displaymode.h"

#define NOMINMAX
#include <windows.h>

namespace jet::video {

// Provides the samejet::video::Display interface as JetEGLDisplay so the editor can run on Windows without EGL
class Win32Display : public Display {
public:
    Win32Display(HWND hwnd = nullptr);
    virtual ~Win32Display();

    unsigned GetClassId() override { return 0x57474C; } /* 'WGL' */
    void* GetContext() override { return m_hrc; }
    void* CreateSharedContext() override { return nullptr; }
    bool SetThreadContext(void* ctx) override { return wglMakeCurrent(m_hdc, (HGLRC)ctx) == TRUE; }
    void Swap() override { if (m_hdc) { wglMakeCurrent(m_hdc, m_hrc); SwapBuffers(m_hdc); } }
    safe_enum<StateDef, StateDef::type> GetState() override { return StateDef::Active; }
    bool Init(const DisplayMode& mode) override;
    void Cleanup() override;
    DisplayMode GetMode() override { return m_mode; }
    void SetOrientation(safe_enum<OrientationDef, OrientationDef::type>) override {}
    safe_enum<OrientationDef, OrientationDef::type> GetOrientation() override { return OrientationDef::LandscapeLeft; }
    math::vec2<unsigned int> GetSize() override { return m_size; }
    math::vec2<unsigned int> GetNativeSize() override { return m_size; }
    const boost::shared_ptr<RenderTarget>& GetRenderTarget() override { return m_renderTarget; }
    boost::shared_ptr<RenderTarget> CreateCompatibleRenderTarget() override { return boost::shared_ptr<RenderTarget>(); }
    const boost::shared_ptr<RenderTarget>& CreatePostEffectRenderTarget(unsigned, unsigned) override { return m_renderTarget; }
    boost::shared_ptr<RenderTarget> GetPostEffectRenderTarget(unsigned) override { return boost::shared_ptr<RenderTarget>(); }
    bool IsLocked() override { return m_locked; }
    HWND WindowHandle() const { return m_hwnd; }
    HDC DeviceContext() const { return m_hdc; }

    void SetLocked(bool l) override { m_locked = l; }
    bool MakeCurrent() override { return wglMakeCurrent(m_hdc, m_hrc) == TRUE; }
    void Disconnect() override { wglMakeCurrent(nullptr, nullptr); }

private:
    HWND m_hwnd;
    HDC m_hdc;
    HGLRC m_hrc;
    DisplayMode m_mode;
    math::vec2<unsigned int> m_size;
    bool m_locked;
    boost::shared_ptr<RenderTarget> m_renderTarget;
};

}

#endif

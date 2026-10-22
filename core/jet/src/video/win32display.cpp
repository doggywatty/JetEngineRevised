// core/jet/src/video/win32display.cpp
#if defined(_WIN32)

#include "video/win32display.h"
#include "video/gles20/gles20interface.h"
#include <cstdio>

namespace jet::video {

Win32Display::Win32Display(HWND hwnd)
    : m_hwnd(hwnd), m_hdc(nullptr), m_hrc(nullptr), m_size(0, 0), m_locked(false) {}

Win32Display::~Win32Display() { Cleanup(); }

bool Win32Display::Init(const DisplayMode& mode) {
    if (!m_hwnd) {
        std::fprintf(stderr, "Win32Display: no window handle\n");
        return false;
    }

    m_hdc = GetDC(m_hwnd);
    if (!m_hdc) {
        std::fprintf(stderr, "Win32Display: cannot get device context\n");
        return false;
    }

    // Set up a pixel format for OpenGL
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int pf = ChoosePixelFormat(m_hdc, &pfd);
    if (pf == 0) {
        std::fprintf(stderr, "Win32Display: ChoosePixelFormat failed\n");
        ReleaseDC(m_hwnd, m_hdc);
        DestroyWindow(m_hwnd);
        m_hdc = nullptr;
        m_hwnd = nullptr;
        return false;
    }
    if (!SetPixelFormat(m_hdc, pf, &pfd)) {
        std::fprintf(stderr, "Win32Display: SetPixelFormat failed\n");
        ReleaseDC(m_hwnd, m_hdc);
        DestroyWindow(m_hwnd);
        m_hdc = nullptr;
        m_hwnd = nullptr;
        return false;
    }

    m_hrc = wglCreateContext(m_hdc);
    if (!m_hrc) {
        std::fprintf(stderr, "Win32Display: wglCreateContext failed\n");
        ReleaseDC(m_hwnd, m_hdc);
        DestroyWindow(m_hwnd);
        m_hrc = nullptr;
        m_hdc = nullptr;
        m_hwnd = nullptr;
        return false;
    }

    if (!wglMakeCurrent(m_hdc, m_hrc)) {
        std::fprintf(stderr, "Win32Display: wglMakeCurrent failed\n");
        wglDeleteContext(m_hrc);
        ReleaseDC(m_hwnd, m_hdc);
        DestroyWindow(m_hwnd);
        m_hrc = nullptr;
        m_hdc = nullptr;
        m_hwnd = nullptr;
        return false;
    }

    m_mode = mode;
    m_size = mode.size;
    gles::SetClientVersion(2);

    std::fprintf(stderr, "Win32Display: WGL context created, mode %ux%u\n", m_size.x, m_size.y);
    return true;
}

void Win32Display::Cleanup() {
    if (m_hrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(m_hrc);
        m_hrc = nullptr;
    }
    if (m_hdc && m_hwnd) {
        ReleaseDC(m_hwnd, m_hdc);
        m_hdc = nullptr;
    }
    m_renderTarget.reset();
}

}

#endif

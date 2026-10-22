// core/jet/src/video/egl/jetegldisplay.cpp
#if defined(__ANDROID__) || (defined(__linux__) && __has_include(<EGL/egl.h>))

#include "video/egl/jetegldisplay.h"
#include "video/gles20/gles20interface.h"
#include "video/gles20/gles20rendertargetwrapper.h"
#include "video/gles20/gles20rendertarget.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#if defined(__ANDROID__)
#include <android/native_window.h>
#endif

namespace jet::video {

// The client version asked for: GLES 3.0, unless JET_GLES_VERSION=2 selects the GLES 2.0 path. A device without
// a GLES 3.0 config or context falls back to 2.0 either way.
static unsigned RequestedClientVersion() {
    const char *e = std::getenv("JET_GLES_VERSION");
    return e && std::strcmp(e, "2") == 0 ? 2 : 3;
}

JetEGLDisplay::JetEGLDisplay(void *nativeWindow, JetEGLDisplay *shared)
    : m_nativeWindow(nativeWindow), m_shared(shared), m_display(EGL_NO_DISPLAY), m_config(0),
      m_surface(EGL_NO_SURFACE), m_context(EGL_NO_CONTEXT), m_clientVersion(2), m_size(0, 0), m_locked(false) {}

JetEGLDisplay::~JetEGLDisplay() { Cleanup(); }

unsigned JetEGLDisplay::GetClassId() { return 0x45474C; /* 'EGL' */ }

bool JetEGLDisplay::Init(const DisplayMode &mode) {
    Cleanup();
    m_mode = mode;
    const int r = mode.color == DisplayMode::ColorDef::RGB565 ? 5 : 8;
    const int g = mode.color == DisplayMode::ColorDef::RGB565 ? 6 : 8;
    const int b = r;
    const int d = mode.depth == DisplayMode::DepthDef::None ? 0 : mode.depth == DisplayMode::DepthDef::Depth16 ? 16 : 24;
    const int s = mode.stencil == DisplayMode::StencilDef::Stencil8 ? 8 : 0;
    std::printf("Choose configs RGB%d%d%d D%d\n", r, g, b, d);

    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    bool ok = dpy != EGL_NO_DISPLAY && eglInitialize(dpy, 0, 0);
#if !defined(__ANDROID__) && defined(EGL_PLATFORM_SURFACELESS_MESA)
    // no X/Wayland server (headless container): Mesa's surfaceless platform still gives pbuffers
    if (!ok && !m_nativeWindow) {
        typedef EGLDisplay (*GetPlatformDisplayFn)(EGLenum, void*, const EGLint*);
        GetPlatformDisplayFn gpd = (GetPlatformDisplayFn)eglGetProcAddress("eglGetPlatformDisplayEXT");
        dpy = gpd ? gpd(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, 0) : EGL_NO_DISPLAY;
        ok = dpy != EGL_NO_DISPLAY && eglInitialize(dpy, 0, 0);
    }
#endif
    if (!ok) return false;
    m_display = dpy;

    // GLES 3.0 first when requested; the 2.0 attempt covers both JET_GLES_VERSION=2 and a device that reports no 3.0
    if (!(RequestedClientVersion() >= 3 && CreateSurfaceAndContext(3, r, g, b, d, s)) && !CreateSurfaceAndContext(2, r, g, b, d, s))
        return false;

    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, m_surface, EGL_WIDTH, &w);
    eglQuerySurface(dpy, m_surface, EGL_HEIGHT, &h);
    m_size = math::vec2<unsigned int>(w, h);

    GLES20RenderTargetWrapper *rt = new GLES20RenderTargetWrapper(0);
    rt->Create("backbuffer", mode.color == DisplayMode::ColorDef::RGBA8888 ? "rgba8888" : "rgb565", m_size);
    rt->SetHasColorTarget(true);
    rt->SetHasDepthTarget(d != 0);
    rt->SetHasStencilTarget(s != 0);
    rt->SetAllocatedSize(m_size);
    rt->SetSize(m_size);
    m_renderTarget.reset(rt);
    return true;
}

bool JetEGLDisplay::CreateSurfaceAndContext(unsigned version, int r, int g, int b, int d, int s) {
    EGLDisplay dpy = m_display;
    const EGLint surfType = m_nativeWindow ? EGL_WINDOW_BIT : EGL_PBUFFER_BIT;
    const EGLint renderable = version >= 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
    EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, renderable, EGL_SURFACE_TYPE, surfType,
        EGL_RED_SIZE, r, EGL_GREEN_SIZE, g, EGL_BLUE_SIZE, b,
        EGL_DEPTH_SIZE, d, EGL_STENCIL_SIZE, s, EGL_NONE
    };
    EGLint n = 0;
    eglChooseConfig(dpy, attribs, 0, 0, &n);
    if (n <= 0) { std::printf("0 available configs for GLES %u\n", version); return false; }
    std::vector<EGLConfig> cfgs(n);
    eglChooseConfig(dpy, attribs, &cfgs[0], n, &n);
    // Same selection as the original: the first config whose RGB/depth are exactly the requested
    // ones, falling back to the first one returned.
    int sel = 0;
    for (int i = 0; i < n; ++i) {
        EGLint cr, cg, cb, cd;
        eglGetConfigAttrib(dpy, cfgs[i], EGL_RED_SIZE, &cr);
        eglGetConfigAttrib(dpy, cfgs[i], EGL_GREEN_SIZE, &cg);
        eglGetConfigAttrib(dpy, cfgs[i], EGL_BLUE_SIZE, &cb);
        eglGetConfigAttrib(dpy, cfgs[i], EGL_DEPTH_SIZE, &cd);
        if (cr == r && cg == g && cb == b && cd == d) { sel = i; break; }
    }
    m_config = cfgs[sel];

    if (m_nativeWindow) {
#if defined(__ANDROID__)
        EGLint fmt;
        eglGetConfigAttrib(dpy, m_config, EGL_NATIVE_VISUAL_ID, &fmt);
        ANativeWindow_setBuffersGeometry((ANativeWindow*)m_nativeWindow, 0, 0, fmt);
#endif
        m_surface = eglCreateWindowSurface(dpy, m_config, (EGLNativeWindowType)m_nativeWindow, 0);
    } else {
        EGLint pb[] = { EGL_WIDTH, (EGLint)m_mode.size.x, EGL_HEIGHT, (EGLint)m_mode.size.y, EGL_NONE };
        m_surface = eglCreatePbufferSurface(dpy, m_config, pb);
    }
    if (m_surface == EGL_NO_SURFACE) { DropSurfaceAndContext(); return false; }

    eglBindAPI(EGL_OPENGL_ES_API);
    EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, (EGLint)version, EGL_NONE };
    m_context = eglCreateContext(dpy, m_config, m_shared ? m_shared->m_context : EGL_NO_CONTEXT, ctxAttribs);
    if (m_context == EGL_NO_CONTEXT) { DropSurfaceAndContext(); return false; }
    if (!eglMakeCurrent(dpy, m_surface, m_surface, m_context)) { std::printf("Unable to eglMakeCurrent\n"); DropSurfaceAndContext(); return false; }

    // The context must be the client version asked for: EGL reports it (a 3.0 request the driver cannot
    // serve is a fallback to 2.0, not a success). GL_VERSION is logged as the driver's own string.
    EGLint got = 0;
    eglQueryContext(dpy, m_context, EGL_CONTEXT_CLIENT_VERSION, &got);
    const char *ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (got != (EGLint)version) {
        std::printf("EGL context is GLES %d, not GLES %u (GL_VERSION \"%s\")\n", (int)got, version, ver ? ver : "(null)");
        DropSurfaceAndContext();
        return false;
    }
    m_clientVersion = version;
    gles::SetClientVersion(version);
    std::printf("EGL context: OpenGL ES %d (EGL_CONTEXT_CLIENT_VERSION), GL_VERSION \"%s\"\n", (int)got, ver ? ver : "(null)");
    return true;
}

void JetEGLDisplay::DropSurfaceAndContext() {
    if (m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_context != EGL_NO_CONTEXT) eglDestroyContext(m_display, m_context);
        if (m_surface != EGL_NO_SURFACE) eglDestroySurface(m_display, m_surface);
    }
    m_context = EGL_NO_CONTEXT;
    m_surface = EGL_NO_SURFACE;
    m_config = 0;
}

void JetEGLDisplay::Cleanup() {
    m_renderTarget.reset();
    m_postEffectTargets.clear();
    if (m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_context != EGL_NO_CONTEXT) eglDestroyContext(m_display, m_context);
        if (m_surface != EGL_NO_SURFACE) eglDestroySurface(m_display, m_surface);
        if (!m_shared) eglTerminate(m_display);
    }
    m_display = EGL_NO_DISPLAY;
    m_context = EGL_NO_CONTEXT;
    m_surface = EGL_NO_SURFACE;
}

void* JetEGLDisplay::GetContext() { return m_context; }

void* JetEGLDisplay::CreateSharedContext() {
    EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, (EGLint)m_clientVersion, EGL_NONE };
    return eglCreateContext(m_display, m_config, m_context, ctxAttribs);
}

bool JetEGLDisplay::SetThreadContext(void *ctx) {
    if (!ctx) return eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    EGLSurface s = ctx == m_context ? m_surface : EGL_NO_SURFACE;
    return eglMakeCurrent(m_display, s, s, (EGLContext)ctx);
}
bool JetEGLDisplay::MakeCurrent() { return SetThreadContext(m_context); }
void JetEGLDisplay::Disconnect() { eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); }

void JetEGLDisplay::Swap() { eglSwapBuffers(m_display, m_surface); }

safe_enum<Display::StateDef,Display::StateDef::type> JetEGLDisplay::GetState() { return StateDef::Active; }
DisplayMode JetEGLDisplay::GetMode() { return m_mode; }
void JetEGLDisplay::SetOrientation(safe_enum<OrientationDef,OrientationDef::type>) {}
safe_enum<Display::OrientationDef,Display::OrientationDef::type> JetEGLDisplay::GetOrientation() { return OrientationDef::LandscapeLeft; }
math::vec2<unsigned int> JetEGLDisplay::GetSize() { return m_size; }
math::vec2<unsigned int> JetEGLDisplay::GetNativeSize() { return m_size; }
const boost::shared_ptr<RenderTarget> &JetEGLDisplay::GetRenderTarget() { return m_renderTarget; }

boost::shared_ptr<RenderTarget> JetEGLDisplay::CreateCompatibleRenderTarget() {
    boost::shared_ptr<GLES20RenderTarget> rt(new GLES20RenderTarget());
    rt->Create("compatible", "rgba8888", m_size);
    return rt;
}

const boost::shared_ptr<RenderTarget> &JetEGLDisplay::CreatePostEffectRenderTarget(unsigned w, unsigned h) {
    boost::shared_ptr<GLES20RenderTarget> rt(new GLES20RenderTarget());
    // CreatePostEffectRenderTarget @0x7287ac: "effect<i>", format from GetRTFormatString
    // ("T565 D16", " S8" with a stencil display mode)
    char name[32];
    snprintf(name, sizeof(name), "effect%u", (unsigned)m_postEffectTargets.size());
    rt->Create(name, "T565 D16", math::vec2<unsigned int>(w, h));
    m_postEffectTargets.push_back(rt);
    return m_postEffectTargets.back();
}

boost::shared_ptr<RenderTarget> JetEGLDisplay::GetPostEffectRenderTarget(unsigned idx) {
    return idx < m_postEffectTargets.size() ? m_postEffectTargets[idx] : boost::shared_ptr<RenderTarget>();
}

bool JetEGLDisplay::IsLocked() { return m_locked; }
void JetEGLDisplay::SetLocked(bool l) { m_locked = l; }

}

#elif defined(_WIN32)

// theres nothing
#include "video/egl/jetegldisplay.h"
#include <cstdio>

namespace jet::video {

JetEGLDisplay::JetEGLDisplay(void *, JetEGLDisplay *) : m_size(0, 0), m_clientVersion(2), m_locked(false) {}

JetEGLDisplay::~JetEGLDisplay() {}

unsigned JetEGLDisplay::GetClassId() { return 0x45474C; }

bool JetEGLDisplay::Init(const DisplayMode &mode) {
    m_mode = mode;
    m_size = mode.size;
    std::printf("JetEGLDisplay (Windows stub): Init mode %ux%u\n", m_size.x, m_size.y);
    return true;
}

safe_enum<Display::StateDef, Display::StateDef::type> JetEGLDisplay::GetState() { return StateDef::Active; }
DisplayMode JetEGLDisplay::GetMode() { return m_mode; }
void JetEGLDisplay::SetOrientation(safe_enum<OrientationDef, OrientationDef::type>) {}
safe_enum<Display::OrientationDef, Display::OrientationDef::type> JetEGLDisplay::GetOrientation() { return OrientationDef::LandscapeLeft; }
math::vec2<unsigned int> JetEGLDisplay::GetSize() { return m_size; }
math::vec2<unsigned int> JetEGLDisplay::GetNativeSize() { return m_size; }
const boost::shared_ptr<RenderTarget> &JetEGLDisplay::GetRenderTarget() { return m_renderTarget; }

void JetEGLDisplay::Swap() {}
void JetEGLDisplay::Disconnect() {}
void JetEGLDisplay::SetLocked(bool l) { m_locked = l; }
bool JetEGLDisplay::IsLocked() { return m_locked; }
void* JetEGLDisplay::GetContext() { return nullptr; }
void* JetEGLDisplay::CreateSharedContext() { return nullptr; }
bool JetEGLDisplay::SetThreadContext(void *) { return true; }
void JetEGLDisplay::Cleanup() {}
bool JetEGLDisplay::MakeCurrent() { return true; }
boost::shared_ptr<RenderTarget> JetEGLDisplay::CreateCompatibleRenderTarget() { return boost::shared_ptr<RenderTarget>(); }
const boost::shared_ptr<RenderTarget> &JetEGLDisplay::CreatePostEffectRenderTarget(unsigned, unsigned) { return m_renderTarget; }
boost::shared_ptr<RenderTarget> JetEGLDisplay::GetPostEffectRenderTarget(unsigned) { return boost::shared_ptr<RenderTarget>(); }

}

#endif

// core/jet/src/video/mac/macdisplay.cpp
// Only builds on desktop macOS (CGL is a Mac-only API, distinct from iOS's EAGL); compiles to
// an empty translation unit everywhere else, including the iOS syntax-check pass in buildall.sh.
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if defined(__APPLE__) && !TARGET_OS_IPHONE

#include "video/mac/macdisplay.h"
#include "video/gles20/gles20rendertarget.h"
#include "video/gles20/gles20interface.h"
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

namespace jet::video {

MacDisplay::MacDisplay() : m_cglContext(0), m_cglPixelFormat(0), m_locked(false) {}
MacDisplay::~MacDisplay() { Cleanup(); }

unsigned MacDisplay::GetClassId() { return 0x4D4143; /* 'MAC' */ }

bool MacDisplay::Init(const DisplayMode &mode) {
    m_mode = mode;
    CGLPixelFormatAttribute attribs[] = {
        kCGLPFAAccelerated,
        kCGLPFAColorSize, (CGLPixelFormatAttribute)32,
        kCGLPFADepthSize, (CGLPixelFormatAttribute)(mode.depth == DisplayMode::DepthDef::Depth16 ? 16 : 24),
        (CGLPixelFormatAttribute)0
    };
    CGLPixelFormatObj pix = 0;
    GLint nPix = 0;
    if (CGLChoosePixelFormat(attribs, &pix, &nPix) != kCGLNoError || !pix) return false;
    CGLContextObj ctx = 0;
    CGLError err = CGLCreateContext(pix, 0, &ctx);
    m_cglPixelFormat = pix;
    if (err != kCGLNoError || !ctx) return false;
    m_cglContext = ctx;
    CGLSetCurrentContext(ctx);

    boost::shared_ptr<GLES20RenderTarget> rt(new GLES20RenderTarget());
    rt->Create("default", mode.color == DisplayMode::ColorDef::RGBA8888 ? "T8888 D24" : "T565 D16", mode.size);
    m_renderTarget = rt;
    return true;
}

void MacDisplay::Cleanup() {
    m_renderTarget.reset();
    m_postEffectTargets.clear();
    if (m_cglContext) { CGLSetCurrentContext(0); CGLDestroyContext((CGLContextObj)m_cglContext); m_cglContext = 0; }
    if (m_cglPixelFormat) { CGLDestroyPixelFormat((CGLPixelFormatObj)m_cglPixelFormat); m_cglPixelFormat = 0; }
}

void* MacDisplay::GetContext() { return m_cglContext; }

void* MacDisplay::CreateSharedContext() {
    CGLContextObj shared = 0;
    if (!m_cglPixelFormat) return 0;
    CGLCreateContext((CGLPixelFormatObj)m_cglPixelFormat, (CGLContextObj)m_cglContext, &shared);
    return shared;
}

bool MacDisplay::SetThreadContext(void *ctx) { return CGLSetCurrentContext((CGLContextObj)ctx) == kCGLNoError; }
bool MacDisplay::MakeCurrent() { return SetThreadContext(m_cglContext); }
void MacDisplay::Disconnect() { CGLSetCurrentContext(0); }

void MacDisplay::Swap() { glFlush(); }

safe_enum<Display::StateDef,Display::StateDef::type> MacDisplay::GetState() { return StateDef::Active; }

DisplayMode MacDisplay::GetMode() { return m_mode; }

void MacDisplay::SetOrientation(safe_enum<OrientationDef,OrientationDef::type>) {}
safe_enum<Display::OrientationDef,Display::OrientationDef::type> MacDisplay::GetOrientation() { return OrientationDef::Portrait; }

math::vec2<unsigned int> MacDisplay::GetSize() { return m_mode.size; }
math::vec2<unsigned int> MacDisplay::GetNativeSize() { return m_mode.size; }

const boost::shared_ptr<RenderTarget> &MacDisplay::GetRenderTarget() { return m_renderTarget; }

void MacDisplay::Resize(math::vec2<unsigned int> size) {
    if (!m_renderTarget || (m_mode.size.x == size.x && m_mode.size.y == size.y)) return;
    m_mode.size = size;
    // The framebuffer is unbound through the engine's bind cache first, so the cache does not name the deleted one
    jet::video::gles::Interface gl;
    gl.iglBindFramebuffer(0x8D40 /*GL_FRAMEBUFFER*/, 0);
    m_renderTarget->Destroy();
    m_renderTarget->Create("default", m_mode.color == DisplayMode::ColorDef::RGBA8888 ? "T8888 D24" : "T565 D16", size);
}

boost::shared_ptr<RenderTarget> MacDisplay::CreateCompatibleRenderTarget() {
    boost::shared_ptr<GLES20RenderTarget> rt(new GLES20RenderTarget());
    rt->Create("compatible", "T8888 D24", m_mode.size);
    return rt;
}

const boost::shared_ptr<RenderTarget> &MacDisplay::CreatePostEffectRenderTarget(unsigned w, unsigned h) {
    boost::shared_ptr<GLES20RenderTarget> rt(new GLES20RenderTarget());
    rt->Create("posteffect", "T8888 D24", math::vec2<unsigned int>(w, h));
    m_postEffectTargets.push_back(rt);
    return m_postEffectTargets.back();
}

boost::shared_ptr<RenderTarget> MacDisplay::GetPostEffectRenderTarget(unsigned idx) {
    return idx < m_postEffectTargets.size() ? m_postEffectTargets[idx] : boost::shared_ptr<RenderTarget>();
}

bool MacDisplay::IsLocked() { return m_locked; }
void MacDisplay::SetLocked(bool l) { m_locked = l; }

}

#endif

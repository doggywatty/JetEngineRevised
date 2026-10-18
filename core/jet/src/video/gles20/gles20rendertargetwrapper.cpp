#include <cstdio>
#include <cstdlib>
// core/jet/src/video/gles20/gles20rendertargetwrapper.cpp
#include "video/gles20/gles20rendertargetwrapper.h"
#include "video/gles20/gles20texture.h"
#include "video/driver.h"
#include <algorithm>

namespace jet::video {

GLES20RenderTargetWrapper::GLES20RenderTargetWrapper(unsigned fboId)
    : m_fboId(fboId), m_msaaFboId(0), m_wasRenderedInto(false), m_clearColor(0, 0, 0, 1),
      m_clearColorBuffer(true), m_clearDepthBuffer(true), m_clearPerFrame(true),
      m_hasColorTarget(true), m_hasDepthTarget(false), m_hasStencilTarget(false),
      m_scissorTestEnabled(false), m_lastRenderedFrameIdx(0xffffffff) {
}
GLES20RenderTargetWrapper::~GLES20RenderTargetWrapper() {}

void GLES20RenderTargetWrapper::Create(const jet::String &name, const jet::String &format, const math::vec2<unsigned int> &size) {
    m_name = name;
    m_format = format;
    SetSize(size);
    SetAllocatedSize(size);
}

void GLES20RenderTargetWrapper::Destroy() { Unload(); }
void GLES20RenderTargetWrapper::Unload() {
    gles::Interface gl;
    if (m_fboId) { gl.iglDeleteFramebuffers(1, &m_fboId); m_fboId = 0; }
    if (m_msaaFboId) { gl.iglDeleteFramebuffers(1, &m_msaaFboId); m_msaaFboId = 0; }
    m_colorTexture.reset();
    m_depthTexture.reset();
}
bool GLES20RenderTargetWrapper::IsValid() { return m_fboId != 0 || (m_size.x > 0 && m_size.y > 0); }

const jet::String &GLES20RenderTargetWrapper::GetName() { return m_name; }
const jet::String &GLES20RenderTargetWrapper::GetFormat() { return m_format; }

const boost::shared_ptr<Texture> &GLES20RenderTargetWrapper::GetColorTexture() { return m_colorTexture; }
void GLES20RenderTargetWrapper::SetColorTexture(boost::shared_ptr<Texture> t) {
    m_colorTexture = t;
    m_hasColorTarget = t.get() != 0;
    if (!m_fboId) {
        gles::Interface gl;
        gl.iglGenFramebuffers(1, &m_fboId);
        gl.iglBindFramebuffer(0x8D40 /*GL_FRAMEBUFFER*/, m_fboId);
        if (t) {
            GLES20Texture *gt = static_cast<GLES20Texture*>(t.get());
            gl.iglFramebufferTexture2D(0x8D40, 0x8CE0 /*GL_COLOR_ATTACHMENT0*/, 0x0DE1 /*GL_TEXTURE_2D*/, gt->GetGLId(), 0);
        }
    }
}
const boost::shared_ptr<Texture> &GLES20RenderTargetWrapper::GetDepthTexture() { return m_depthTexture; }

bool GLES20RenderTargetWrapper::HasColorTarget() { return m_hasColorTarget; }
bool GLES20RenderTargetWrapper::HasDepthTarget() { return m_hasDepthTarget; }
void GLES20RenderTargetWrapper::SetHasColorTarget(bool v) { m_hasColorTarget = v; }
void GLES20RenderTargetWrapper::SetHasDepthTarget(bool v) { m_hasDepthTarget = v; }
void GLES20RenderTargetWrapper::SetHasStencilTarget(bool v) { m_hasStencilTarget = v; }

void GLES20RenderTargetWrapper::SetClearColor(const math::vec4<float> &c) { m_clearColor = c; }
const math::vec4<float> &GLES20RenderTargetWrapper::GetClearColor() { return m_clearColor; }
void GLES20RenderTargetWrapper::SetClearDepthBuffer(bool v) { m_clearDepthBuffer = v; }
bool GLES20RenderTargetWrapper::GetClearDepthBuffer() { return m_clearDepthBuffer; }
void GLES20RenderTargetWrapper::SetClearColorBuffer(bool v) { m_clearColorBuffer = v; }
bool GLES20RenderTargetWrapper::GetClearColorBuffer() { return m_clearColorBuffer; }
void GLES20RenderTargetWrapper::SetClearPerFrame(bool v) { m_clearPerFrame = v; }
bool GLES20RenderTargetWrapper::GetClearPerFrame() { return m_clearPerFrame; }

const math::vec2<unsigned int> &GLES20RenderTargetWrapper::GetSize() { return m_size; }
const math::vec2<unsigned int> &GLES20RenderTargetWrapper::GetAllocatedSize() { return m_allocatedSize; }
void GLES20RenderTargetWrapper::SetSize(const math::vec2<unsigned int> &s) { m_size = s; }
void GLES20RenderTargetWrapper::SetAllocatedSize(const math::vec2<unsigned int> &s) { m_allocatedSize = s; }

// the scissor of the render target that is bound is applied at once (the draws that follow are clipped)
void GLES20RenderTargetWrapper::ApplyScissorIfBound() {
    Driver *d = Driver::GetCurrent();
    if (!d || d->GetCrtRenderTarget().get() != static_cast<RenderTarget*>(this)) return;
    gles::Interface gl;
    if (m_scissorTestEnabled) {
        gl.iglEnable(0x0C11 /*GL_SCISSOR_TEST*/);
        math::rect<unsigned int> r = GetHWScissorRect();
        gl.iglScissor(r.upperLeftCorner.x, r.upperLeftCorner.y, r.getWidth(), r.getHeight());
    } else {
        gl.iglDisable(0x0C11);
    }
}
void GLES20RenderTargetWrapper::SetScissorTest(bool v) { m_scissorTestEnabled = v; ApplyScissorIfBound(); }
bool GLES20RenderTargetWrapper::GetScissorTest() { return m_scissorTestEnabled; }
void GLES20RenderTargetWrapper::SetScissorRect(const math::rect<unsigned int> &r) { m_scissorRect = r; ApplyScissorIfBound(); }
const math::rect<unsigned int> &GLES20RenderTargetWrapper::GetScissorRect() { return m_scissorRect; }
// @0x6c1bbc: the scissor rectangle (corners included, the origin at the top left) in GL's bottom left origin; the
// display's rotation turns it for the other orientations (not ported: the display is never rotated here)
math::rect<unsigned int> GLES20RenderTargetWrapper::GetHWScissorRect() {
    const unsigned x = m_scissorRect.upperLeftCorner.x, y = m_scissorRect.upperLeftCorner.y;
    const unsigned w = m_scissorRect.lowerRightCorner.x + 1 - x, h = m_scissorRect.lowerRightCorner.y + 1 - y;
    const unsigned top = m_size.y - y;
    return math::rect<unsigned int>(x, top - h, x + w, top);
}

void GLES20RenderTargetWrapper::SetFBOId(unsigned id) { m_fboId = id; }
unsigned GLES20RenderTargetWrapper::GetFBOId() { return m_fboId; }
void GLES20RenderTargetWrapper::SetMSAAFBOId(unsigned id) { m_msaaFboId = id; }
unsigned GLES20RenderTargetWrapper::GetMSAAFBOId() { return m_msaaFboId; }

unsigned GLES20RenderTargetWrapper::GetLastRenderedFrameIdx() { return m_lastRenderedFrameIdx; }
void GLES20RenderTargetWrapper::SetLastRenderedFrameIdx(unsigned f) { m_lastRenderedFrameIdx = f; }

void GLES20RenderTargetWrapper::Bind() {
    gles::Interface gl;
    gl.iglBindFramebuffer(0x8D40 /*GL_FRAMEBUFFER*/, m_fboId);
    gl.iglViewport(0, 0, m_size.x, m_size.y);
    if (m_scissorTestEnabled) {
        gl.iglEnable(0x0C11 /*GL_SCISSOR_TEST*/);
        math::rect<unsigned int> r = GetHWScissorRect();
        gl.iglScissor(r.upperLeftCorner.x, r.upperLeftCorner.y, r.getWidth(), r.getHeight());
    } else {
        gl.iglDisable(0x0C11);
    }
    m_wasRenderedInto = true;
}

void GLES20RenderTargetWrapper::Activate() {
    Bind();
    if (GetClearPerFrame()) Clear();
}

void GLES20RenderTargetWrapper::Clear() {
    gles::Interface gl;
    gl.iglColorMask(true, true, true, true);  // a clear only touches what the masks let it
    gl.iglDepthMask(true);
    unsigned mask = 0;
    if (m_clearColorBuffer && m_hasColorTarget) { gl.iglClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w); mask |= 0x4000 /*GL_COLOR_BUFFER_BIT*/; }
    if (m_clearDepthBuffer && m_hasDepthTarget) { gl.iglClearDepthf(1.0f); mask |= 0x100 /*GL_DEPTH_BUFFER_BIT*/; }
    if (m_hasStencilTarget) { gl.iglClearStencil(0); mask |= 0x400 /*GL_STENCIL_BUFFER_BIT*/; }
    if (mask) gl.iglClear(mask);
}

}

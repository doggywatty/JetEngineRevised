// source/editor/world/viewport_display.cpp
#include "editor/world/viewport_display.h"

#include "video/gles20/gles20texture.h"

#include <cstdio>

namespace jeteditor::world {

using jet::video::DisplayMode;
using jet::video::RenderTarget;
using Base = jet::video::Display;

namespace {

// The frame's format: 8-bit RGBA colour and 24-bit depth, as the game's own targets use
const char *const kFrameFormat = "T8888 D24";
// The post-effect targets of the engine's display use this format
const char *const kPostEffectFormat = "T565 D16";

}  // namespace

ViewportDisplay::ViewportDisplay(jet::video::Display *window) : window_(window), size_(0, 0) {}

ViewportDisplay::~ViewportDisplay() { Release(); }

bool ViewportDisplay::Create(unsigned w, unsigned h, std::string &error) {
    Release();
    if (w == 0 || h == 0) {
        error = "the viewport has no size";
        return false;
    }
    if (!window_) {
        error = "the viewport has no GL display to draw with";
        return false;
    }
    target_ = RenderTarget::New();
    target_->Create("viewport", kFrameFormat, math::vec2<unsigned int>(w, h));
    if (!target_->IsValid() || !target_->GetColorTexture()) {
        error = "the viewport frame could not be created";
        Release();
        return false;
    }
    size_ = math::vec2<unsigned int>(w, h);
    return true;
}

void ViewportDisplay::Release() {
    target_.reset();
    postEffectTargets_.clear();
    size_ = math::vec2<unsigned int>(0, 0);
}

unsigned ViewportDisplay::ColorTexture() {
    if (!target_ || !target_->GetColorTexture()) return 0;
    return static_cast<unsigned>(static_cast<jet::video::GLES20Texture *>(target_->GetColorTexture().get())->GetGLId());
}

unsigned ViewportDisplay::GetClassId() { return 0x565044; /* 'VPD', the editor's offscreen display */ }
void *ViewportDisplay::GetContext() { return window_->GetContext(); }
void *ViewportDisplay::CreateSharedContext() { return window_->CreateSharedContext(); }
bool ViewportDisplay::SetThreadContext(void *ctx) { return window_->SetThreadContext(ctx); }

// The frame stays in its target. The editor draws it inside its panel after the game's frame.
void ViewportDisplay::Swap() {}

safe_enum<Base::StateDef, Base::StateDef::type> ViewportDisplay::GetState() { return Base::StateDef::Active; }

bool ViewportDisplay::Init(const DisplayMode &mode) {
    std::string error;
    return Create(mode.size.x, mode.size.y, error);
}

void ViewportDisplay::Cleanup() { Release(); }

DisplayMode ViewportDisplay::GetMode() {
    DisplayMode mode;
    mode.size = size_;
    return mode;
}

// The viewport is never rotated, so the orientation is fixed
void ViewportDisplay::SetOrientation(safe_enum<Base::OrientationDef, Base::OrientationDef::type>) {}
safe_enum<Base::OrientationDef, Base::OrientationDef::type> ViewportDisplay::GetOrientation() {
    return Base::OrientationDef::LandscapeLeft;
}

math::vec2<unsigned int> ViewportDisplay::GetSize() { return size_; }
math::vec2<unsigned int> ViewportDisplay::GetNativeSize() { return size_; }

const boost::shared_ptr<RenderTarget> &ViewportDisplay::GetRenderTarget() { return target_; }

boost::shared_ptr<RenderTarget> ViewportDisplay::CreateCompatibleRenderTarget() {
    boost::shared_ptr<RenderTarget> rt = RenderTarget::New();
    rt->Create("compatible", kFrameFormat, size_);
    return rt;
}

const boost::shared_ptr<RenderTarget> &ViewportDisplay::CreatePostEffectRenderTarget(unsigned w, unsigned h) {
    boost::shared_ptr<RenderTarget> rt = RenderTarget::New();
    char name[32];
    snprintf(name, sizeof(name), "effect%u", (unsigned)postEffectTargets_.size());
    rt->Create(name, kPostEffectFormat, math::vec2<unsigned int>(w, h));
    postEffectTargets_.push_back(rt);
    return postEffectTargets_.back();
}

boost::shared_ptr<RenderTarget> ViewportDisplay::GetPostEffectRenderTarget(unsigned idx) {
    return idx < postEffectTargets_.size() ? postEffectTargets_[idx] : boost::shared_ptr<RenderTarget>();
}

bool ViewportDisplay::IsLocked() { return locked_; }
void ViewportDisplay::SetLocked(bool locked) { locked_ = locked; }
bool ViewportDisplay::MakeCurrent() { return window_->SetThreadContext(window_->GetContext()); }
void ViewportDisplay::Disconnect() { window_->SetThreadContext(nullptr); }

}  // namespace jeteditor::world

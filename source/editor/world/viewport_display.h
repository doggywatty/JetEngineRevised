// source/editor/world/viewport_display.h
//
// The game's display inside the editor. The editor's window owns the GL context and the window surface; the game
// needs a display whose size is the viewport and whose render target is an offscreen frame the editor can draw.
// Game code reads the size (GUI layout, camera orientation, shader passes) and the render target from the display, so
// this is the only place the game learns that it is not in a window. Context calls go to the window's display.
#pragma once

#include "video/display.h"
#include "video/displaymode.h"
#include "video/rendertarget.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

namespace jeteditor::world {

class ViewportDisplay : public jet::video::Display {
public:
    // window: the display of the editor's window; its GL context is the one the game draws with.
    explicit ViewportDisplay(jet::video::Display *window);
    ~ViewportDisplay() override;

    // Allocates the frame (a colour texture and a 24-bit depth buffer) at w x h. Needs the window's context current.
    bool Create(unsigned w, unsigned h, std::string &error);
    // Frees the frame. The GL objects go with it, so the window's context must still be current.
    void Release();
    // The GL name of the colour texture of the frame, 0 when there is no frame.
    unsigned ColorTexture();

    unsigned GetClassId() override;
    void *GetContext() override;
    void *CreateSharedContext() override;
    bool SetThreadContext(void *ctx) override;
    void Swap() override;
    safe_enum<StateDef, StateDef::type> GetState() override;
    bool Init(const jet::video::DisplayMode &mode) override;
    void Cleanup() override;
    jet::video::DisplayMode GetMode() override;
    void SetOrientation(safe_enum<OrientationDef, OrientationDef::type>) override;
    safe_enum<OrientationDef, OrientationDef::type> GetOrientation() override;
    math::vec2<unsigned int> GetSize() override;
    math::vec2<unsigned int> GetNativeSize() override;
    const boost::shared_ptr<jet::video::RenderTarget> &GetRenderTarget() override;
    boost::shared_ptr<jet::video::RenderTarget> CreateCompatibleRenderTarget() override;
    const boost::shared_ptr<jet::video::RenderTarget> &CreatePostEffectRenderTarget(unsigned w, unsigned h) override;
    boost::shared_ptr<jet::video::RenderTarget> GetPostEffectRenderTarget(unsigned idx) override;
    bool IsLocked() override;

protected:
    void SetLocked(bool locked) override;
    bool MakeCurrent() override;
    void Disconnect() override;

private:
    jet::video::Display *window_;
    math::vec2<unsigned int> size_;
    boost::shared_ptr<jet::video::RenderTarget> target_;
    std::vector<boost::shared_ptr<jet::video::RenderTarget> > postEffectTargets_;
    bool locked_ = false;
};

}  // namespace jeteditor::world

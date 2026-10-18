// source/editor/ui/ui_gl.h
// Draws a ui::Context's draw list with GLES 2 on Linux (the engine's GL API, through EGL) and with GL 2.1 on macOS (the
// engine's CGL context). The font atlas is uploaded once; each frame's vertices go to one buffer, and each clip rectangle
// becomes a scissor. A command that names a texture (an image, such as the game's frame) is drawn with the image program
// instead of the font's.
#pragma once

#include "editor/ui/ui.h"

#include <cstdint>
#include <string>
#include <vector>

namespace jeteditor::ui {

class GlRenderer {
public:
    GlRenderer() = default;
    ~GlRenderer();
    GlRenderer(const GlRenderer &) = delete;
    GlRenderer &operator=(const GlRenderer &) = delete;

    // Needs a current GL context. Compiles the shader and uploads the atlas.
    bool Init(const Font &font, std::string &error);
    // The framebuffer the frame is drawn into and read back from: 0 is the window's own (Linux); macOS passes the
    // render target of its offscreen display. Set before each Draw(), because a resize makes a new framebuffer.
    void SetFramebuffer(unsigned framebuffer) { framebuffer_ = framebuffer; }
    void Draw(const Context &ctx, int framebufferWidth, int framebufferHeight, Color clear);
    // Reads the window framebuffer back as RGB rows, top row first. Call after Draw(), before the buffers swap.
    void ReadBack(int framebufferWidth, int framebufferHeight, std::vector<uint8_t> &rgb) const;
    void Shutdown();

private:
    unsigned program_ = 0, imageProgram_ = 0, texture_ = 0, buffer_ = 0, framebuffer_ = 0;
    int locPos_ = -1, locUv_ = -1, locColor_ = -1, locSize_ = -1, locTex_ = -1;
    int imageLocSize_ = -1, imageLocTex_ = -1;
};

}  // namespace jeteditor::ui

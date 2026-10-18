// source/editor/app/window.h
//
// The editor's window. On Linux it is an X11 window with the engine's EGL display on it (core/jet, JetEGLDisplay, the
// same GLES 2 setup the game uses there), in window.cpp. On macOS it is an NSWindow that shows the engine's offscreen
// MacDisplay, in window_mac.mm. Events are turned into ui::Input. On other platforms Open() says the window is not built
// yet, and the headless commands still work.
#pragma once

#include "editor/ui/ui.h"

#include <string>

namespace jet::video {
class Display;
}

namespace jeteditor::app {

class Window {
public:
    Window();
    ~Window();
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    bool Open(const std::string &title, int width, int height, std::string &error);
    void Close();
    bool IsOpen() const;

    // Takes every event waiting for the window. Sets closeRequested when the window manager asks to close it.
    // anyEvent says whether there was at least one event, so the loop can sleep when there was none.
    void Poll(ui::Input &input, bool &closeRequested, bool &anyEvent);
    void Size(int &width, int &height) const;
    void Swap();
    // The engine's display of this window (its GL context is current while the window is open), or null.
    jet::video::Display *GlDisplay() const;
    // The GL framebuffer the editor's frame goes into: the window's own (0) on Linux, the display's render target on macOS.
    unsigned Framebuffer() const;

private:
    struct Impl;
    Impl *impl_;
};

}  // namespace jeteditor::app

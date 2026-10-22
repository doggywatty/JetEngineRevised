// source/editor/app/platform.h
//
// The engine's GL display for the editor on this platform: the Mac's offscreen CGL display (core/jet, MacDisplay) on
// macOS, the WGL display (core/jet, Win32Display) on Windows, the EGL display everywhere else (core/jet,
// JetEGLDisplay, the same GLES 2 setup the game uses there). The same choice tests/testdisplay.h makes for the tests.
#pragma once

#if defined(__APPLE__)
#include "video/mac/macdisplay.h"
#elif defined(_WIN32)
#include "video/win32display.h"
#else
#include "video/egl/jetegldisplay.h"
#endif

namespace jeteditor::app {

#if defined(__APPLE__)
using PlatformDisplay = jet::video::MacDisplay;
#elif defined(_WIN32)
using PlatformDisplay = jet::video::Win32Display;
#else
using PlatformDisplay = jet::video::JetEGLDisplay;
#endif

}  // namespace jeteditor::app

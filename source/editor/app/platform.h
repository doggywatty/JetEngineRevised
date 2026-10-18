// source/editor/app/platform.h
//
// The engine's GL display for the editor on this platform: the Mac's offscreen CGL display (core/jet, MacDisplay) on
// macOS, the EGL display everywhere else. The same choice tests/testdisplay.h makes for the tests.
#pragma once

#if defined(__APPLE__)
#include "video/mac/macdisplay.h"
#else
#include "video/egl/jetegldisplay.h"
#endif

namespace jeteditor::app {

#if defined(__APPLE__)
using PlatformDisplay = jet::video::MacDisplay;
#else
using PlatformDisplay = jet::video::JetEGLDisplay;
#endif

}  // namespace jeteditor::app

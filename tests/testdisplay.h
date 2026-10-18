#pragma once
#if defined(__APPLE__)
#include "video/mac/macdisplay.h"
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
typedef jet::video::MacDisplay TestDisplay;
#else
#include "video/egl/jetegldisplay.h"
#include <GLES2/gl2.h>
typedef jet::video::JetEGLDisplay TestDisplay;
#endif

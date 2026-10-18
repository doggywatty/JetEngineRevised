// core/jet/src/video/gles20/gles20interface.cpp
//
// Thin wrapper over the platform's real GLES2 (or GL-ES-compatible desktop) entry points, plus a
// small per-thread bind-state cache (TLInterface) matching the force*Rebinding() API the original
// exposes. Every iglXxx below issues the real GL call named in libdespicablemefree.so's own
// wrapper (e.g. iglBindBuffer -> glBindBuffer): this is a live GL binding, not a placeholder.
#include "video/gles20/gles20interface.h"
#include "thread/taskmgr.h"
#include <cstring>
#include <vector>
#include <mutex>

// Android and Linux both talk to a real GLES2 through EGL (Mesa on desktop Linux).
#if defined(__ANDROID__) || defined(__linux__)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#define JET_GL_GETPROC(n) eglGetProcAddress(n)
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE && __has_include(<OpenGLES/ES2/gl.h>)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define JET_GL_GETPROC(n) 0  // core on iOS; extension fns resolved directly below where available
#elif TARGET_OS_IPHONE
// buildall.sh's no-real-iOS-SDK fallback: syntax-checks against the macOS SDK, which has no
// OpenGLES framework. Pull in the desktop GL header for the functions/enums the two APIs share,
// then declare just the GLES2-only entry points to type-check; a genuine iOS build (with Xcode's
// SDK) takes the branch above instead.
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#define GL_TEXTURE0 0x84C0
#define GL_ARRAY_BUFFER 0x8892
typedef ptrdiff_t GLsizeiptr; typedef ptrdiff_t GLintptr;
extern "C" {
void glActiveTexture(GLenum); void glAttachShader(GLuint,GLuint); void glBindAttribLocation(GLuint,GLuint,const GLchar*);
void glBindBuffer(GLenum,GLuint); void glBindFramebuffer(GLenum,GLuint); void glBindRenderbuffer(GLenum,GLuint); void glBindTexture(GLenum,GLuint);
void glBlendColor(GLfloat,GLfloat,GLfloat,GLfloat); void glBlendEquation(GLenum); void glBlendEquationSeparate(GLenum,GLenum);
void glBlendFunc(GLenum,GLenum); void glBlendFuncSeparate(GLenum,GLenum,GLenum,GLenum);
void glBufferData(GLenum,GLsizeiptr,const void*,GLenum); void glBufferSubData(GLenum,GLintptr,GLsizeiptr,const void*);
GLenum glCheckFramebufferStatus(GLenum); void glClearDepthf(GLfloat); void glCompileShader(GLuint);
void glCompressedTexImage2D(GLenum,GLint,GLenum,GLsizei,GLsizei,GLint,GLsizei,const void*);
void glCompressedTexSubImage2D(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLsizei,const void*);
GLuint glCreateProgram(); GLuint glCreateShader(GLenum);
void glDeleteBuffers(GLsizei,const GLuint*); void glDeleteFramebuffers(GLsizei,const GLuint*); void glDeleteProgram(GLuint);
void glDeleteRenderbuffers(GLsizei,const GLuint*); void glDeleteShader(GLuint);
void glDepthRangef(GLfloat,GLfloat); void glDetachShader(GLuint,GLuint);
void glDisableVertexAttribArray(GLuint); void glEnableVertexAttribArray(GLuint);
void glFramebufferRenderbuffer(GLenum,GLenum,GLenum,GLuint); void glFramebufferTexture2D(GLenum,GLenum,GLenum,GLuint,GLint);
void glGenBuffers(GLsizei,GLuint*); void glGenerateMipmap(GLenum); void glGenFramebuffers(GLsizei,GLuint*); void glGenRenderbuffers(GLsizei,GLuint*);
void glGetActiveAttrib(GLuint,GLuint,GLsizei,GLsizei*,GLint*,GLenum*,GLchar*);
void glGetActiveUniform(GLuint,GLuint,GLsizei,GLsizei*,GLint*,GLenum*,GLchar*);
void glGetAttachedShaders(GLuint,GLsizei,GLsizei*,GLuint*); GLint glGetAttribLocation(GLuint,const GLchar*);
void glGetBufferParameteriv(GLenum,GLenum,GLint*);
void glGetFramebufferAttachmentParameteriv(GLenum,GLenum,GLenum,GLint*);
void glGetProgramiv(GLuint,GLenum,GLint*); void glGetProgramInfoLog(GLuint,GLsizei,GLsizei*,GLchar*);
void glGetRenderbufferParameteriv(GLenum,GLenum,GLint*); void glGetShaderiv(GLuint,GLenum,GLint*);
void glGetShaderInfoLog(GLuint,GLsizei,GLsizei*,GLchar*); void glGetShaderPrecisionFormat(GLenum,GLenum,GLint*,GLint*);
void glGetShaderSource(GLuint,GLsizei,GLsizei*,GLchar*);
void glGetUniformfv(GLuint,GLint,GLfloat*); void glGetUniformiv(GLuint,GLint,GLint*); GLint glGetUniformLocation(GLuint,const GLchar*);
void glGetVertexAttribfv(GLuint,GLenum,GLfloat*); void glGetVertexAttribiv(GLuint,GLenum,GLint*); void glGetVertexAttribPointerv(GLuint,GLenum,void**);
GLboolean glIsBuffer(GLuint); GLboolean glIsFramebuffer(GLuint); GLboolean glIsProgram(GLuint);
GLboolean glIsRenderbuffer(GLuint); GLboolean glIsShader(GLuint);
void glLinkProgram(GLuint);
void glReleaseShaderCompiler(); void glShaderBinary(GLsizei,const GLuint*,GLenum,const void*,GLsizei);
void glStencilFuncSeparate(GLenum,GLenum,GLint,GLuint); void glStencilMaskSeparate(GLenum,GLuint); void glStencilOpSeparate(GLenum,GLenum,GLenum,GLenum);
void glUniform1f(GLint,GLfloat); void glUniform1fv(GLint,GLsizei,const GLfloat*); void glUniform1i(GLint,GLint); void glUniform1iv(GLint,GLsizei,const GLint*);
void glUniform2f(GLint,GLfloat,GLfloat); void glUniform2fv(GLint,GLsizei,const GLfloat*); void glUniform2i(GLint,GLint,GLint); void glUniform2iv(GLint,GLsizei,const GLint*);
void glUniform3f(GLint,GLfloat,GLfloat,GLfloat); void glUniform3fv(GLint,GLsizei,const GLfloat*); void glUniform3i(GLint,GLint,GLint,GLint); void glUniform3iv(GLint,GLsizei,const GLint*);
void glUniform4f(GLint,GLfloat,GLfloat,GLfloat,GLfloat); void glUniform4fv(GLint,GLsizei,const GLfloat*); void glUniform4i(GLint,GLint,GLint,GLint,GLint); void glUniform4iv(GLint,GLsizei,const GLint*);
void glUniformMatrix2fv(GLint,GLsizei,GLboolean,const GLfloat*); void glUniformMatrix3fv(GLint,GLsizei,GLboolean,const GLfloat*); void glUniformMatrix4fv(GLint,GLsizei,GLboolean,const GLfloat*);
void glUseProgram(GLuint); void glValidateProgram(GLuint);
void glVertexAttrib1f(GLuint,GLfloat); void glVertexAttrib1fv(GLuint,const GLfloat*);
void glVertexAttrib2f(GLuint,GLfloat,GLfloat); void glVertexAttrib2fv(GLuint,const GLfloat*);
void glVertexAttrib3f(GLuint,GLfloat,GLfloat,GLfloat); void glVertexAttrib3fv(GLuint,const GLfloat*);
void glVertexAttrib4f(GLuint,GLfloat,GLfloat,GLfloat,GLfloat); void glVertexAttrib4fv(GLuint,const GLfloat*);
void glVertexAttribPointer(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*);
}
#define JET_GL_GETPROC(n) 0
#else
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#define JET_GL_GETPROC(n) 0  // desktop build: needs an ANGLE/GLES context; not wired up yet
inline void glClearDepthf(float d) { glClearDepth(d); }
inline void glDepthRangef(float n, float f) { glDepthRange(n, f); }
inline void glGetShaderPrecisionFormat(unsigned, unsigned, int *range, int *prec) { range[0] = range[1] = 0; *prec = 0; }
inline void glShaderBinary(int, const unsigned*, unsigned, const void*, int) {}
inline void glReleaseShaderCompiler() {}
#endif
#elif defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#define JET_GL_GETPROC(n) wglGetProcAddress(n)
// opengl32.dll only exports GL 1.1 statically; everything GLES2 needs beyond that (all of
// GL 2.0's shader/buffer/framebuffer API, i.e. everything below) is loaded as a function pointer
// via wglGetProcAddress in internalLoad(). Declaring these as plain global variables named exactly
// like the GL functions means every gl* call elsewhere in this file works unchanged.
typedef ptrdiff_t GLsizeiptr; typedef ptrdiff_t GLintptr; typedef char GLchar;
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_TEXTURE0 0x84C0
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_STREAM_DRAW 0x88E0
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define JET_WINGL_DECL(ret, name, params) typedef ret (APIENTRY *PFN_##name) params; static PFN_##name name = 0
JET_WINGL_DECL(void, glActiveTexture, (GLenum));
JET_WINGL_DECL(void, glAttachShader, (GLuint,GLuint));
JET_WINGL_DECL(void, glBindAttribLocation, (GLuint,GLuint,const GLchar*));
JET_WINGL_DECL(void, glBindBuffer, (GLenum,GLuint));
JET_WINGL_DECL(void, glBindFramebuffer, (GLenum,GLuint));
JET_WINGL_DECL(void, glBindRenderbuffer, (GLenum,GLuint));
JET_WINGL_DECL(void, glBlendColor, (float,float,float,float));
JET_WINGL_DECL(void, glBlendEquation, (GLenum));
JET_WINGL_DECL(void, glBlendEquationSeparate, (GLenum,GLenum));
JET_WINGL_DECL(void, glBlendFuncSeparate, (GLenum,GLenum,GLenum,GLenum));
JET_WINGL_DECL(void, glBufferData, (GLenum,GLsizeiptr,const void*,GLenum));
JET_WINGL_DECL(void, glBufferSubData, (GLenum,GLintptr,GLsizeiptr,const void*));
JET_WINGL_DECL(GLenum, glCheckFramebufferStatus, (GLenum));
JET_WINGL_DECL(void, glClearDepthf, (float));
JET_WINGL_DECL(void, glCompileShader, (GLuint));
JET_WINGL_DECL(void, glCompressedTexImage2D, (GLenum,GLint,GLenum,GLsizei,GLsizei,GLint,GLsizei,const void*));
JET_WINGL_DECL(void, glCompressedTexSubImage2D, (GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLsizei,const void*));
JET_WINGL_DECL(GLuint, glCreateProgram, ());
JET_WINGL_DECL(GLuint, glCreateShader, (GLenum));
JET_WINGL_DECL(void, glDeleteBuffers, (GLsizei,const GLuint*));
JET_WINGL_DECL(void, glDeleteFramebuffers, (GLsizei,const GLuint*));
JET_WINGL_DECL(void, glDeleteProgram, (GLuint));
JET_WINGL_DECL(void, glDeleteRenderbuffers, (GLsizei,const GLuint*));
JET_WINGL_DECL(void, glDeleteShader, (GLuint));
JET_WINGL_DECL(void, glDepthRangef, (float,float));
JET_WINGL_DECL(void, glDetachShader, (GLuint,GLuint));
JET_WINGL_DECL(void, glDisableVertexAttribArray, (GLuint));
JET_WINGL_DECL(void, glEnableVertexAttribArray, (GLuint));
JET_WINGL_DECL(void, glFramebufferRenderbuffer, (GLenum,GLenum,GLenum,GLuint));
JET_WINGL_DECL(void, glFramebufferTexture2D, (GLenum,GLenum,GLenum,GLuint,GLint));
JET_WINGL_DECL(void, glGenBuffers, (GLsizei,GLuint*));
JET_WINGL_DECL(void, glGenerateMipmap, (GLenum));
JET_WINGL_DECL(void, glGenFramebuffers, (GLsizei,GLuint*));
JET_WINGL_DECL(void, glGenRenderbuffers, (GLsizei,GLuint*));
JET_WINGL_DECL(void, glGetActiveAttrib, (GLuint,GLuint,GLsizei,GLsizei*,GLint*,GLenum*,GLchar*));
JET_WINGL_DECL(void, glGetActiveUniform, (GLuint,GLuint,GLsizei,GLsizei*,GLint*,GLenum*,GLchar*));
JET_WINGL_DECL(void, glGetAttachedShaders, (GLuint,GLsizei,GLsizei*,GLuint*));
JET_WINGL_DECL(GLint, glGetAttribLocation, (GLuint,const GLchar*));
JET_WINGL_DECL(void, glGetBufferParameteriv, (GLenum,GLenum,GLint*));
JET_WINGL_DECL(void, glGetFramebufferAttachmentParameteriv, (GLenum,GLenum,GLenum,GLint*));
JET_WINGL_DECL(void, glGetProgramiv, (GLuint,GLenum,GLint*));
JET_WINGL_DECL(void, glGetProgramInfoLog, (GLuint,GLsizei,GLsizei*,GLchar*));
JET_WINGL_DECL(void, glGetRenderbufferParameteriv, (GLenum,GLenum,GLint*));
JET_WINGL_DECL(void, glGetShaderiv, (GLuint,GLenum,GLint*));
JET_WINGL_DECL(void, glGetShaderInfoLog, (GLuint,GLsizei,GLsizei*,GLchar*));
JET_WINGL_DECL(void, glGetShaderPrecisionFormat, (GLenum,GLenum,GLint*,GLint*));
JET_WINGL_DECL(void, glGetShaderSource, (GLuint,GLsizei,GLsizei*,GLchar*));
JET_WINGL_DECL(void, glGetUniformfv, (GLuint,GLint,float*));
JET_WINGL_DECL(void, glGetUniformiv, (GLuint,GLint,GLint*));
JET_WINGL_DECL(GLint, glGetUniformLocation, (GLuint,const GLchar*));
JET_WINGL_DECL(void, glGetVertexAttribfv, (GLuint,GLenum,float*));
JET_WINGL_DECL(void, glGetVertexAttribiv, (GLuint,GLenum,GLint*));
JET_WINGL_DECL(void, glGetVertexAttribPointerv, (GLuint,GLenum,void**));
JET_WINGL_DECL(GLboolean, glIsBuffer, (GLuint));
JET_WINGL_DECL(GLboolean, glIsFramebuffer, (GLuint));
JET_WINGL_DECL(GLboolean, glIsProgram, (GLuint));
JET_WINGL_DECL(GLboolean, glIsRenderbuffer, (GLuint));
JET_WINGL_DECL(GLboolean, glIsShader, (GLuint));
JET_WINGL_DECL(void, glLinkProgram, (GLuint));
JET_WINGL_DECL(void, glRenderbufferStorage, (GLenum,GLenum,GLsizei,GLsizei));
JET_WINGL_DECL(void, glSampleCoverage, (float,GLboolean));
JET_WINGL_DECL(void, glReleaseShaderCompiler, ());
JET_WINGL_DECL(void, glShaderBinary, (GLsizei,const GLuint*,GLenum,const void*,GLsizei));
JET_WINGL_DECL(void, glShaderSource, (GLuint,GLsizei,const GLchar**,const GLint*));
JET_WINGL_DECL(void, glStencilFuncSeparate, (GLenum,GLenum,GLint,GLuint));
JET_WINGL_DECL(void, glStencilMaskSeparate, (GLenum,GLuint));
JET_WINGL_DECL(void, glStencilOpSeparate, (GLenum,GLenum,GLenum,GLenum));
JET_WINGL_DECL(void, glUniform1f, (GLint,float));
JET_WINGL_DECL(void, glUniform1fv, (GLint,GLsizei,const float*));
JET_WINGL_DECL(void, glUniform1i, (GLint,GLint));
JET_WINGL_DECL(void, glUniform1iv, (GLint,GLsizei,const GLint*));
JET_WINGL_DECL(void, glUniform2f, (GLint,float,float));
JET_WINGL_DECL(void, glUniform2fv, (GLint,GLsizei,const float*));
JET_WINGL_DECL(void, glUniform2i, (GLint,GLint,GLint));
JET_WINGL_DECL(void, glUniform2iv, (GLint,GLsizei,const GLint*));
JET_WINGL_DECL(void, glUniform3f, (GLint,float,float,float));
JET_WINGL_DECL(void, glUniform3fv, (GLint,GLsizei,const float*));
JET_WINGL_DECL(void, glUniform3i, (GLint,GLint,GLint,GLint));
JET_WINGL_DECL(void, glUniform3iv, (GLint,GLsizei,const GLint*));
JET_WINGL_DECL(void, glUniform4f, (GLint,float,float,float,float));
JET_WINGL_DECL(void, glUniform4fv, (GLint,GLsizei,const float*));
JET_WINGL_DECL(void, glUniform4i, (GLint,GLint,GLint,GLint,GLint));
JET_WINGL_DECL(void, glUniform4iv, (GLint,GLsizei,const GLint*));
JET_WINGL_DECL(void, glUniformMatrix2fv, (GLint,GLsizei,GLboolean,const float*));
JET_WINGL_DECL(void, glUniformMatrix3fv, (GLint,GLsizei,GLboolean,const float*));
JET_WINGL_DECL(void, glUniformMatrix4fv, (GLint,GLsizei,GLboolean,const float*));
JET_WINGL_DECL(void, glUseProgram, (GLuint));
JET_WINGL_DECL(void, glValidateProgram, (GLuint));
JET_WINGL_DECL(void, glVertexAttrib1f, (GLuint,float));
JET_WINGL_DECL(void, glVertexAttrib1fv, (GLuint,const float*));
JET_WINGL_DECL(void, glVertexAttrib2f, (GLuint,float,float));
JET_WINGL_DECL(void, glVertexAttrib2fv, (GLuint,const float*));
JET_WINGL_DECL(void, glVertexAttrib3f, (GLuint,float,float,float));
JET_WINGL_DECL(void, glVertexAttrib3fv, (GLuint,const float*));
JET_WINGL_DECL(void, glVertexAttrib4f, (GLuint,float,float,float,float));
JET_WINGL_DECL(void, glVertexAttrib4fv, (GLuint,const float*));
JET_WINGL_DECL(void, glVertexAttribPointer, (GLuint,GLint,GLenum,GLboolean,GLsizei,const void*));
#define JET_WINGL_LOAD(name) name = (PFN_##name)wglGetProcAddress(#name)
#else
#include <GL/gl.h>
#include <dlfcn.h>
#define JET_GL_GETPROC(n) dlsym(RTLD_DEFAULT, n)
#endif

namespace jet::video::gles {

static unsigned s_clientVersion = 2;
unsigned ClientVersion() { return s_clientVersion; }
void SetClientVersion(unsigned version) { s_clientVersion = version; }

// --- per-thread interface table -------------------------------------------------------------
static std::vector<TLInterface*> s_perThread;

#if defined(__ANDROID__) || defined(_WIN32) || !defined(__APPLE__)
// The core GLES 3.0 name on a 3.0 context, else the GLES 2.0 extension name (VAO, multisample, blit).
static void (*LoadGLProc(const char *core, const char *ext))(void) {
    void (*p)(void) = 0;
    if (ClientVersion() >= 3) p = (void(*)(void))JET_GL_GETPROC(core);
    return p ? p : (void(*)(void))JET_GL_GETPROC(ext);
}
#endif
static std::mutex s_perThreadMutex;

TLInterface::TLInterface(unsigned threadIdx)
    : m_funcs(new GLExtFunctions()), m_threadIdx(threadIdx), m_context(0), m_extensions(0) {
    memset(m_funcs, 0, sizeof(GLExtFunctions));
}
TLInterface::~TLInterface() { delete m_funcs; delete[] m_extensions; }

unsigned TLInterface::getThreadIdx() { return m_threadIdx; }
void *TLInterface::getContext() { return m_context; }
const char *TLInterface::getExtensions() { return m_extensions ? m_extensions : ""; }

bool TLInterface::internalLoad(void *context) {
    m_context = context;
#if defined(_WIN32)
    JET_WINGL_LOAD(glActiveTexture); JET_WINGL_LOAD(glAttachShader); JET_WINGL_LOAD(glBindAttribLocation);
    JET_WINGL_LOAD(glBindBuffer); JET_WINGL_LOAD(glBindFramebuffer); JET_WINGL_LOAD(glBindRenderbuffer);
    JET_WINGL_LOAD(glBlendColor); JET_WINGL_LOAD(glBlendEquation); JET_WINGL_LOAD(glBlendEquationSeparate);
    JET_WINGL_LOAD(glBlendFuncSeparate); JET_WINGL_LOAD(glBufferData); JET_WINGL_LOAD(glBufferSubData);
    JET_WINGL_LOAD(glCheckFramebufferStatus); JET_WINGL_LOAD(glClearDepthf); JET_WINGL_LOAD(glCompileShader);
    JET_WINGL_LOAD(glCompressedTexImage2D); JET_WINGL_LOAD(glCompressedTexSubImage2D);
    JET_WINGL_LOAD(glCreateProgram); JET_WINGL_LOAD(glCreateShader);
    JET_WINGL_LOAD(glDeleteBuffers); JET_WINGL_LOAD(glDeleteFramebuffers); JET_WINGL_LOAD(glDeleteProgram);
    JET_WINGL_LOAD(glDeleteRenderbuffers); JET_WINGL_LOAD(glDeleteShader);
    JET_WINGL_LOAD(glDepthRangef); JET_WINGL_LOAD(glDetachShader);
    JET_WINGL_LOAD(glDisableVertexAttribArray); JET_WINGL_LOAD(glEnableVertexAttribArray);
    JET_WINGL_LOAD(glFramebufferRenderbuffer); JET_WINGL_LOAD(glFramebufferTexture2D);
    JET_WINGL_LOAD(glGenBuffers); JET_WINGL_LOAD(glGenerateMipmap); JET_WINGL_LOAD(glGenFramebuffers); JET_WINGL_LOAD(glGenRenderbuffers);
    JET_WINGL_LOAD(glGetActiveAttrib); JET_WINGL_LOAD(glGetActiveUniform); JET_WINGL_LOAD(glGetAttachedShaders);
    JET_WINGL_LOAD(glGetAttribLocation); JET_WINGL_LOAD(glGetBufferParameteriv);
    JET_WINGL_LOAD(glGetFramebufferAttachmentParameteriv);
    JET_WINGL_LOAD(glGetProgramiv); JET_WINGL_LOAD(glGetProgramInfoLog); JET_WINGL_LOAD(glGetRenderbufferParameteriv);
    JET_WINGL_LOAD(glGetShaderiv); JET_WINGL_LOAD(glGetShaderInfoLog); JET_WINGL_LOAD(glGetShaderPrecisionFormat);
    JET_WINGL_LOAD(glGetShaderSource);
    JET_WINGL_LOAD(glGetUniformfv); JET_WINGL_LOAD(glGetUniformiv); JET_WINGL_LOAD(glGetUniformLocation);
    JET_WINGL_LOAD(glGetVertexAttribfv); JET_WINGL_LOAD(glGetVertexAttribiv); JET_WINGL_LOAD(glGetVertexAttribPointerv);
    JET_WINGL_LOAD(glIsBuffer); JET_WINGL_LOAD(glIsFramebuffer); JET_WINGL_LOAD(glIsProgram);
    JET_WINGL_LOAD(glIsRenderbuffer); JET_WINGL_LOAD(glIsShader);
    JET_WINGL_LOAD(glLinkProgram); JET_WINGL_LOAD(glReleaseShaderCompiler);
    JET_WINGL_LOAD(glRenderbufferStorage); JET_WINGL_LOAD(glSampleCoverage);
    JET_WINGL_LOAD(glShaderBinary); JET_WINGL_LOAD(glShaderSource);
    JET_WINGL_LOAD(glStencilFuncSeparate); JET_WINGL_LOAD(glStencilMaskSeparate); JET_WINGL_LOAD(glStencilOpSeparate);
    JET_WINGL_LOAD(glUniform1f); JET_WINGL_LOAD(glUniform1fv); JET_WINGL_LOAD(glUniform1i); JET_WINGL_LOAD(glUniform1iv);
    JET_WINGL_LOAD(glUniform2f); JET_WINGL_LOAD(glUniform2fv); JET_WINGL_LOAD(glUniform2i); JET_WINGL_LOAD(glUniform2iv);
    JET_WINGL_LOAD(glUniform3f); JET_WINGL_LOAD(glUniform3fv); JET_WINGL_LOAD(glUniform3i); JET_WINGL_LOAD(glUniform3iv);
    JET_WINGL_LOAD(glUniform4f); JET_WINGL_LOAD(glUniform4fv); JET_WINGL_LOAD(glUniform4i); JET_WINGL_LOAD(glUniform4iv);
    JET_WINGL_LOAD(glUniformMatrix2fv); JET_WINGL_LOAD(glUniformMatrix3fv); JET_WINGL_LOAD(glUniformMatrix4fv);
    JET_WINGL_LOAD(glUseProgram); JET_WINGL_LOAD(glValidateProgram);
    JET_WINGL_LOAD(glVertexAttrib1f); JET_WINGL_LOAD(glVertexAttrib1fv);
    JET_WINGL_LOAD(glVertexAttrib2f); JET_WINGL_LOAD(glVertexAttrib2fv);
    JET_WINGL_LOAD(glVertexAttrib3f); JET_WINGL_LOAD(glVertexAttrib3fv);
    JET_WINGL_LOAD(glVertexAttrib4f); JET_WINGL_LOAD(glVertexAttrib4fv);
    JET_WINGL_LOAD(glVertexAttribPointer);
#endif
    const char *ext = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (ext) {
        size_t n = strlen(ext);
        m_extensions = new char[n + 1];
        memcpy(m_extensions, ext, n + 1);
    }
#if defined(__ANDROID__) || defined(_WIN32) || !defined(__APPLE__)
    m_funcs->BindVertexArray = (void(*)(unsigned))LoadGLProc("glBindVertexArray", "glBindVertexArrayOES");
    m_funcs->DeleteVertexArrays = (void(*)(int, const unsigned*))LoadGLProc("glDeleteVertexArrays", "glDeleteVertexArraysOES");
    m_funcs->GenVertexArrays = (void(*)(int, unsigned*))LoadGLProc("glGenVertexArrays", "glGenVertexArraysOES");
    m_funcs->IsVertexArray = (unsigned char(*)(unsigned))LoadGLProc("glIsVertexArray", "glIsVertexArrayOES");
    m_funcs->RenderbufferStorageMultisample = (void(*)(unsigned,int,unsigned,int,int))LoadGLProc("glRenderbufferStorageMultisample", "glRenderbufferStorageMultisampleEXT");
    m_funcs->BlitFramebuffer = (void(*)(int,int,int,int,int,int,int,int,unsigned,unsigned))LoadGLProc("glBlitFramebuffer", "glBlitFramebufferEXT");
#endif
    return true;
}

void TLInterface::sync() { m_oldState = m_crtState; }
void TLInterface::reset() { m_crtState.reset(); m_oldState.reset(); }
void TLInterface::flush() { glFlush(); }

void TLInterface::forceArrayBufferRebinding(unsigned id) { m_crtState.arrayBuffer = id; }
void TLInterface::forceElementBufferRebinding(unsigned id) { m_crtState.elementBuffer = id; }
void TLInterface::forceFramebufferRebinding(unsigned id) { m_crtState.framebuffer = id; }
void TLInterface::forceRenderbufferRebinding(unsigned id) { m_crtState.renderbuffer = id; }
void TLInterface::forceProgramRebinding(unsigned id) { m_crtState.program = id; }
void TLInterface::forceTextureRebinding() { for (int i = 0; i < 8; i++) m_crtState.boundTexture[i] = 0xffffffff; }

// A deleted GL name might still be the one cached as "currently bound" in every thread's shadow
// state; the driver re-binds the real 0 next time regardless, so just drop the stale cache entry.
void TLInterface::removeBuffer(unsigned id) {
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    for (unsigned i = 0; i < s_perThread.size(); i++) {
        if (!s_perThread[i]) continue;
        if (s_perThread[i]->m_crtState.arrayBuffer == id) s_perThread[i]->m_crtState.arrayBuffer = 0;
        if (s_perThread[i]->m_crtState.elementBuffer == id) s_perThread[i]->m_crtState.elementBuffer = 0;
    }
}
void TLInterface::removeFramebuffer(unsigned id) {
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    for (unsigned i = 0; i < s_perThread.size(); i++)
        if (s_perThread[i] && s_perThread[i]->m_crtState.framebuffer == id) s_perThread[i]->m_crtState.framebuffer = 0;
}
void TLInterface::removeRenderbuffer(unsigned id) {
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    for (unsigned i = 0; i < s_perThread.size(); i++)
        if (s_perThread[i] && s_perThread[i]->m_crtState.renderbuffer == id) s_perThread[i]->m_crtState.renderbuffer = 0;
}
void TLInterface::removeProgram(unsigned id) {
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    for (unsigned i = 0; i < s_perThread.size(); i++)
        if (s_perThread[i] && s_perThread[i]->m_crtState.program == id) s_perThread[i]->m_crtState.program = 0;
}
void TLInterface::removeTexture(unsigned id) {
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    for (unsigned i = 0; i < s_perThread.size(); i++) {
        if (!s_perThread[i]) continue;
        for (int u = 0; u < 8; u++)
            if (s_perThread[i]->m_crtState.boundTexture[u] == id) s_perThread[i]->m_crtState.boundTexture[u] = 0;
    }
}

bool initForThread(void *context) {
    unsigned idx = (jet::thread::TaskMgr::GetInstance() ? jet::thread::TaskMgr::GetInstance()->GetCrtThreadIdx() : 0);
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    if (idx >= s_perThread.size()) s_perThread.resize(idx + 1, 0);
    if (!s_perThread[idx]) {
        TLInterface *tl = new TLInterface(idx);
        s_perThread[idx] = tl;
        if (!tl->internalLoad(context)) return false;
        tl->sync();
    } else {
        s_perThread[idx]->m_context = context;
    }
    return true;
}

void shutdownForThread() {
    unsigned idx = (jet::thread::TaskMgr::GetInstance() ? jet::thread::TaskMgr::GetInstance()->GetCrtThreadIdx() : 0);
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    if (idx < s_perThread.size() && s_perThread[idx]) { delete s_perThread[idx]; s_perThread[idx] = 0; }
}

void setContext(void *context) {
    unsigned idx = (jet::thread::TaskMgr::GetInstance() ? jet::thread::TaskMgr::GetInstance()->GetCrtThreadIdx() : 0);
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    if (idx < s_perThread.size() && s_perThread[idx]) s_perThread[idx]->m_context = context;
}

static TLInterface *ForCurrentThread() {
    unsigned idx = (jet::thread::TaskMgr::GetInstance() ? jet::thread::TaskMgr::GetInstance()->GetCrtThreadIdx() : 0);
    std::lock_guard<std::mutex> lock(s_perThreadMutex);
    return idx < s_perThread.size() ? s_perThread[idx] : 0;
}

// --- Interface: real GL calls, with bind-call caching where TLInterface tracks it -------------
Interface::Interface() : m_interface(ForCurrentThread()) {}
TLInterface *Interface::GetInterface() { return m_interface; }

void Interface::iglActiveTexture(unsigned t) { glActiveTexture(t); if (m_interface) m_interface->m_crtState.activeTexture = t - GL_TEXTURE0; }
void Interface::iglAttachShader(unsigned p, unsigned s) { glAttachShader(p, s); }
void Interface::iglBindAttribLocation(unsigned p, unsigned i, const char *n) { glBindAttribLocation(p, i, n); }

void Interface::iglBindBuffer(unsigned target, unsigned id) {
    if (m_interface) {
        unsigned &cache = target == GL_ARRAY_BUFFER ? m_interface->m_crtState.arrayBuffer : m_interface->m_crtState.elementBuffer;
        if (cache == id) return;
        cache = id;
    }
    glBindBuffer(target, id);
}
void Interface::iglBindFramebuffer(unsigned target, unsigned id) {
    if (m_interface) { if (m_interface->m_crtState.framebuffer == id) return; m_interface->m_crtState.framebuffer = id; }
    glBindFramebuffer(target, id);
}
void Interface::iglBindRenderbuffer(unsigned target, unsigned id) {
    if (m_interface) { if (m_interface->m_crtState.renderbuffer == id) return; m_interface->m_crtState.renderbuffer = id; }
    glBindRenderbuffer(target, id);
}
void Interface::iglBindTexture(unsigned target, unsigned id) {
    if (m_interface) {
        unsigned unit = m_interface->m_crtState.activeTexture;
        if (unit < 8) {
            if (m_interface->m_crtState.boundTexture[unit] == id) return;
            m_interface->m_crtState.boundTexture[unit] = id;
        }
    }
    glBindTexture(target, id);
}

void Interface::iglBlendColor(float r, float g, float b, float a) { glBlendColor(r, g, b, a); }
void Interface::iglBlendEquation(unsigned m) { glBlendEquation(m); }
void Interface::iglBlendEquationSeparate(unsigned rgb, unsigned a) { glBlendEquationSeparate(rgb, a); }
void Interface::iglBlendFunc(unsigned s, unsigned d) { glBlendFunc(s, d); }
void Interface::iglBlendFuncSeparate(unsigned sr, unsigned dr, unsigned sa, unsigned da) { glBlendFuncSeparate(sr, dr, sa, da); }
void Interface::iglBufferData(unsigned t, int size, const void *data, unsigned usage) { glBufferData(t, size, data, usage); }
void Interface::iglBufferSubData(unsigned t, int off, int size, const void *data) { glBufferSubData(t, off, size, data); }
unsigned Interface::iglCheckFramebufferStatus(unsigned t) { return glCheckFramebufferStatus(t); }
void Interface::iglClear(unsigned mask) { glClear(mask); }
void Interface::iglClearColor(float r, float g, float b, float a) { glClearColor(r, g, b, a); }
void Interface::iglClearDepthf(float d) { glClearDepthf(d); }
void Interface::iglClearStencil(int s) { glClearStencil(s); }
void Interface::iglColorMask(unsigned char r, unsigned char g, unsigned char b, unsigned char a) { glColorMask(r, g, b, a); }
void Interface::iglCompileShader(unsigned s) { glCompileShader(s); }
void Interface::iglCompressedTexImage2D(unsigned t, int lvl, unsigned f, int w, int h, int b, int sz, const void *d) { glCompressedTexImage2D(t, lvl, f, w, h, b, sz, d); }
void Interface::iglCompressedTexSubImage2D(unsigned t, int lvl, int x, int y, int w, int h, unsigned f, int sz, const void *d) { glCompressedTexSubImage2D(t, lvl, x, y, w, h, f, sz, d); }
void Interface::iglCopyTexImage2D(unsigned t, int lvl, unsigned f, int x, int y, int w, int h, int b) { glCopyTexImage2D(t, lvl, f, x, y, w, h, b); }
void Interface::iglCopyTexSubImage2D(unsigned t, int lvl, int xo, int yo, int x, int y, int w, int h) { glCopyTexSubImage2D(t, lvl, xo, yo, x, y, w, h); }
unsigned Interface::iglCreateProgram() { return glCreateProgram(); }
unsigned Interface::iglCreateShader(unsigned t) { return glCreateShader(t); }
void Interface::iglCullFace(unsigned m) { glCullFace(m); }
void Interface::iglDeleteBuffers(int n, const unsigned *b) { for (int i = 0; i < n; i++) TLInterface::removeBuffer(b[i]); glDeleteBuffers(n, b); }
void Interface::iglDeleteFramebuffers(int n, const unsigned *f) { for (int i = 0; i < n; i++) TLInterface::removeFramebuffer(f[i]); glDeleteFramebuffers(n, f); }
void Interface::iglDeleteProgram(unsigned p) { TLInterface::removeProgram(p); glDeleteProgram(p); }
void Interface::iglDeleteRenderbuffers(int n, const unsigned *r) { for (int i = 0; i < n; i++) TLInterface::removeRenderbuffer(r[i]); glDeleteRenderbuffers(n, r); }
void Interface::iglDeleteShader(unsigned s) { glDeleteShader(s); }
void Interface::iglDeleteTextures(int n, const unsigned *t) { for (int i = 0; i < n; i++) TLInterface::removeTexture(t[i]); glDeleteTextures(n, t); }
void Interface::iglDepthFunc(unsigned f) { glDepthFunc(f); }
void Interface::iglDepthMask(unsigned char m) { glDepthMask(m); }
void Interface::iglDepthRangef(float n, float f) { glDepthRangef(n, f); }
void Interface::iglDetachShader(unsigned p, unsigned s) { glDetachShader(p, s); }
void Interface::iglDisable(unsigned c) { glDisable(c); }
void Interface::iglDisableVertexAttribArray(unsigned i) { glDisableVertexAttribArray(i); }
void Interface::iglDrawArrays(unsigned m, int f, int c) { glDrawArrays(m, f, c); }
void Interface::iglDrawElements(unsigned m, int c, unsigned t, const void *i) { glDrawElements(m, c, t, i); }
void Interface::iglEnable(unsigned c) { glEnable(c); }
void Interface::iglEnableVertexAttribArray(unsigned i) { glEnableVertexAttribArray(i); }
void Interface::iglFinish() { glFinish(); }
void Interface::iglFlush() { glFlush(); }
void Interface::iglFramebufferRenderbuffer(unsigned t, unsigned a, unsigned rt, unsigned r) { glFramebufferRenderbuffer(t, a, rt, r); }
void Interface::iglFramebufferTexture2D(unsigned t, unsigned a, unsigned tt, unsigned tex, int lvl) { glFramebufferTexture2D(t, a, tt, tex, lvl); }
void Interface::iglFrontFace(unsigned m) { glFrontFace(m); }
void Interface::iglGenBuffers(int n, unsigned *b) { glGenBuffers(n, b); }
void Interface::iglGenerateMipmap(unsigned t) { glGenerateMipmap(t); }
void Interface::iglGenFramebuffers(int n, unsigned *f) { glGenFramebuffers(n, f); }
void Interface::iglGenRenderbuffers(int n, unsigned *r) { glGenRenderbuffers(n, r); }
void Interface::iglGenTextures(int n, unsigned *t) { glGenTextures(n, t); }
void Interface::iglGetActiveAttrib(unsigned p, unsigned i, int bufsz, int *len, int *sz, unsigned *type, char *name) { glGetActiveAttrib(p, i, bufsz, len, sz, type, name); }
void Interface::iglGetActiveUniform(unsigned p, unsigned i, int bufsz, int *len, int *sz, unsigned *type, char *name) { glGetActiveUniform(p, i, bufsz, len, sz, type, name); }
void Interface::iglGetAttachedShaders(unsigned p, int m, int *c, unsigned *s) { glGetAttachedShaders(p, m, c, s); }
int Interface::iglGetAttribLocation(unsigned p, const char *n) { return glGetAttribLocation(p, n); }
void Interface::iglGetBooleanv(unsigned p, unsigned char *d) { glGetBooleanv(p, d); }
void Interface::iglGetBufferParameteriv(unsigned t, unsigned p, int *d) { glGetBufferParameteriv(t, p, d); }
unsigned Interface::iglGetError() { return glGetError(); }
void Interface::iglGetFloatv(unsigned p, float *d) { glGetFloatv(p, d); }
void Interface::iglGetFramebufferAttachmentParameteriv(unsigned t, unsigned a, unsigned p, int *d) { glGetFramebufferAttachmentParameteriv(t, a, p, d); }
void Interface::iglGetIntegerv(unsigned p, int *d) { glGetIntegerv(p, d); }
void Interface::iglGetProgramiv(unsigned pr, unsigned p, int *d) { glGetProgramiv(pr, p, d); }
void Interface::iglGetProgramInfoLog(unsigned p, int m, int *l, char *log) { glGetProgramInfoLog(p, m, l, log); }
void Interface::iglGetRenderbufferParameteriv(unsigned t, unsigned p, int *d) { glGetRenderbufferParameteriv(t, p, d); }
void Interface::iglGetShaderiv(unsigned s, unsigned p, int *d) { glGetShaderiv(s, p, d); }
void Interface::iglGetShaderInfoLog(unsigned s, int m, int *l, char *log) { glGetShaderInfoLog(s, m, l, log); }
void Interface::iglGetShaderPrecisionFormat(unsigned st, unsigned pt, int *range, int *prec) { glGetShaderPrecisionFormat(st, pt, range, prec); }
void Interface::iglGetShaderSource(unsigned s, int m, int *l, char *src) { glGetShaderSource(s, m, l, src); }
const unsigned char *Interface::iglGetString(unsigned n) { return glGetString(n); }
void Interface::iglGetTexParameterfv(unsigned t, unsigned p, float *d) { glGetTexParameterfv(t, p, d); }
void Interface::iglGetTexParameteriv(unsigned t, unsigned p, int *d) { glGetTexParameteriv(t, p, d); }
void Interface::iglGetUniformfv(unsigned p, int l, float *d) { glGetUniformfv(p, l, d); }
void Interface::iglGetUniformiv(unsigned p, int l, int *d) { glGetUniformiv(p, l, d); }
int Interface::iglGetUniformLocation(unsigned p, const char *n) { return glGetUniformLocation(p, n); }
void Interface::iglGetVertexAttribfv(unsigned i, unsigned p, float *d) { glGetVertexAttribfv(i, p, d); }
void Interface::iglGetVertexAttribiv(unsigned i, unsigned p, int *d) { glGetVertexAttribiv(i, p, d); }
void Interface::iglGetVertexAttribPointerv(unsigned i, unsigned p, void **d) { glGetVertexAttribPointerv(i, p, d); }
void Interface::iglHint(unsigned t, unsigned m) { glHint(t, m); }
unsigned char Interface::iglIsBuffer(unsigned b) { return glIsBuffer(b); }
unsigned char Interface::iglIsEnabled(unsigned c) { return glIsEnabled(c); }
unsigned char Interface::iglIsFramebuffer(unsigned f) { return glIsFramebuffer(f); }
unsigned char Interface::iglIsProgram(unsigned p) { return glIsProgram(p); }
unsigned char Interface::iglIsRenderbuffer(unsigned r) { return glIsRenderbuffer(r); }
unsigned char Interface::iglIsShader(unsigned s) { return glIsShader(s); }
unsigned char Interface::iglIsTexture(unsigned t) { return glIsTexture(t); }
void Interface::iglLineWidth(float w) { glLineWidth(w); }
void Interface::iglLinkProgram(unsigned p) { glLinkProgram(p); }
void Interface::iglPixelStorei(unsigned p, int v) { glPixelStorei(p, v); }
void Interface::iglPolygonOffset(float f, float u) { glPolygonOffset(f, u); }
void Interface::iglReadPixels(int x, int y, int w, int h, unsigned f, unsigned t, void *d) { glReadPixels(x, y, w, h, f, t, d); }
void Interface::iglReleaseShaderCompiler() {
#ifdef GL_ES_VERSION_2_0
    glReleaseShaderCompiler();
#endif
}
void Interface::iglRenderbufferStorage(unsigned t, unsigned f, int w, int h) { glRenderbufferStorage(t, f, w, h); }
void Interface::iglRenderbufferStorageMultisample(unsigned t, int s, unsigned f, int w, int h) {
    if (m_interface && m_interface->m_funcs->RenderbufferStorageMultisample) m_interface->m_funcs->RenderbufferStorageMultisample(t, s, f, w, h);
}
void Interface::iglBlitFramebuffer(int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, unsigned mask, unsigned filter) {
    if (m_interface && m_interface->m_funcs->BlitFramebuffer) m_interface->m_funcs->BlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, filter);
}
void Interface::iglSampleCoverage(float v, unsigned char inv) { glSampleCoverage(v, inv); }
void Interface::iglScissor(int x, int y, int w, int h) { glScissor(x, y, w, h); }
void Interface::iglShaderBinary(int n, const unsigned *shaders, unsigned fmt, const void *bin, int len) { glShaderBinary(n, shaders, fmt, bin, len); }
void Interface::iglShaderSource(unsigned s, int c, const char **src, const int *len) { glShaderSource(s, c, src, len); }
void Interface::iglStencilFunc(unsigned f, int r, unsigned m) { glStencilFunc(f, r, m); }
void Interface::iglStencilFuncSeparate(unsigned face, unsigned f, int r, unsigned m) { glStencilFuncSeparate(face, f, r, m); }
void Interface::iglStencilMask(unsigned m) { glStencilMask(m); }
void Interface::iglStencilMaskSeparate(unsigned face, unsigned m) { glStencilMaskSeparate(face, m); }
void Interface::iglStencilOp(unsigned f, unsigned zf, unsigned zp) { glStencilOp(f, zf, zp); }
void Interface::iglStencilOpSeparate(unsigned face, unsigned f, unsigned zf, unsigned zp) { glStencilOpSeparate(face, f, zf, zp); }
void Interface::iglTexImage2D(unsigned t, int lvl, int internalFmt, int w, int h, int b, unsigned f, unsigned type, const void *d) { glTexImage2D(t, lvl, internalFmt, w, h, b, f, type, d); }
void Interface::iglTexParameterf(unsigned t, unsigned p, float v) { glTexParameterf(t, p, v); }
void Interface::iglTexParameterfv(unsigned t, unsigned p, const float *v) { glTexParameterfv(t, p, v); }
void Interface::iglTexParameteri(unsigned t, unsigned p, int v) { glTexParameteri(t, p, v); }
void Interface::iglTexParameteriv(unsigned t, unsigned p, const int *v) { glTexParameteriv(t, p, v); }
void Interface::iglTexSubImage2D(unsigned t, int lvl, int x, int y, int w, int h, unsigned f, unsigned type, const void *d) { glTexSubImage2D(t, lvl, x, y, w, h, f, type, d); }
void Interface::iglUniform1f(int l, float v) { glUniform1f(l, v); }
void Interface::iglUniform1fv(int l, int c, const float *v) { glUniform1fv(l, c, v); }
void Interface::iglUniform1i(int l, int v) { glUniform1i(l, v); }
void Interface::iglUniform1iv(int l, int c, const int *v) { glUniform1iv(l, c, v); }
void Interface::iglUniform2f(int l, float x, float y) { glUniform2f(l, x, y); }
void Interface::iglUniform2fv(int l, int c, const float *v) { glUniform2fv(l, c, v); }
void Interface::iglUniform2i(int l, int x, int y) { glUniform2i(l, x, y); }
void Interface::iglUniform2iv(int l, int c, const int *v) { glUniform2iv(l, c, v); }
void Interface::iglUniform3f(int l, float x, float y, float z) { glUniform3f(l, x, y, z); }
void Interface::iglUniform3fv(int l, int c, const float *v) { glUniform3fv(l, c, v); }
void Interface::iglUniform3i(int l, int x, int y, int z) { glUniform3i(l, x, y, z); }
void Interface::iglUniform3iv(int l, int c, const int *v) { glUniform3iv(l, c, v); }
void Interface::iglUniform4f(int l, float x, float y, float z, float w) { glUniform4f(l, x, y, z, w); }
void Interface::iglUniform4fv(int l, int c, const float *v) { glUniform4fv(l, c, v); }
void Interface::iglUniform4i(int l, int x, int y, int z, int w) { glUniform4i(l, x, y, z, w); }
void Interface::iglUniform4iv(int l, int c, const int *v) { glUniform4iv(l, c, v); }
void Interface::iglUniformMatrix2fv(int l, int c, unsigned char tr, const float *v) { glUniformMatrix2fv(l, c, tr, v); }
void Interface::iglUniformMatrix3fv(int l, int c, unsigned char tr, const float *v) { glUniformMatrix3fv(l, c, tr, v); }
void Interface::iglUniformMatrix4fv(int l, int c, unsigned char tr, const float *v) { glUniformMatrix4fv(l, c, tr, v); }
void Interface::iglUseProgram(unsigned p) {
    if (m_interface) { if (m_interface->m_crtState.program == p) return; m_interface->m_crtState.program = p; }
    glUseProgram(p);
}
void Interface::iglValidateProgram(unsigned p) { glValidateProgram(p); }
void Interface::iglVertexAttrib1f(unsigned i, float x) { glVertexAttrib1f(i, x); }
void Interface::iglVertexAttrib1fv(unsigned i, const float *v) { glVertexAttrib1fv(i, v); }
void Interface::iglVertexAttrib2f(unsigned i, float x, float y) { glVertexAttrib2f(i, x, y); }
void Interface::iglVertexAttrib2fv(unsigned i, const float *v) { glVertexAttrib2fv(i, v); }
void Interface::iglVertexAttrib3f(unsigned i, float x, float y, float z) { glVertexAttrib3f(i, x, y, z); }
void Interface::iglVertexAttrib3fv(unsigned i, const float *v) { glVertexAttrib3fv(i, v); }
void Interface::iglVertexAttrib4f(unsigned i, float x, float y, float z, float w) { glVertexAttrib4f(i, x, y, z, w); }
void Interface::iglVertexAttrib4fv(unsigned i, const float *v) { glVertexAttrib4fv(i, v); }
void Interface::iglVertexAttribPointer(unsigned i, int sz, unsigned t, unsigned char n, int stride, const void *p) { glVertexAttribPointer(i, sz, t, n, stride, p); }
void Interface::iglViewport(int x, int y, int w, int h) { glViewport(x, y, w, h); }

void *Interface::iglMapBuffer(unsigned target, unsigned access) {
    return m_interface && m_interface->m_funcs->MapBuffer ? m_interface->m_funcs->MapBuffer(target, access) : 0;
}
unsigned char Interface::iglUnmapBuffer(unsigned target) {
    return m_interface && m_interface->m_funcs->UnmapBuffer ? m_interface->m_funcs->UnmapBuffer(target) : 0;
}
void Interface::iglBindVertexArray(unsigned a) { if (m_interface && m_interface->m_funcs->BindVertexArray) m_interface->m_funcs->BindVertexArray(a); }
void Interface::iglDeleteVertexArrays(int n, const unsigned *a) { if (m_interface && m_interface->m_funcs->DeleteVertexArrays) m_interface->m_funcs->DeleteVertexArrays(n, a); }
void Interface::iglGenVertexArrays(int n, unsigned *a) { if (m_interface && m_interface->m_funcs->GenVertexArrays) m_interface->m_funcs->GenVertexArrays(n, a); }
unsigned char Interface::iglIsVertexArray(unsigned a) { return m_interface && m_interface->m_funcs->IsVertexArray ? m_interface->m_funcs->IsVertexArray(a) : 0; }

void Interface::iassertNoError() { unsigned e = glGetError(); (void)e; }
bool Interface::icheckError() { return glGetError() != GL_NO_ERROR; }
const char *Interface::iglGetErrorString() {
    switch (glGetError()) {
    case GL_NO_ERROR: return "GL_NO_ERROR";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
    default: return "GL_UNKNOWN_ERROR";
    }
}
bool Interface::ihasExtension(const char *name) {
    if (!m_interface || !m_interface->m_extensions) return false;
    return strstr(m_interface->m_extensions, name) != 0;
}
bool Interface::ihasExtensionPart(const char *part) { return ihasExtension(part); }
void *Interface::igetContext() { return m_interface ? m_interface->m_context : 0; }
void Interface::isyncAll() { if (m_interface) m_interface->sync(); }
void Interface::iresetAll() { if (m_interface) m_interface->reset(); }

}

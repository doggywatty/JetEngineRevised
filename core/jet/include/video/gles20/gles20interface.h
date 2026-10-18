// core/jet/include/video/gles20/gles20interface.h
#ifndef CORE_JET_INCLUDE_VIDEO_GLES20_GLES20INTERFACE_H
#define CORE_JET_INCLUDE_VIDEO_GLES20_GLES20INTERFACE_H

#include <cstddef>

namespace jet::video::gles {

// GLES client version (2 or 3) of the context the EGL display created. Set by JetEGLDisplay::Init; stays 2
// on the desktop GL 2.1 path (macOS), which never sets it. It selects the shader dialect (GLSL ES 1.00 or
// 3.00) and whether the GLES 3.0 core entry points are loaded.
unsigned ClientVersion();
void SetClientVersion(unsigned version);

// Function pointers for GL calls not guaranteed to be linkable core symbols on every platform
// (VAO and multisample-blit are GL_OES_* extensions under GLES2; desktop GL needs them loaded
// too). Loaded once per thread by TLInterface::internalLoad via eglGetProcAddress/dlsym/wglGetProcAddress.
struct GLExtFunctions {
    void (*BindVertexArray)(unsigned);
    void (*DeleteVertexArrays)(int, const unsigned*);
    void (*GenVertexArrays)(int, unsigned*);
    unsigned char (*IsVertexArray)(unsigned);
    void (*RenderbufferStorageMultisample)(unsigned, int, unsigned, int, int);
    void (*BlitFramebuffer)(int, int, int, int, int, int, int, int, unsigned, unsigned);
    void* (*MapBuffer)(unsigned, unsigned);
    unsigned char (*UnmapBuffer)(unsigned);
};

// Cached subset of GL binding state, to skip redundant glBind*/glUseProgram calls; mirrors the
// TLInterface::force*Rebinding() API (buffer/framebuffer/renderbuffer/program/texture).
struct iGLState {
    iGLState() { reset(); }
    void reset() {
        arrayBuffer = elementBuffer = framebuffer = renderbuffer = program = 0;
        activeTexture = 0;
        for (int i = 0; i < 8; i++) boundTexture[i] = 0;
    }
    unsigned arrayBuffer;
    unsigned elementBuffer;
    unsigned framebuffer;
    unsigned renderbuffer;
    unsigned program;
    unsigned activeTexture;
    unsigned boundTexture[8];
};

class TLInterface {
public:
    explicit TLInterface(unsigned threadIdx);
    ~TLInterface();
    unsigned getThreadIdx();
    void* getContext();
    const char *getExtensions();
    void sync();
    void reset();
    void flush();
    bool internalLoad(void* context);
    void forceArrayBufferRebinding(unsigned);
    void forceElementBufferRebinding(unsigned);
    void forceFramebufferRebinding(unsigned);
    void forceRenderbufferRebinding(unsigned);
    void forceProgramRebinding(unsigned);
    void forceTextureRebinding();
    static void removeBuffer(unsigned);
    static void removeFramebuffer(unsigned);
    static void removeProgram(unsigned);
    static void removeRenderbuffer(unsigned);
    static void removeTexture(unsigned);

    GLExtFunctions *m_funcs;
    iGLState m_crtState;
    iGLState m_oldState;
    unsigned m_threadIdx;
    void* m_context;
    char* m_extensions;
};

// Thin per-call wrapper: each iglXxx forwards to the real GL call, going through the thread's
// TLInterface for the handful of calls (bind buffer/texture/framebuffer/renderbuffer/program)
// that are worth deduplicating. All ~150 entry points are real GL calls, not stubs.
class Interface {
public:
    Interface();
    TLInterface *GetInterface();

    void iglActiveTexture(unsigned);
    void iglAttachShader(unsigned, unsigned);
    void iglBindAttribLocation(unsigned, unsigned, const char*);
    void iglBindBuffer(unsigned, unsigned);
    void iglBindFramebuffer(unsigned, unsigned);
    void iglBindRenderbuffer(unsigned, unsigned);
    void iglBindTexture(unsigned, unsigned);
    void iglBlendColor(float, float, float, float);
    void iglBlendEquation(unsigned);
    void iglBlendEquationSeparate(unsigned, unsigned);
    void iglBlendFunc(unsigned, unsigned);
    void iglBlendFuncSeparate(unsigned, unsigned, unsigned, unsigned);
    void iglBufferData(unsigned, int, const void*, unsigned);
    void iglBufferSubData(unsigned, int, int, const void*);
    unsigned iglCheckFramebufferStatus(unsigned);
    void iglClear(unsigned);
    void iglClearColor(float, float, float, float);
    void iglClearDepthf(float);
    void iglClearStencil(int);
    void iglColorMask(unsigned char, unsigned char, unsigned char, unsigned char);
    void iglCompileShader(unsigned);
    void iglCompressedTexImage2D(unsigned, int, unsigned, int, int, int, int, const void*);
    void iglCompressedTexSubImage2D(unsigned, int, int, int, int, int, unsigned, int, const void*);
    void iglCopyTexImage2D(unsigned, int, unsigned, int, int, int, int, int);
    void iglCopyTexSubImage2D(unsigned, int, int, int, int, int, int, int);
    unsigned iglCreateProgram();
    unsigned iglCreateShader(unsigned);
    void iglCullFace(unsigned);
    void iglDeleteBuffers(int, const unsigned*);
    void iglDeleteFramebuffers(int, const unsigned*);
    void iglDeleteProgram(unsigned);
    void iglDeleteRenderbuffers(int, const unsigned*);
    void iglDeleteShader(unsigned);
    void iglDeleteTextures(int, const unsigned*);
    void iglDepthFunc(unsigned);
    void iglDepthMask(unsigned char);
    void iglDepthRangef(float, float);
    void iglDetachShader(unsigned, unsigned);
    void iglDisable(unsigned);
    void iglDisableVertexAttribArray(unsigned);
    void iglDrawArrays(unsigned, int, int);
    void iglDrawElements(unsigned, int, unsigned, const void*);
    void iglEnable(unsigned);
    void iglEnableVertexAttribArray(unsigned);
    void iglFinish();
    void iglFlush();
    void iglFramebufferRenderbuffer(unsigned, unsigned, unsigned, unsigned);
    void iglFramebufferTexture2D(unsigned, unsigned, unsigned, unsigned, int);
    void iglFrontFace(unsigned);
    void iglGenBuffers(int, unsigned*);
    void iglGenerateMipmap(unsigned);
    void iglGenFramebuffers(int, unsigned*);
    void iglGenRenderbuffers(int, unsigned*);
    void iglGenTextures(int, unsigned*);
    void iglGetActiveAttrib(unsigned, unsigned, int, int*, int*, unsigned*, char*);
    void iglGetActiveUniform(unsigned, unsigned, int, int*, int*, unsigned*, char*);
    void iglGetAttachedShaders(unsigned, int, int*, unsigned*);
    int iglGetAttribLocation(unsigned, const char*);
    void iglGetBooleanv(unsigned, unsigned char*);
    void iglGetBufferParameteriv(unsigned, unsigned, int*);
    unsigned iglGetError();
    void iglGetFloatv(unsigned, float*);
    void iglGetFramebufferAttachmentParameteriv(unsigned, unsigned, unsigned, int*);
    void iglGetIntegerv(unsigned, int*);
    void iglGetProgramiv(unsigned, unsigned, int*);
    void iglGetProgramInfoLog(unsigned, int, int*, char*);
    void iglGetRenderbufferParameteriv(unsigned, unsigned, int*);
    void iglGetShaderiv(unsigned, unsigned, int*);
    void iglGetShaderInfoLog(unsigned, int, int*, char*);
    void iglGetShaderPrecisionFormat(unsigned, unsigned, int*, int*);
    void iglGetShaderSource(unsigned, int, int*, char*);
    const unsigned char *iglGetString(unsigned);
    void iglGetTexParameterfv(unsigned, unsigned, float*);
    void iglGetTexParameteriv(unsigned, unsigned, int*);
    void iglGetUniformfv(unsigned, int, float*);
    void iglGetUniformiv(unsigned, int, int*);
    int iglGetUniformLocation(unsigned, const char*);
    void iglGetVertexAttribfv(unsigned, unsigned, float*);
    void iglGetVertexAttribiv(unsigned, unsigned, int*);
    void iglGetVertexAttribPointerv(unsigned, unsigned, void**);
    void iglHint(unsigned, unsigned);
    unsigned char iglIsBuffer(unsigned);
    unsigned char iglIsEnabled(unsigned);
    unsigned char iglIsFramebuffer(unsigned);
    unsigned char iglIsProgram(unsigned);
    unsigned char iglIsRenderbuffer(unsigned);
    unsigned char iglIsShader(unsigned);
    unsigned char iglIsTexture(unsigned);
    void iglLineWidth(float);
    void iglLinkProgram(unsigned);
    void iglPixelStorei(unsigned, int);
    void iglPolygonOffset(float, float);
    void iglReadPixels(int, int, int, int, unsigned, unsigned, void*);
    void iglReleaseShaderCompiler();
    void iglRenderbufferStorage(unsigned, unsigned, int, int);
    void iglRenderbufferStorageMultisample(unsigned, int, unsigned, int, int);
    void iglBlitFramebuffer(int, int, int, int, int, int, int, int, unsigned, unsigned);
    void iglSampleCoverage(float, unsigned char);
    void iglScissor(int, int, int, int);
    void iglShaderBinary(int, const unsigned*, unsigned, const void*, int);
    void iglShaderSource(unsigned, int, const char**, const int*);
    void iglStencilFunc(unsigned, int, unsigned);
    void iglStencilFuncSeparate(unsigned, unsigned, int, unsigned);
    void iglStencilMask(unsigned);
    void iglStencilMaskSeparate(unsigned, unsigned);
    void iglStencilOp(unsigned, unsigned, unsigned);
    void iglStencilOpSeparate(unsigned, unsigned, unsigned, unsigned);
    void iglTexImage2D(unsigned, int, int, int, int, int, unsigned, unsigned, const void*);
    void iglTexParameterf(unsigned, unsigned, float);
    void iglTexParameterfv(unsigned, unsigned, const float*);
    void iglTexParameteri(unsigned, unsigned, int);
    void iglTexParameteriv(unsigned, unsigned, const int*);
    void iglTexSubImage2D(unsigned, int, int, int, int, int, unsigned, unsigned, const void*);
    void iglUniform1f(int, float);
    void iglUniform1fv(int, int, const float*);
    void iglUniform1i(int, int);
    void iglUniform1iv(int, int, const int*);
    void iglUniform2f(int, float, float);
    void iglUniform2fv(int, int, const float*);
    void iglUniform2i(int, int, int);
    void iglUniform2iv(int, int, const int*);
    void iglUniform3f(int, float, float, float);
    void iglUniform3fv(int, int, const float*);
    void iglUniform3i(int, int, int, int);
    void iglUniform3iv(int, int, const int*);
    void iglUniform4f(int, float, float, float, float);
    void iglUniform4fv(int, int, const float*);
    void iglUniform4i(int, int, int, int, int);
    void iglUniform4iv(int, int, const int*);
    void iglUniformMatrix2fv(int, int, unsigned char, const float*);
    void iglUniformMatrix3fv(int, int, unsigned char, const float*);
    void iglUniformMatrix4fv(int, int, unsigned char, const float*);
    void iglUseProgram(unsigned);
    void iglValidateProgram(unsigned);
    void iglVertexAttrib1f(unsigned, float);
    void iglVertexAttrib1fv(unsigned, const float*);
    void iglVertexAttrib2f(unsigned, float, float);
    void iglVertexAttrib2fv(unsigned, const float*);
    void iglVertexAttrib3f(unsigned, float, float, float);
    void iglVertexAttrib3fv(unsigned, const float*);
    void iglVertexAttrib4f(unsigned, float, float, float, float);
    void iglVertexAttrib4fv(unsigned, const float*);
    void iglVertexAttribPointer(unsigned, int, unsigned, unsigned char, int, const void*);
    void iglViewport(int, int, int, int);
    void* iglMapBuffer(unsigned, unsigned);
    unsigned char iglUnmapBuffer(unsigned);
    void iglBindVertexArray(unsigned);
    void iglDeleteVertexArrays(int, const unsigned*);
    void iglGenVertexArrays(int, unsigned*);
    unsigned char iglIsVertexArray(unsigned);
    void iassertNoError();
    bool icheckError();
    const char *iglGetErrorString();
    bool ihasExtension(const char*);
    bool ihasExtensionPart(const char*);
    void* igetContext();
    void isyncAll();
    void iresetAll();

    TLInterface *m_interface;
};

bool initForThread(void* context);
void shutdownForThread();
void setContext(void* context);

}

#endif

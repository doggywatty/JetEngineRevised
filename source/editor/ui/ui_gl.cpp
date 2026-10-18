// source/editor/ui/ui_gl.cpp
#include "editor/ui/ui_gl.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#if defined(__APPLE__)
// macOS: the desktop GL 2.1 context of core/jet's MacDisplay (CGL), with GL's own header, as gles20interface.cpp includes it
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
typedef ptrdiff_t GLsizeiptr;  // desktop GL declares it as ptrdiff_t, as GLES2 does
#elif defined(__linux__)
#include <GLES2/gl2.h>
#endif

#if defined(__linux__) || defined(__APPLE__)

namespace jeteditor::ui {

namespace {

#if defined(__APPLE__)
// GLSL 1.20 on the GL 2.1 context: the same attributes and varyings, a version line, and no precision statements,
// which only GLSL ES has
const char *kVertexShader = R"(#version 120
attribute vec2 aPos;
attribute vec2 aUv;
attribute vec4 aColor;
uniform vec2 uSize;
varying vec2 vUv;
varying vec4 vColor;
void main() {
    vUv = aUv;
    vColor = aColor;
    gl_Position = vec4(aPos.x / uSize.x * 2.0 - 1.0, 1.0 - aPos.y / uSize.y * 2.0, 0.0, 1.0);
}
)";

// Text: the font atlas is alpha only, and the vertex colour gives the ink
const char *kFontFragmentShader = R"(#version 120
uniform sampler2D uTex;
varying vec2 vUv;
varying vec4 vColor;
void main() {
    gl_FragColor = vec4(vColor.rgb, vColor.a * texture2D(uTex, vUv).a);
}
)";

// Images: an RGBA texture of the game's frame, drawn as it is
const char *kImageFragmentShader = R"(#version 120
uniform sampler2D uTex;
varying vec2 vUv;
varying vec4 vColor;
void main() {
    gl_FragColor = vec4(texture2D(uTex, vUv).rgb, 1.0) * vec4(vColor.rgb, 1.0);
}
)";
#else
const char *kVertexShader = R"(
attribute vec2 aPos;
attribute vec2 aUv;
attribute vec4 aColor;
uniform vec2 uSize;
varying vec2 vUv;
varying vec4 vColor;
void main() {
    vUv = aUv;
    vColor = aColor;
    gl_Position = vec4(aPos.x / uSize.x * 2.0 - 1.0, 1.0 - aPos.y / uSize.y * 2.0, 0.0, 1.0);
}
)";

// Text: the font atlas is alpha only, and the vertex colour gives the ink
const char *kFontFragmentShader = R"(
precision mediump float;
uniform sampler2D uTex;
varying vec2 vUv;
varying vec4 vColor;
void main() {
    gl_FragColor = vec4(vColor.rgb, vColor.a * texture2D(uTex, vUv).a);
}
)";

// Images: an RGBA texture of the game's frame, drawn as it is
const char *kImageFragmentShader = R"(
precision mediump float;
uniform sampler2D uTex;
varying vec2 vUv;
varying vec4 vColor;
void main() {
    gl_FragColor = vec4(texture2D(uTex, vUv).rgb, 1.0) * vec4(vColor.rgb, 1.0);
}
)";
#endif

bool CompileStage(GLenum type, const char *source, unsigned &shader, std::string &error) {
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512] = {0};
        glGetShaderInfoLog(shader, sizeof log - 1, nullptr, log);
        error = std::string("shader does not compile: ") + log;
        glDeleteShader(shader);
        shader = 0;
        return false;
    }
    return true;
}

// Links the shared vertex shader with one fragment shader. The attribute locations are the same in every program.
bool LinkProgram(const char *fragmentSource, unsigned &program, std::string &error) {
    unsigned vs = 0, fs = 0;
    if (!CompileStage(GL_VERTEX_SHADER, kVertexShader, vs, error)) return false;
    if (!CompileStage(GL_FRAGMENT_SHADER, fragmentSource, fs, error)) {
        glDeleteShader(vs);
        return false;
    }
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glBindAttribLocation(program, 0, "aPos");
    glBindAttribLocation(program, 1, "aUv");
    glBindAttribLocation(program, 2, "aColor");
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512] = {0};
        glGetProgramInfoLog(program, sizeof log - 1, nullptr, log);
        error = std::string("shader program does not link: ") + log;
        glDeleteProgram(program);
        program = 0;
        return false;
    }
    return true;
}

}  // namespace

GlRenderer::~GlRenderer() { Shutdown(); }

bool GlRenderer::Init(const Font &font, std::string &error) {
    if (!LinkProgram(kFontFragmentShader, program_, error)) return false;
    if (!LinkProgram(kImageFragmentShader, imageProgram_, error)) {
        Shutdown();
        return false;
    }
    locPos_ = 0;
    locUv_ = 1;
    locColor_ = 2;
    locSize_ = glGetUniformLocation(program_, "uSize");
    locTex_ = glGetUniformLocation(program_, "uTex");
    imageLocSize_ = glGetUniformLocation(imageProgram_, "uSize");
    imageLocTex_ = glGetUniformLocation(imageProgram_, "uTex");

    const int n = Font::kAtlasSize;
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, n, n, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font.Atlas().data());

    glGenBuffers(1, &buffer_);
    if (glGetError() != GL_NO_ERROR) {
        error = "GL setup failed";
        Shutdown();
        return false;
    }
    return true;
}

void GlRenderer::Draw(const Context &ctx, int fbWidth, int fbHeight, Color clear) {
    // The editor's frame is drawn after the game's (the game leaves its own state behind), so every state the
    // editor relies on is set here. The game's last target is its offscreen frame, so the editor's own framebuffer is
    // bound first: the window's on Linux (0), the MacDisplay's render target on macOS (see SetFramebuffer).
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glViewport(0, 0, fbWidth, fbHeight);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(clear.r / 255.0f, clear.g / 255.0f, clear.b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (ctx.Vertices().empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    const std::vector<Vertex> &verts = ctx.Vertices();
    glBindBuffer(GL_ARRAY_BUFFER, buffer_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(verts.size() * sizeof(Vertex)), verts.data(),
                 GL_STREAM_DRAW);
    glEnableVertexAttribArray(locPos_);
    glEnableVertexAttribArray(locUv_);
    glEnableVertexAttribArray(locColor_);
    glVertexAttribPointer(locPos_, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, x)));
    glVertexAttribPointer(locUv_, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, u)));
    glVertexAttribPointer(locColor_, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, r)));

    // Scissor is in framebuffer pixels with the origin at the bottom-left; the UI's clip is top-left
    for (const Command &cmd : ctx.Commands()) {
        const Rect &c = cmd.clip;
        const int x0 = static_cast<int>(std::floor(c.x));
        const int y0 = static_cast<int>(std::floor(c.y));
        const int x1 = static_cast<int>(std::ceil(c.Right()));
        const int y1 = static_cast<int>(std::ceil(c.Bottom()));
        glScissor(x0, fbHeight - y1, x1 - x0 > 0 ? x1 - x0 : 0, y1 - y0 > 0 ? y1 - y0 : 0);
        if (cmd.texture) {
            glUseProgram(imageProgram_);
            glBindTexture(GL_TEXTURE_2D, cmd.texture);
            // The game's frame is drawn at its own size: its sampler state can ask for mip levels that do not exist,
            // which samples black, so the editor sets a complete state for every image it draws
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glUniform1i(imageLocTex_, 0);
            glUniform2f(imageLocSize_, static_cast<float>(fbWidth), static_cast<float>(fbHeight));
        } else {
            glUseProgram(program_);
            glBindTexture(GL_TEXTURE_2D, texture_);
            glUniform1i(locTex_, 0);
            glUniform2f(locSize_, static_cast<float>(fbWidth), static_cast<float>(fbHeight));
        }
        glDrawArrays(GL_TRIANGLES, static_cast<GLint>(cmd.first), static_cast<GLsizei>(cmd.count));
    }

    glDisableVertexAttribArray(locPos_);
    glDisableVertexAttribArray(locUv_);
    glDisableVertexAttribArray(locColor_);
    glDisable(GL_SCISSOR_TEST);
}

void GlRenderer::ReadBack(int w, int h, std::vector<uint8_t> &rgb) const {
    // GL rows run bottom-up; the PPM is written top row first
    std::vector<uint8_t> raw(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, raw.data());
    rgb.assign(static_cast<size_t>(w) * static_cast<size_t>(h) * 3, 0);
    for (int y = 0; y < h; ++y) {
        const uint8_t *src = raw.data() + static_cast<size_t>(h - 1 - y) * static_cast<size_t>(w) * 4;
        uint8_t *dst = rgb.data() + static_cast<size_t>(y) * static_cast<size_t>(w) * 3;
        for (int x = 0; x < w; ++x) {
            dst[3 * x] = src[4 * x];
            dst[3 * x + 1] = src[4 * x + 1];
            dst[3 * x + 2] = src[4 * x + 2];
        }
    }
}

void GlRenderer::Shutdown() {
    if (buffer_) glDeleteBuffers(1, &buffer_);
    if (texture_) glDeleteTextures(1, &texture_);
    if (program_) glDeleteProgram(program_);
    if (imageProgram_) glDeleteProgram(imageProgram_);
    buffer_ = texture_ = program_ = imageProgram_ = 0;
}

}  // namespace jeteditor::ui

#else  // neither Linux nor macOS: there is no GL path for this platform yet

namespace jeteditor::ui {

GlRenderer::~GlRenderer() = default;

bool GlRenderer::Init(const Font &, std::string &error) {
    error = "the editor's GL drawing is built for Linux and macOS only";
    return false;
}

void GlRenderer::Draw(const Context &, int, int, Color) {}

void GlRenderer::ReadBack(int, int, std::vector<uint8_t> &rgb) const { rgb.clear(); }

void GlRenderer::Shutdown() {}

}  // namespace jeteditor::ui

#endif

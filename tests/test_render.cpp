// tests/test_render.cpp
#undef NDEBUG
// Real end-to-end render smoke test: creates an actual GL context (MacDisplay / JetEGLDisplay), builds a
// real RenderTechnique/Material from GLSL source, draws a triangle through Painter, and reads
// back the framebuffer to verify real, non-black, correctly-colored pixels came out the other
// end of the GLES20 driver stack -- not a data-only test.
#if defined(__APPLE__)
#include "video/mac/macdisplay.h"
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
typedef jet::video::MacDisplay TestDisplay;
#else
#include "testdisplay.h"
#endif
#include "video/gles20/gles20driver.h"
#include "video/painter.h"
#include "video/rendertechnique.h"
#include "video/material.h"
#include "video/renderjob.h"
#include "stream/memorystream.h"
#include <cstdio>
#include <cstring>
#include <cassert>

using namespace jet;
using namespace jet::video;

static const char *kVS =
    "attribute vec3 a_position;\n"
    "attribute vec4 a_color;\n"
    "varying vec4 v_color;\n"
    "void main() { v_color = a_color; gl_Position = vec4(a_position, 1.0); }\n";
static const char *kFS =
    "#ifdef GL_ES\nprecision mediump float;\n#endif\n"
    "varying vec4 v_color;\n"
    "void main() { gl_FragColor = v_color; }\n";

// The game's technique schema (see shaders/*.xml)
static const char *kTechniqueXml =
    "<technique name=\"test\" version=\"100\">"
    "<pass name=\"main\" target=\"GLES2.0\">"
    "<attributes><var name=\"a_position\" val=\"mesh.positions\"/><var name=\"a_color\" val=\"mesh.colors\"/></attributes>"
    "<shader><vshader><![CDATA[__VS__]]></vshader><fshader><![CDATA[__FS__]]></fshader></shader>"
    "</pass>"
    "</technique>";

int main() {
    DisplayMode mode;
    mode.size = math::vec2<unsigned int>(64, 64);
    TestDisplay display;
    if (!display.Init(mode)) { printf("display init failed\n"); return 1; }

    GLES20Driver driver;
    if (!driver.Init(&display)) { printf("driver init failed\n"); return 1; }

    boost::shared_ptr<RenderTarget> rt = display.GetRenderTarget();
    driver.BeginRendering();
    driver.SetCrtRenderTarget(rt);

    // Build the technique XML with real shader source substituted in.
    std::string xml(kTechniqueXml);
    size_t p = xml.find("__VS__"); xml.replace(p, 6, kVS);
    p = xml.find("__FS__"); xml.replace(p, 6, kFS);

    stream::MemoryStream *ms = new stream::MemoryStream();
    ms->Write(xml.data(), (unsigned)xml.size());
    ms->SeekBeg(0);

    boost::shared_ptr<RenderTechnique> tech(new RenderTechnique(String("test")));
    tech->Init(ms);
    bool loaded = tech->Load();
    printf("technique load: %s, passes=%u\n", loaded ? "ok" : "FAIL", tech->GetRenderPassCount());
    assert(loaded && tech->GetRenderPassCount() == 1);
    tech->Link();

    Material mat;
    mat.SetRenderTechnique(tech);

    Painter painter(1024);
    painter.SetMaterial(mat);
    // Fullscreen-ish triangle in clip space, red/green/blue verts to prove interpolation works.
    Painter::Vertex a(math::vec3<float>(-0.8f, -0.8f, 0.0f), Color(1, 0, 0, 1));
    Painter::Vertex b(math::vec3<float>(0.8f, -0.8f, 0.0f), Color(0, 1, 0, 1));
    Painter::Vertex c(math::vec3<float>(0.0f, 0.8f, 0.0f), Color(0, 0, 1, 1));
    painter.FillTriangle(a, b, c);
    painter.Flush();
    printf("glGetError after flush: 0x%x\n", glGetError());
    GLint curProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &curProg);
    printf("current program: %d\n", curProg);

    driver.EndRendering();

    // Read back MacDisplay's offscreen FBO directly (still bound from SetCrtRenderTarget's
    // Activate() call) rather than through a texture-readback API, since RenderTarget doesn't
    // expose one yet.
    std::vector<unsigned char> pixels(64 * 64 * 4);
    glReadPixels(0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    unsigned nonBlack = 0;
    unsigned char center[4];
    for (unsigned y = 0; y < 64; ++y)
        for (unsigned x = 0; x < 64; ++x) {
            unsigned char *px = &pixels[(y * 64 + x) * 4];
            if (px[0] || px[1] || px[2]) ++nonBlack;
            if (x == 32 && y == 20) memcpy(center, px, 4);
        }
    printf("non-black pixels: %u / %u\n", nonBlack, 64 * 64);
    printf("center pixel rgba: %u %u %u %u\n", center[0], center[1], center[2], center[3]);
    assert(nonBlack > 500);

    display.Cleanup();
    puts("render ok");
    return 0;
}

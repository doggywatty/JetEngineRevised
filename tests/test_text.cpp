// tests/test_text.cpp
#undef NDEBUG
// TrueType text: FontMgr draws strings through text2::Texter and the _font_ttf_screen_coords
// technique, written as a PPM:
//   test_text <data dir> [out.ppm]
#include "testdisplay.h"
#include "video/gles20/gles20driver.h"
#include "video/painter.h"
#include "stream/streammgr.h"
#include "text/fontloader.h"
#include "sprites/fontmgr.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

using namespace jet;
typedef text2::Texter Texter;

int main(int argc, char **argv) {
    std::string data = argc > 1 ? argv[1] : ".";
    std::string out = argc > 2 ? argv[2] : "text.ppm";
    if (FILE *f = fopen((data + "/fonts_ttf/gothambold.ttf").c_str(), "rb")) fclose(f);
    else { printf("no fonts under %s, skipped\n", data.c_str()); return 0; }
    const unsigned W = 960, H = 540;
    video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(W, H);
    TestDisplay display;
    assert(display.Init(mode));
    video::GLES20Driver driver;
    assert(driver.Init(&display));
    stream::StreamMgr &sm = stream::StreamMgr::GetInstance();
    for (const char *d : { "shaders", "fonts_ttf" }) sm.AddPath(String((data + "/" + d).c_str()), String());

    printf("display ok\n"); fflush(stdout);
    text::FontLoader loader;
    FontMgr fonts;
    video::Painter painter(4096);
    printf("painter ok\n"); fflush(stdout);
    FontMgr::Set2DRenderingWorldCoords(false);
    printf("coords ok\n"); fflush(stdout);
    fonts.AddFreeTypeFont(String("gotham"), String("gothambold.ttf"), 48, painter);
    fonts.AddFreeTypeFont(String("small"), String("despicableme.ttf"), 24, painter);
    printf("fonts added\n"); fflush(stdout);
    assert(fonts.GetLoadedFontsCount() == 2);

    driver.BeginRendering();
    display.GetRenderTarget()->SetClearColor(math::vec4<float>(0.1f, 0.2f, 0.4f, 1));
    driver.SetCrtRenderTarget(display.GetRenderTarget());
    glViewport(0, 0, W, H);
    glClearColor(0.1f, 0.2f, 0.4f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Texter::Style style;
    math::rect<float> noClip(0, 0, 0, 0);
    math::mat4<float> id;
    typedef safe_enum<Texter::AlignmentDef, Texter::AlignmentDef::type> Align;
    fonts.DrawString("MINION RUSH", 40, 40, Align(Texter::AlignmentDef::TOP_LEFT), String("gotham"), style, 1.0f, -1, false, noClip, id);
    style.decoration = Texter::DecorationDef::SHADOW;
    style.decorationThickness = 2;
    style.primaryColor = 0xffffd800;
    fonts.DrawString("Tap to start!\nsecond line", 40, 140, Align(Texter::AlignmentDef::TOP_LEFT), String("gotham"), style, 1.0f, -1, false, noClip, id);
    Texter::Style grad;
    grad.colorMode = Texter::ColorModeDef::GRADIENT_TB;
    grad.primaryColor = 0xffff0000;
    grad.gradientColor = 0xff00ff00;
    fonts.DrawString("gradient #0000ffblue tag", 480, 300, Align(Texter::AlignmentDef::CENTER), String("small"), grad, 2.0f, -1, false, noClip, id);
    fonts.DrawString("wrapped text wraps at the given maximum width", 40, 400, Align(Texter::AlignmentDef::TOP_LEFT), String("small"), Texter::Style(), 1.0f, 300, false, noClip, id);
    math::vec2<float> sz = fonts.GetTextSize("MINION RUSH", String("gotham"), -1, Texter::Style());
    printf("size of MINION RUSH: %g x %g\n", sz.x, sz.y);
    assert(sz.x > 100 && sz.y > 30);
    painter.Flush();
    driver.EndRendering();
    printf("glGetError 0x%x\n", glGetError());

    std::vector<unsigned char> px(W * H * 4);
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    unsigned lit = 0;
    for (unsigned i = 0; i < W * H; i++)
        if (px[i * 4] > 200 && px[i * 4 + 1] > 150) lit++;
    printf("%u yellow-ish pixels\n", lit);
    assert(lit > 500);
    FILE *f = fopen(out.c_str(), "wb");
    fprintf(f, "P6\n%u %u\n255\n", W, H);
    for (int y = (int)H - 1; y >= 0; y--)
        for (unsigned x = 0; x < W; x++) fwrite(&px[(y * W + x) * 4], 1, 3, f);
    fclose(f);
    printf("wrote %s\n", out.c_str());
    display.Cleanup();
    return 0;
}

// tests/test_sprites.cpp
#undef NDEBUG
// Aurora sprites: loads <name>.bsprite from the "sprites" datalib with its atlas and paints every
// frame in a grid through the Painter and the sprite2d technique, written as a PPM:
//   test_sprites <data dir> [sprite] [out.ppm]
#include "testdisplay.h"
#include "video/gles20/gles20driver.h"
#include "video/painter.h"
#include "stream/streammgr.h"
#include "sprites/sprite.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

using namespace jet;

int main(int argc, char **argv) {
    std::string data = argc > 1 ? argv[1] : ".";
    std::string name = argc > 2 ? argv[2] : "menu_01";
    std::string out = argc > 3 ? argv[3] : "sprites.ppm";
    if (FILE *f = fopen((data + "/datalibs/sprites").c_str(), "rb")) fclose(f);
    else { printf("no sprites datalib under %s, skipped\n", data.c_str()); return 0; }
    const unsigned W = 1024, H = 1024;
    video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(W, H);
    TestDisplay display;
    assert(display.Init(mode));
    video::GLES20Driver driver;
    assert(driver.Init(&display));
    driver.SetSeparateAlphaTextures(true);
    stream::StreamMgr &sm = stream::StreamMgr::GetInstance();
    for (const char *d : { "shaders", "textures_android", "datalibs/sprites" })
        sm.AddPath(String((data + "/" + d).c_str()), String());

    Sprite *sp = Sprite::LoadSprite(String(name.c_str()), false);
    assert(sp);
    printf("%s: %d modules, %d frames, %d anims, image '%s', texture %s\n", name.c_str(), sp->GetNumberOfModules(),
           sp->GetNumberOfFrames(), sp->GetNumberOfAnims(), sp->GetImageName().c_str(), sp->GetTexture(0) ? "ok" : "MISSING");

    driver.BeginRendering();
    display.GetRenderTarget()->SetClearColor(math::vec4<float>(0.2f, 0.2f, 0.25f, 1));
    driver.SetCrtRenderTarget(display.GetRenderTarget());
    glViewport(0, 0, W, H);
    glClearColor(0.2f, 0.2f, 0.25f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    video::Painter painter(4096);
    // frames packed in rows by their bounds, scaled to fit a 128 px cell
    const float cell = 128;
    int col = 0, row = 0;
    // MODULE=n paints one module, FRAME=n one frame, at (512, 512)
    if (getenv("MODULE") || getenv("FRAME")) {
        if (const char *mod = getenv("MODULE")) sp->PaintModule(painter, (unsigned)atoi(mod), math::vec2<float>(0, 0));
        else {
            math::mat3<float> m;
            m.m[6] = m.m[7] = 512;
            sp->PaintFrame(painter, (unsigned)atoi(getenv("FRAME")), m);
        }
        painter.Flush();
        driver.EndRendering();
        std::vector<unsigned char> px(W * H * 4);
        glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
        FILE *o = fopen(out.c_str(), "wb");
        fprintf(o, "P6\n%u %u\n255\n", W, H);
        for (int y = (int)H - 1; y >= 0; y--)
            for (unsigned x = 0; x < W; x++) fwrite(&px[(y * W + x) * 4], 1, 3, o);
        fclose(o);
        return 0;
    }
    for (int f = 0; f < sp->GetNumberOfFrames(); f++) {
        math::rect<float> r;
        sp->GetFrameRect(r, (unsigned)f, math::mat3<float>());
        float w = r.getWidth(), h = r.getHeight();
        if (w <= 0 || h <= 0) continue;
        float s = std::min(1.0f, std::min(cell / w, cell / h));
        math::mat3<float> m;
        m.m[0] = m.m[4] = s;
        m.m[6] = col * cell - r.upperLeftCorner.x * s;
        m.m[7] = row * cell - r.upperLeftCorner.y * s;
        sp->PaintFrame(painter, (unsigned)f, m);
        if (++col == (int)(W / cell)) { col = 0; if (++row == (int)(H / cell)) break; }
    }
    painter.Flush();
    driver.EndRendering();
    printf("glGetError 0x%x\n", glGetError());

    std::vector<unsigned char> px(W * H * 4);
    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    FILE *f = fopen(out.c_str(), "wb");
    fprintf(f, "P6\n%u %u\n255\n", W, H);
    for (int y = (int)H - 1; y >= 0; y--)
        for (unsigned x = 0; x < W; x++) fwrite(&px[(y * W + x) * 4], 1, 3, f);
    fclose(f);
    printf("wrote %s\n", out.c_str());
    delete sp;
    display.Cleanup();
    return 0;
}

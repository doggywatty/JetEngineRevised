// tests/test_menu.cpp
#undef NDEBUG
// Main-menu scene: the game's clara libraries (designlib + lablib) loaded with the game entity
// classes, the Background_LaboStart library's Deco3d rendered from MenuUI_DefaultCamera with the
// background's fog, written as a PPM:
//   test_menu <data dir> [out.ppm] [width height]
#include "testdisplay.h"
#include "video/gles20/gles20driver.h"
#include "scene/camera.h"
#include "stream/streammgr.h"
#include "stream/filestream.h"
#include "project.h"
#include "folder.h"
#include "game/entities/GameEntityCreator.h"
#include "game/entities/Deco3d.h"
#include "game/entities/MaterialDef.h"
#include "game/entities/LightDef.h"
#include "game/entities/TouchInteract3d.h"
#include "game/entities/Minion.h"
#include "scene/model.h"
#include "game/actors/statemachine/StateDatabase.h"
#include "video/posteffect.h"
#include "scene/modelbase.h"
#include "game/CGameConfig.h"
#include "game/GameLevel.h"
#include "luavm.h"
#include "scene/scenemgr.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace jet;

static void CollectDecos(const clara::Folder &f, std::vector<Deco3d*> &out) {
    for (unsigned i = 0; i < f.GetEntityCount(); i++)
        if (Deco3d *d = dynamic_cast<Deco3d*>(f.GetEntity(i))) out.push_back(d);
    for (unsigned i = 0; i < f.GetFolderCount(); i++) CollectDecos(f.GetFolder(i), out);
}

static bool AddPack(clara::Project &p, const clara::Creator &c, const std::string &path) {
    if (FILE *f = fopen(path.c_str(), "rb")) fclose(f);
    else return false;
    stream::FileStream s(String(path.c_str()), stream::FileStream::k_in, false);
    p.AddLibraries(s, c);
    return true;
}

// sink is given every finished frame (RGBA, bottom row first); without one a single PPM is written
int MenuMain(int argc, char **argv, bool (*sink)(const unsigned char*, unsigned, unsigned, void*), void *ud) {
    std::string data = argc > 1 ? argv[1] : ".";
    std::string out = argc > 2 ? argv[2] : "menu.ppm";
    const unsigned W = argc > 4 ? atoi(argv[3]) : 960, H = argc > 4 ? atoi(argv[4]) : 540;

    video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(W, H);
    TestDisplay display;
    assert(display.Init(mode));
    video::GLES20Driver driver;
    assert(driver.Init(&display));
    driver.SetCrtRenderTarget(display.GetRenderTarget());
    stream::StreamMgr &sm = stream::StreamMgr::GetInstance();
    for (const char *d : { "models", "shaders", "textures_android", "levels", "anims", "datalibs" })
        sm.AddPath(String((data + "/" + d).c_str()), String());

    // Game::PostInit: the device profile's shader defines and the hardware skinning palette size
    PlatformToggles toggles;
    {
        std::ifstream f(data + "/profiles.json", std::ios::binary);
        std::stringstream ss;
        ss << f.rdbuf();
        const char *device = getenv("JET_DEVICE") ? getenv("JET_DEVICE") : "GT-N7000";
        if (CGameConfig::ComputePlatformToggles(ss.str(), device, toggles)) printf("profile for %s\n", device);
    }
    CGameConfig::AddShaderDefines(toggles, &driver);
    scene::ModelBase::s_hwSkinningBoneLimit = 30;
    clara::Project project;
    GameEntityCreator creator;
    if (!AddPack(project, creator, data + "/levels/designlib.blibclara") || !AddPack(project, creator, data + "/levels/lablib.blibclara")) {
        printf("clara libraries not found under %s/levels, skipped\n", data.c_str());
        display.Cleanup();
        return 0;
    }
    GameLevel gameLevel(false);
    gameLevel.SetLuaVM(new LuaVM);
    // the level itself (LevelDef, the player Minion, ...)
    {
        stream::FileStream lvl(String((data + "/levels/level.bclara").c_str()), stream::FileStream::k_in, false);
        lvl.Start();
        project.LoadClaraFile(lvl, creator);
        lvl.Finish();
    }
    printf("states: %s\n", StateDatabase::GetInstance()->Load(String("states")) ? "ok" : "FAILED");
    project.Init();
    project.PostInit();
    Minion *minion = dynamic_cast<Minion*>(project.FindEntityByName(String("Minion")));
    if (minion)
        if (const clara::Param *p = minion->FindParamByName(String("MainMenuStateMachine")))
            minion->SetStateMachine(p->GetAsState(0).a, p->GetAsState(0).b);
    printf("minion state: %s\n", minion && minion->SM_GetState() ? minion->SM_GetState()->GetName().c_str() : "none");

    if (getenv("JET_DBG") && minion && minion->GetDeco() && minion->GetDeco()->GetModel()) {
        scene::Model *mm = minion->GetDeco()->GetModel();
        for (unsigned i = 0; i < mm->GetMaterialCount(); i++) {
            const video::Material &m = mm->GetMaterial(i);
            printf("mat %s passes=%u", mm->GetMaterialName(i).c_str(), m.GetRenderPassCount());
            if (m.GetRenderPassCount()) {
                const video::RenderState &rs = m.GetRenderState(0);
                printf(" tech=%s dt=%d dw=%d df=%d blend=%d cull=%d", m.GetRenderTechnique()->GetName().c_str(), rs.GetDepthTest(), rs.GetDepthWrite(), (int)(unsigned char)rs.GetDepthFunc(), rs.GetBlending(), rs.GetCulling());
                for (unsigned l = 0; l < m.GetTextureLayerCount(0); l++)
                    printf(" [%s=%s]", m.GetTextureLayerName(0, l).c_str(), m.GetTextureLayer(0, l).GetTexture() ? "tex" : "-");
            }
            printf("\n");
        }
    }
    std::vector<Deco3d*> decos;
    clara::Entity *camEntity = 0, *background = 0;
    LightDef *menuLight = 0;
    std::vector<TouchInteract3d*> menuButtons;
    for (unsigned i = 0; i < project.GetLibraryCount(); i++) {
        clara::ClaraFile *lib = project.GetLibrary(i);
        if (lib->GetName().Equals("Background_LaboStart")) {
            CollectDecos(lib->GetRoot(), decos);
            background = dynamic_cast<clara::Entity*>(lib->GetRoot().FindEntityByName(String("BackgroundData_LaboStart")));
        }
        if (lib->GetName().Equals("MenuUI")) {
            camEntity = dynamic_cast<clara::Entity*>(lib->GetRoot().FindEntityByName(String("MenuUI_DefaultCamera")));
            menuLight = dynamic_cast<LightDef*>(lib->GetRoot().FindEntityByName(String("MenuUI_DefaultLight")));
            if (const clara::Folder *mm = lib->GetRoot().FindFolderByName(String("MainMenu")))
                for (unsigned e = 0; e < mm->GetEntityCount(); e++)
                    if (TouchInteract3d *t = dynamic_cast<TouchInteract3d*>(mm->GetEntity(e))) menuButtons.push_back(t);
        }
    }
    printf("Background_LaboStart: %zu decos\n", decos.size());
    assert(camEntity && background);
    unsigned withModel = 0;
    for (Deco3d *d : decos) if (d->GetModel()) ++withModel;
    printf("%u with models\n", withModel);

    // CameraDirect: the entity transform and its Fov (degrees)
    float fov = 50;
    camEntity->GetParam(String("Fov"), fov);
    scene::Camera cam(camEntity->GetPosition(), camEntity->GetRotation());
    if (const char *c = getenv("JET_CAM")) {
        // debug view: "x y z yaw pitch" (degrees; yaw 0 looks along -Y, 90 along +X)
        float x, y, z, yaw, pitch;
        if (sscanf(c, "%f %f %f %f %f", &x, &y, &z, &yaw, &pitch) == 5) {
            yaw *= 0.017453292f; pitch *= 0.017453292f;
            math::quat<float> qz(0, 0, sinf(yaw * 0.5f), cosf(yaw * 0.5f)), qx(sinf(pitch * 0.5f), 0, 0, cosf(pitch * 0.5f));
            cam.SetTransform(math::vec3<float>(x, y, z), qz * qx);
        }
    }
    cam.SetViewport(math::rect<unsigned int>(0, 0, W, H));
    printf("camera (%g %g %g) front (%g %g %g)\n", cam.GetPosition().x, cam.GetPosition().y, cam.GetPosition().z,
           cam.GetFrontVector().x, cam.GetFrontVector().y, cam.GetFrontVector().z);
    cam.SetFov(fov * 0.017453292f);
    cam.SetNearDistance(0.1f);
    cam.SetFarDistance(500.0f);

    // BackgroundData fog
    float fogStart = 0, fogEnd = 0;
    math::vec3<float> fogColor(0, 0, 0);
    bool fogOn = false;
    background->GetParam(String("FogEnabled"), fogOn);
    background->GetParam(String("FogStartDepth"), fogStart);
    background->GetParam(String("FogEndDepth"), fogEnd);
    math::vec4<float> fc(0, 0, 0, 0);
    if (!background->GetParam(String("FogColor"), fc)) { background->GetParam(String("FogColor"), fogColor); fc = math::vec4<float>(fogColor.x, fogColor.y, fogColor.z, 1); }
    driver.SetFog(fogOn);
    driver.SetFogParams(video::Color(fc.x, fc.y, fc.z, fc.w), fogStart, fogEnd);
    printf("fog %d %g..%g (%g %g %g %g)\n", fogOn, fogStart, fogEnd, fc.x, fc.y, fc.z, fc.w);

    // MenuMgr::SetDefault3dLightFrom(LevelDef UIDefaultLight3D = /MenuUI/MenuUI_DefaultLight)
    LightDef *light = menuLight ? LightDef::Instantiate(menuLight) : 0;
    LightDef::s_camera = &cam;
    // LevelDef OnInitMainMenu: SetPostFx("MAT_Labo_postFx")
    MaterialDef *postFx = getenv("JET_NO_POSTFX") ? 0 : dynamic_cast<MaterialDef*>(project.FindEntityByName(String("MAT_Labo_postFx")));
    video::PostEffect *pe = postFx ? new video::PostEffect(postFx->GetMaterial()) : 0;
    printf("post effect %s\n", pe ? "MAT_Labo_postFx" : "none");
    driver.SetCamera(&cam);
    std::vector<unsigned char> px(W * H * 4);
    // advance the animations by dt ms, then draw one frame
    auto frame = [&](int dt) {
        driver.BeginRendering();
        display.GetRenderTarget()->SetClearColor(math::vec4<float>(fc.x, fc.y, fc.z, 1));
        driver.SetCrtRenderTarget(display.GetRenderTarget());
        glViewport(0, 0, W, H);
        glClearColor(fc.x, fc.y, fc.z, 1);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (light) light->Render();
        LightMgr::GetInstance()->Process();
        if (pe) driver.BeginPostEffect(pe, video::PostFXAADef::None);
        for (int step = 0; step < dt; step += 33) {
            GameEntity::s_frameDt = dt - step < 33 ? dt - step : 33;
            for (Deco3d *d : decos) d->Update();
            for (TouchInteract3d *b : menuButtons) b->Update();
            if (minion) minion->Update();
        }
        driver.SetCamera(&cam);
        driver.AddTime(16);
        for (Deco3d *d : decos) d->Render();
        for (TouchInteract3d *b : menuButtons) b->Render();
        if (minion) minion->Render();
        gameLevel.GetSceneMgr(GameLevel::SceneMgr_World)->SetCamera(&cam);
        gameLevel.GetSceneMgr(GameLevel::SceneMgr_World)->Render();
        if (pe) driver.EndPostEffect(math::mat4<float>());
        driver.EndRendering();
        glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    };
    int t = getenv("JET_TIME") ? atoi(getenv("JET_TIME")) : 1000;
    frame(t);
    printf("glGetError 0x%x\n", glGetError());
    if (sink) {
        while (sink(px.data(), W, H, ud)) frame(33);
    } else {
        FILE *f = fopen(out.c_str(), "wb");
        fprintf(f, "P6\n%u %u\n255\n", W, H);
        for (int y = (int)H - 1; y >= 0; y--)
            for (unsigned x = 0; x < W; x++) fwrite(&px[(y * W + x) * 4], 1, 3, f);
        fclose(f);
        printf("wrote %s\n", out.c_str());
    }
    display.Cleanup();
    return 0;
}

#ifndef JET_MENU_NO_MAIN
int main(int argc, char **argv) { return MenuMain(argc, argv, 0, 0); }
#endif

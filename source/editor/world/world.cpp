// source/editor/world/world.cpp
#include "editor/world/world.h"

#include "editor/world/viewport_display.h"

#include "system.h"
#include "video/gles20/gles20driver.h"
#include "video/gles20/gles20texture.h"
#include "stream/streammgr.h"
#include "sound/soundmgr.h"
#include "localization/babel.h"
#include "game/entities/LightDef.h"
#include "scene/model.h"
#include "scene/scenemgr.h"
#include "game/Game.h"
#include "game/CGameConfig.h"
#include "game/GameLevel.h"
#include "game/gamestates/GameState.h"
#include "game/gamestates/GS_Loading.h"
#include "game/gamestates/GS_MainMenu.h"
#include "game/gamemanagers/ClaraFilesMgr.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/entities/Deco3d.h"
#include "game/entities/Object.h"
#include "scene/camera.h"
#include "project.h"
#include "folder.h"
#include "entity.h"

#include <unistd.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <system_error>

namespace jeteditor::world {

namespace fs = std::filesystem;

namespace {

// The game's objects live for the whole process, as they do in the game's own host: the driver and the display are
// read by the game's singletons, so they are never freed while the editor runs.
ViewportDisplay *s_viewport = nullptr;
jet::video::GLES20Driver *s_driver = nullptr;
Game *s_game = nullptr;
bool s_started = false;

// The device the data was captured on (tests/test_intro.cpp uses the same one): the game picks its colour profiles,
// its compatibility toggles and its GUI config from it
const char *const kDevice = "GT-N7000";

// The editor's orbit: a target, a distance, yaw about the level's up axis (z) and pitch above its plane. The engine's
// quaternions turn the other way round from the usual convention (a rotation by a about an axis is given by the
// quaternion for -a), and its product is the reverse of the usual one. Both are why the angles are negated in
// Rotation(): the camera then looks along rot * (0, -1, 0) at the target.
struct OrbitView {
    static constexpr float kFov = 0.9f;  // radians, vertical
    static constexpr float kMinPitch = -1.5f, kMaxPitch = 1.5f;

    math::vec3<float> Position() const {
        const float cp = std::cos(pitch);
        const math::vec3<float> back(-cp * std::sin(yaw), cp * std::cos(yaw), std::sin(pitch));
        return target + back * distance;
    }
    math::quat<float> Rotation() const {
        return math::quat<float>(math::quat<float>::axisZ, -yaw) * math::quat<float>(math::quat<float>::axisX, -pitch);
    }
    // Radians for a drag of the given size in frame pixels
    void Orbit(float dx, float dy) {
        yaw -= dx * 0.01f;
        pitch = std::clamp(pitch + dy * 0.01f, kMinPitch, kMaxPitch);
    }
    void Zoom(float notches) {
        distance = std::clamp(distance * std::pow(0.9f, notches), MinDistance(), MaxDistance());
    }
    float MinDistance() const { return radius * 0.05f + 0.5f; }
    float MaxDistance() const { return radius * 20.0f + 10.0f; }

    math::vec3<float> target{0, 0, 0};
    float yaw = 0.0f;
    float pitch = 0.35f;
    float distance = 100.0f;
    float radius = 20.0f;  // the radius of the drawn entities' box, the reference for the zoom limits
};

OrbitView s_orbit;
// The camera the view draws with: the editor's own, placed by s_orbit. The game's camera manager is not used.
jet::scene::Camera *s_camera = nullptr;

// Templates whose content the game builds or chooses at gameplay start: the level templates (the track's sections, its
// platforms, obstacles, bananas and runners) come from the level sequence, and the rest is picked by the game's state.
const char *const kRuntimeTemplates[] = {
    "TemplateData", "TemplatePool", "PlatformTemplate", "PlatformInstance", "ObstacleTemplate", "ChunkTemplate",
    "BananaTemplate", "BananaTemplatePath", "JumperTemplate", "SpeederTemplate", "InteractiveAnimatedPusher", "Launcher",
    "Runner",
};

bool IsRuntimeTemplate(const std::string &name) {
    for (const char *t : kRuntimeTemplates)
        if (name == t) return true;
    return false;
}

// What a template that has no visual content is for, in the words the inspector uses
std::string NonVisualKind(const std::string &t) {
    if (t.find("Camera") != std::string::npos) return "a camera";
    if (t.find("Light") != std::string::npos) return "a light";
    if (t == "Fluffy" || t == "Rocket") return "a mount";
    if (t.find("Input") != std::string::npos) return "an input definition";
    if (t.find("Gameplay") != std::string::npos || t == "LevelDef") return "a gameplay definition or mode";
    return "a definition";
}

// The box of a model in world space at its current pose
void SetBounds(LevelEntity &le) {
    if (!le.drawn) return;
    const math::aabb<float> box = le.drawn->ComputeAbsoluteAccurateBoundingBox();
    le.boundsMin[0] = box.minPoint.x;
    le.boundsMin[1] = box.minPoint.y;
    le.boundsMin[2] = box.minPoint.z;
    le.boundsMax[0] = box.maxPoint.x;
    le.boundsMax[1] = box.maxPoint.y;
    le.boundsMax[2] = box.maxPoint.z;
    le.hasBounds = true;
}

// One entry of the level file: its name, template, position, and what the view does with it. The classification is
// read from the level's data and from the game's loaded objects (a Deco3d with a loaded model, an Object with its deco).
LevelEntity Classify(clara::DataEntity *e) {
    LevelEntity le;
    le.name = e->GetName().c_str();
    le.type = e->GetTemplateName().c_str();
    if (clara::Entity *placed = dynamic_cast<clara::Entity *>(e)) {
        const math::vec3<float> &p = placed->GetPosition();
        le.x = p.x;
        le.y = p.y;
        le.z = p.z;
        le.placed = true;
        le.source = placed;
    }

    Deco3d *deco = dynamic_cast<Deco3d *>(e);
    const bool viaObject = !deco;
    if (!deco)
        if (Object *obj = dynamic_cast<Object *>(e)) deco = obj->GetDeco();

    if (deco && deco->GetModel()) {
        le.content = Content::Model;
        le.model = deco->GetModelName().c_str();
        le.drawn = deco->GetModel();
        SetBounds(le);
        le.contentNote = std::string("drawn: model ") + le.model + (viaObject ? ", the deco of its objectDeco data" : "");
    } else if (IsRuntimeTemplate(le.type)) {
        le.content = Content::Runtime;
        le.contentNote = "not drawn: runtime track, built at gameplay start";
    } else if (le.type == "AutoplaceInteractiveObjectTemplate") {
        le.content = Content::Runtime;
        le.contentNote = "not drawn: its class is not ported yet";
    } else {
        le.content = Content::NonVisual;
        le.contentNote = "not drawn: " + NonVisualKind(le.type) + ", it has no visual content in the level file.";
    }
    return le;
}

// The level file's own entities: the ones in its folders and their sub-folders (nested ones are parameters of those)
void CollectEntities(clara::Folder &folder, std::vector<LevelEntity> &out) {
    for (unsigned i = 0; i < folder.GetEntityCount(); ++i) {
        clara::DataEntity *e = folder.GetEntity(i);
        if (!e) continue;
        out.push_back(Classify(e));
    }
    for (unsigned i = 0; i < folder.GetFolderCount(); ++i) CollectEntities(folder.GetFolder(i), out);
}

// The editor's camera follows s_orbit; its projection is the game's default for a level camera (the fov only)
void PlaceCamera() {
    if (!s_camera) return;
    s_camera->SetTransform(s_orbit.Position(), s_orbit.Rotation());
    s_camera->SetFov(OrbitView::kFov);
    s_camera->SetNearDistance(0.05f);
    s_camera->SetFarDistance(std::max(1000.0f, s_orbit.MaxDistance() * 4.0f));
}

}  // namespace

World::~World() { Close(); }

bool World::Open(const std::string &dataDir, jet::video::Display *glDisplay, std::string &error) {
    if (open_) {
        error = "a world is already open";
        return false;
    }
    if (s_started) {
        error = "the game was already started in this editor; restart JetEditor to open another world";
        return false;
    }
    if (!glDisplay) {
        error = "the editor window has no GL display to draw the world with";
        return false;
    }
    std::error_code ec;
    const fs::path dir = fs::absolute(dataDir, ec);
    if (ec || !fs::is_regular_file(dir / "levels" / "level.bclara", ec) || !fs::is_regular_file(dir / "profiles.json", ec)) {
        error = dataDir + " is not a MinionRush data folder (no levels/level.bclara or profiles.json)";
        return false;
    }
    // The game reads its files by relative names, so the data folder is the working directory
    if (chdir(dir.c_str()) != 0) {
        error = "cannot enter " + dir.string();
        return false;
    }
    s_started = true;
    open_ = true;
    dataDir_ = dir.string();
    frames_ = 0;

    s_viewport = new ViewportDisplay(glDisplay);
    if (!s_viewport->Create(kWidth, kHeight, error)) {
        open_ = false;
        return false;
    }
    s_driver = new jet::video::GLES20Driver;
    if (!s_driver->Init(s_viewport)) {
        error = "the game's GL driver did not start";
        open_ = false;
        return false;
    }
    jet::System::Init(s_driver, s_viewport);
    s_driver->SetCrtRenderTarget(s_viewport->GetRenderTarget());

    {
        std::ifstream f("profiles.json", std::ios::binary);
        std::stringstream ss;
        ss << f.rdbuf();
        CGameConfig::ComputePlatformToggles(ss.str(), kDevice, CGameConfig::Current());
    }
    CGameConfig::SetPhoneInfo("samsung", kDevice, "4.2.2");
    // The saves and the settings go to a folder of this run, so a run does not start from what an earlier one saved
    {
        std::error_code dec;
        const fs::path saves = fs::temp_directory_path(dec) / ("jeteditor-world-" + std::to_string(getpid()));
        fs::create_directories(saves, dec);
        jet::System::SetUserFolder(jet::String(saves.string().c_str()));
    }

    s_game = new Game;
    jet::System::SetApplication(jet::String("."), s_game, jet::String("Despicable Me"));
    jet::stream::StreamMgr::GetInstance().AddPath(jet::String("."), jet::String("*"));
    s_game->Init();
    s_game->PostInit();
    babel::Babel::GetInstance()->SelectLanguage(babel::Lang::EN, true);
    s_game->LoadFonts(babel::Lang::EN);
    GameState::InitStatesStack();
    // The loading state runs the game's steps for a level with the minion (Steps_Gameplay: the state database is loaded
    // for the minion's state machine, which Steps_Menu does not load). Its target is GoTo_MainMenu, which only pushes
    // the main menu; FinishLoading() drops that, so the gameplay state (GoTo_Gameplay) is never entered.
    ClaraFilesMgr *files = ClaraFilesMgr::GetInstance();
    GameState::PushState(new GS_Loading(GS_Loading::GoTo_MainMenu, GS_Loading::Steps_Gameplay, files->GetLibFiles(),
                                        files->GetLevelFiles()),
                         true);
    return true;
}

void World::Close() {
    if (!open_) return;
    open_ = false;
    // The sound system ends first: its threads read the game's files (tests/test_intro.cpp does the same)
    if (SoundMgr::HasInstance()) delete SoundMgr::GetInstance();
    if (s_viewport) s_viewport->Release();
}

void World::Update() {
    if (!open_) return;
    if (!loaded_) {
        s_game->OnUpdate();  // the loading steps, one budget of them per frame, as the game runs them
        frames_++;
        GameState *crt = GameState::GetCrtState();
        if (crt && crt->GetStateName().Equals(GS_MainMenu::k_stateName)) FinishLoading();
        return;
    }
    RenderEditFrame();
    frames_++;
}

// The loading state has pushed the main menu, which is not part of the static scene. Its state, its menus and its
// sounds are dropped here, before any of its updates runs. The level stays loaded; no level update runs after this.
void World::FinishLoading() {
    GameState::ReleaseStatesStack();
    GameState::InitStatesStack();
    MenuMgr::GetInstance()->ClearStack();

    if (clara::ClaraFile *file = clara::Project::GetInstance()->GetClaraFile()) CollectEntities(file->GetRoot(), entities_);
    if (!s_camera) s_camera = new jet::scene::Camera(s_orbit.Position(), s_orbit.Rotation());
    loaded_ = true;
    FrameLevel();
}

// One frame of the static scene: the models of the drawn entities, at the transforms the level gives them, drawn by the
// engine's scene manager with the editor's camera. No game state is updated, so nothing moves or animates.
void World::RenderEditFrame() {
    jet::video::Driver *drv = jet::System::GetDriver();
    jet::video::Display *display = jet::System::GetDisplay(0);
    GameLevel *level = GameLevel::GetInstance();
    jet::scene::SceneMgr *sm = GameLevel::GetSceneMgr(GameLevel::SceneMgr_World);

    PlaceCamera();
    display->GetRenderTarget()->SetClearColor(level->GetClearColor());
    sm->ClearRenderablesFromAllTags();
    for (const LevelEntity &e : entities_)
        if (e.content == Content::Model && e.drawn) sm->Add(e.drawn);
    sm->SetCamera(s_camera);
    LightMgr::GetInstance()->Process();

    drv->SetDisplay(display);
    drv->BeginRendering();
    drv->SetCrtRenderTarget(display->GetRenderTarget());
    sm->Render();
    drv->EndRendering();
}

unsigned World::Texture() const { return s_viewport && open_ ? s_viewport->ColorTexture() : 0; }

std::string World::StateName() const {
    if (!loaded_) {
        GameState *crt = GameState::GetCrtState();
        return "loading (" + std::string(crt ? crt->GetStateName().c_str() : "no state") + ")";
    }
    if (GameState::GetCrtState()) return std::string("static scene, game state ") + GameState::GetCrtState()->GetStateName().c_str();
    return "static scene, no game state";
}

size_t World::DrawnCount() const {
    size_t n = 0;
    for (const LevelEntity &e : entities_) n += e.content == Content::Model ? 1 : 0;
    return n;
}

// The eight corners of an entity's box projected to frame pixels. False when any corner is behind the camera.
static bool ProjectBox(const LevelEntity &e, float &x0, float &y0, float &x1, float &y1) {
    if (!s_camera || !e.hasBounds) return false;
    x0 = y0 = std::numeric_limits<float>::max();
    x1 = y1 = -std::numeric_limits<float>::max();
    for (int i = 0; i < 8; ++i) {
        const math::vec3<float> corner(i & 1 ? e.boundsMax[0] : e.boundsMin[0], i & 2 ? e.boundsMax[1] : e.boundsMin[1],
                                       i & 4 ? e.boundsMax[2] : e.boundsMin[2]);
        math::vec3<float> screen(0, 0, 0);
        if (!s_camera->Project(screen, corner)) return false;
        x0 = std::min(x0, screen.x);
        y0 = std::min(y0, screen.y);
        x1 = std::max(x1, screen.x);
        y1 = std::max(y1, screen.y);
    }
    return true;
}

bool World::SelectedScreenBox(float &x0, float &y0, float &x1, float &y1) const {
    if (!loaded_ || selected_ < 0 || static_cast<size_t>(selected_) >= entities_.size()) return false;
    const LevelEntity &e = entities_[static_cast<size_t>(selected_)];
    if (e.content != Content::Model) return false;
    return ProjectBox(e, x0, y0, x1, y1);
}

int World::PickAt(float fx, float fy) const {
    if (!loaded_ || !s_camera) return -1;
    const math::vec3<float> eye = s_camera->GetPosition();
    int best = -1;
    float bestDist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < entities_.size(); ++i) {
        const LevelEntity &e = entities_[i];
        float x0, y0, x1, y1;
        if (e.content != Content::Model || !ProjectBox(e, x0, y0, x1, y1)) continue;
        if (fx < x0 || fx > x1 || fy < y0 || fy > y1) continue;
        const math::vec3<float> centre((e.boundsMin[0] + e.boundsMax[0]) / 2, (e.boundsMin[1] + e.boundsMax[1]) / 2,
                                       (e.boundsMin[2] + e.boundsMax[2]) / 2);
        const float d = (centre - eye).getLength();
        if (d < bestDist) {
            bestDist = d;
            best = static_cast<int>(i);
        }
    }
    return best;
}

bool World::SetSelectedAxis(int axis, float value) {
    if (!loaded_ || selected_ < 0 || static_cast<size_t>(selected_) >= entities_.size() || axis < 0 || axis > 2) return false;
    LevelEntity &e = entities_[static_cast<size_t>(selected_)];
    if (!e.source) return false;
    math::vec3<float> p = e.source->GetPosition();
    if (axis == 0) p.x = value;
    else if (axis == 1) p.y = value;
    else p.z = value;
    e.source->SetPosition(p);
    RefreshEntity(static_cast<size_t>(selected_));
    return true;
}

// Re-reads an entity's position and box after its transform changed
void World::RefreshEntity(size_t index) {
    LevelEntity &e = entities_[index];
    if (e.source) {
        const math::vec3<float> &p = e.source->GetPosition();
        e.x = p.x;
        e.y = p.y;
        e.z = p.z;
    }
    SetBounds(e);
}

void World::Orbit(float dx, float dy) {
    if (loaded_) s_orbit.Orbit(dx, dy);
}

void World::Zoom(float notches) {
    if (loaded_) s_orbit.Zoom(notches);
}

// The camera looks at the centre of the drawn entities' boxes from far enough to take the whole box in. With no drawn
// entity the camera looks at the origin from a fixed distance: nothing else is framed.
void World::FrameLevel() {
    s_orbit.pitch = 0.35f;
    s_orbit.yaw = 0.0f;
    float mn[3] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    float mx[3] = {-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
    bool any = false;
    for (const LevelEntity &e : entities_) {
        if (e.content != Content::Model || !e.hasBounds) continue;
        any = true;
        for (int k = 0; k < 3; ++k) {
            mn[k] = std::min(mn[k], e.boundsMin[k]);
            mx[k] = std::max(mx[k], e.boundsMax[k]);
        }
    }
    if (!any) {
        s_orbit.target = math::vec3<float>(0, 0, 0);
        s_orbit.radius = 20.0f;
    } else {
        s_orbit.target = math::vec3<float>((mn[0] + mx[0]) / 2, (mn[1] + mx[1]) / 2, (mn[2] + mx[2]) / 2);
        const math::vec3<float> half((mx[0] - mn[0]) / 2, (mx[1] - mn[1]) / 2, (mx[2] - mn[2]) / 2);
        s_orbit.radius = std::max(1.0f, half.getLength());
    }
    s_orbit.distance = s_orbit.radius / std::sin(OrbitView::kFov * 0.5f) * 1.1f;
}

// The orbit's target moves to the selected entity, keeping the distance (within the zoom limits)
void World::FocusSelected() {
    if (!loaded_ || selected_ < 0 || static_cast<size_t>(selected_) >= entities_.size()) return;
    const LevelEntity &e = entities_[static_cast<size_t>(selected_)];
    if (!e.placed) return;
    s_orbit.target = math::vec3<float>(e.x, e.y, e.z);
    s_orbit.distance = std::clamp(s_orbit.distance, s_orbit.MinDistance(), s_orbit.MaxDistance());
}

}  // namespace jeteditor::world

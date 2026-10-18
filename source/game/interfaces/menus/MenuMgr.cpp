// source/game/interfaces/menus/MenuMgr.cpp
#include "MenuMgr.h"
#include "particlemgr.h"
#include "game/input/TouchMgr.h"
#include <cmath>

#include "Menu_Base.h"
#include "Menu_MainMenu.h"
#include "Menu_Shop.h"
#include "Menu_Options.h"
#include "Menu_Goals.h"
#include "Menu_Pause.h"
#include "Menu_Social.h"
#include "Menu_InviteFriends.h"
#include "Menu_MoviePlayback.h"
#include "Menu_SendChallengeType.h"
#include "Menu_SendChallengeTaunt.h"
#include "Menu_ChallengeResult.h"
#include "Menu_DeathRevive.h"
#include "Menu_ResultEndScreen.h"
#include "Menu_Ingame.h"
#include "Menu_PortraitMessage.h"
#include "game/GameLevel.h"
#include "game/entities/camera/CameraEntity.h"
#include "game/entities/LevelDef.h"
#include "game/entities/LightDef.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceObject.h"
#include "game/utils/GameUtils.h"
#include "entity.h"
#include "movie.h"
#include "project.h"
#include "scene/camera.h"
#include "scene/scenemgr.h"
#include "video/driver.h"

bool MenuMgr::s_paintGUI = true;

// @0x47c874: a perspective camera with near 0.5 and far 10000 (the menu's 3D)
MenuMgr::MenuMgr()
    : m_extraMenusLoaded(false), m_menuState(0), m_hasActions(false), m_actionNumPressedButtonsLastFrame(0), m_layer3DRotationAnim(0), m_cameraEntity(0), m_light(0),
      m_movie(0), m_previousMenu(0), m_ignoreNextTracking(false), m_disableTouchesTimer(0) {
    for (int i = 0; i < 3; i++) {
        m_layer3DPos[i] = math::vec3<float>(0, 0, 0);
        m_layer3DRot[i] = math::quat<float>(0, 0, 0, 1);
    }
    m_camera = new jet::scene::Camera(math::vec3<float>(0, 0, 0), math::quat<float>(0, 0, 0, 1));
    m_camera->SetNearDistance(0.5f);
    m_camera->SetFarDistance(10000.0f);
}

MenuMgr::~MenuMgr() {
    delete m_camera;
    delete m_light;
}

void MenuMgr::Unload() {
    for (std::map<jet::String, Menu_Base*>::iterator it = m_menuArray.begin(); it != m_menuArray.end(); ++it) delete it->second;
    m_menuArray.clear();
    m_menuActions.clear();
    m_currentMenuName = jet::String();
}

void MenuMgr::AddNewMenu(Menu_Base *m) {
    m_menuArray[m->GetMenuName()] = m;
    m->Init();
}

Menu_Base *MenuMgr::GetMenu(const jet::String &n) { return m_menuArray[n]; }

// @0x47cd20: the first menu, the language screen, is made right away
void MenuMgr::Init() {
    m_extraMenusLoaded = false;
    m_menuState = 0;
    m_layer3DRotationAnim = 0;
    m_movie = 0;
    m_cameraEntity = 0;
    m_light = 0;
    m_camera->SetOrthogonal(false);
}

// @0x47cd9c: the other menus, made once. Menus not ported yet are left out.
void MenuMgr::Loading_AddExtraMenus() {
    if (m_extraMenusLoaded) return;
    m_extraMenusLoaded = true;
    AddNewMenu(new Menu_MainMenu());
    AddNewMenu(new Menu_Shop());
    AddNewMenu(new Menu_Options());
    AddNewMenu(new Menu_OptionsContact());
    AddNewMenu(new Menu_Goals());
    AddNewMenu(new Menu_Pause());
    AddNewMenu(new Menu_Social());
    AddNewMenu(new Menu_InviteFriends());
    AddNewMenu(new Menu_MoviePlayback());
    AddNewMenu(new Menu_SendChallengeType());
    AddNewMenu(new Menu_SendChallengeTaunt());
    AddNewMenu(new Menu_ChallengeResult());
    AddNewMenu(new Menu_DeathRevive());
    AddNewMenu(new Menu_ResultEndScreen());
    AddNewMenu(new Menu_Ingame());
    AddNewMenu(new Menu_PortraitMessage());
}

void MenuMgr::OnMouseWheel(int d) { m_menuStack.back()->OnMouseWheel(d); }

// @0x47cfe8: the menu scene under the page, the menu light, then the current menu's page
void MenuMgr::Render(jet::video::Painter &painter) {
    if (!s_paintGUI) return;
    if (m_cameraEntity) {
        CameraController *c = m_cameraEntity->GetCameraController();
        m_camera->SetPosition(c->GetPosition());
        m_camera->SetRotation(c->GetRotation());
        m_camera->SetFov(c->GetFov());
        painter.SetCamera(m_camera);
    }
    if (GameLevel *l = GameLevel::GetInstance()) {
        jet::scene::SceneMgr *sm = l->GetMenuSceneMgr();
        sm->SetCamera(m_camera);
        if (!getenv("JET_NOMENUSCENE")) GameUtils::MenuRenderScene(sm, 0, false, false);
    }
    Apply3DLayersTransform();
    if (m_light && !getenv("JET_NOMENULIGHT")) {
        m_light->AttachToCamera(m_camera, m_light->GetCameraOffset());
        jet::video::Driver::GetCurrent()->ResetLights();
        jet::video::Driver::GetCurrent()->AddLight(m_light->GetLightData()->data);
    }
    SetCameraDataForParticles(*m_camera);
    if (!getenv("JET_NOPAGE")) m_menuStack.back()->Render(*m_camera, painter);
    if (m_cameraEntity) {
        painter.Flush();
        painter.SetCamera(GameLevel::GetInstance()->GetCamera());
    }
}

// @0x4deca8: the particle manager takes the menu camera
void MenuMgr::SetCameraDataForParticles(const jet::scene::Camera &camera) {
    ps::ParticleMgr *pm = ps::ParticleMgr::GetInstance();
    if (!pm) return;
    jet::scene::Camera &cam = const_cast<jet::scene::Camera&>(camera);
    ps::CameraData cd;
    cd.position = cam.GetPosition();
    cd.front = cam.GetFrontVector();
    cd.up = cam.GetUpVector();
    cd.right = cam.GetRightVector();
    const math::vec2<unsigned int> viewport = cam.GetViewportSize();
    cd.width = (int)viewport.x;
    cd.height = (int)viewport.y;
    cd.fov = cam.GetFov();
    cd.orthogonal = cam.IsOrthogonal();
    pm->SetCameraData(cd);
}

// @0x47d3e8: apply the queued push / pop actions, enter the menu now on top, update it
void MenuMgr::Update() {
    for (;;) {
        ACTION_Reset();
        if (m_menuStack.empty()) {
            m_currentMenuName = jet::String();
            _MenuActions_SetActions();
        }
        if (m_menuStack.empty()) break;
        Menu_Base *top = m_menuStack.back();
        if (!m_menuActions.empty() && m_menuState == 0) m_menuState = 2;
        if (m_menuState == 1) {
            top->SetState(0);
            top->SetState(1);
            m_menuState = 0;
            m_currentMenuName = top->GetMenuName();
            m_previousMenu = top;
            m_ignoreNextTracking = false;
        }
        if (m_menuState == 0) {
            const Menu3DConfig &c = top->GetMenu3DConfig();
            for (int i = 0; i < 3; i++) {
                m_layer3DRot[i].setFromEulerXYZ(c.layer[i].Euler());
                m_layer3DPos[i] = c.layer[i].Pos();
            }
            UpdateNoise(GameEntity::s_frameDt);
            top->Update(GameEntity::s_frameDt);
            if (GameLevel::GetInstance()) GameLevel::GetInstance()->RemoveEntities();
            if (!m_menuActions.empty()) m_menuState = 2;
        }
        if (m_menuState != 2) break;
        m_previousMenu = top;
        top->SetState(2);
        _MenuActions_SetActions();
    }
    m_hasActions = false;
}

void MenuMgr::_MenuActions_SetActions() {
    for (size_t i = 0; i < m_menuActions.size(); i++) {
        if (m_menuActions[i].type == 0) _MenuActions_Push(m_menuActions[i].menu);
        else if (m_menuActions[i].type == 1) _MenuActions_Pop();
    }
    m_menuActions.clear();
}

void MenuMgr::_MenuActions_Push(const jet::String &name) {
    m_menuStack.push_back(m_menuArray[name]);
    m_menuState = 1;
}

void MenuMgr::_MenuActions_Pop() {
    if (!m_menuStack.empty()) m_menuStack.pop_back();
    m_menuState = 1;
}

void MenuMgr::PushMenu(const jet::String &name) {
    SMenuAction a = { 0, name };
    m_menuActions.push_back(a);
    m_hasActions = true;
}

// @0x47d834: popping a menu just pushed in the same frame cancels it
void MenuMgr::PopMenu() {
    if (m_menuActions.empty() || m_menuActions.back().type != 0) {
        SMenuAction a = { 1, jet::String() };
        m_menuActions.push_back(a);
    } else {
        m_menuActions.pop_back();
    }
    m_hasActions = true;
}

void MenuMgr::ClearStack() {
    if (!m_menuStack.empty()) {
        m_menuStack.back()->SetState(2);
        m_previousMenu = m_menuStack.back();
    }
    m_menuState = 0;
    m_currentMenuName = jet::String();
    m_menuStack.clear();
}

// @0x47db5c: the three menu 3D layers, each a quad in front of the menu camera as wide as the screen
void MenuMgr::Apply3DLayersTransform() {
    jet::video::Driver *drv = jet::video::Driver::GetCurrent();
    const math::vec2<unsigned int> &sz = drv->GetCrtRenderTarget()->GetSize();
    math::vec2<float> screen((float)sz.x, (float)sz.y);
    math::line3<float> a = m_camera->GetRayFromScreenCoordinates(math::vec2<float>(0, 0));
    math::line3<float> b = m_camera->GetRayFromScreenCoordinates(math::vec2<float>(screen.x, 0));
    math::line3<float> c = m_camera->GetRayFromScreenCoordinates(screen);
    math::vec3<float> scale(math::distance(b.start, a.start), 1.0f, math::distance(c.start, b.start));
    math::vec3<float> center = math::lerp(a.start, c.start, 0.5f);
    for (int i = 0; i < 3; i++) {
        Interface3DLayer layer;
        math::vec3<float> pos = center + m_camera->GetRotation() * m_layer3DPos[i];
        math::quat<float> rot = m_camera->GetRotation() * m_layer3DRot[i];
        InterfaceObject::Update3DLayerTransform(layer, pos, rot, scale, screen);
        InterfaceObject::SetCustomI3DLayerData(i + 1, layer);
    }
}


// @0x47dcf8: the movie starts at its first time and plays once
void MenuMgr::Movie_StartMovie(clara::Movie *m) {
    Movie_Stop();
    m_movie = m;
    if (m) {
        m->SetTime(m->GetStart());
        m->SetPlaying(true);
    }
}

void MenuMgr::Movie_StartMovie(const jet::String &name) { Movie_StartMovie(clara::Project::GetInstance()->FindMovieByName(name)); }

bool MenuMgr::Movie_IsPlaying() {
    if (m_movie && !m_movie->IsPlaying()) Movie_Stop();
    return m_movie != 0;
}

void MenuMgr::Movie_Stop() {
    if (m_movie) m_movie->SetPlaying(false);
    m_movie = 0;
}

void MenuMgr::RefreshMenus() {
    for (size_t i = 0; i < m_menuStack.size(); i++) m_menuStack[i]->Refresh();
}

void MenuMgr::RefreshMenu(const jet::String &n) {
    if (m_currentMenuName.Equals(n)) m_menuStack.back()->Refresh();
}

void MenuMgr::OnUnloadClaraData() {
    m_movie = 0;
    m_cameraEntity = 0;
}

// @0x47decc: the menu light is a copy of the level's default one
void MenuMgr::SetDefault3dLightFrom(LightDef *l) {
    delete m_light;
    m_light = 0;
    if (GameLevel::GetInstance()) GameLevel::GetInstance()->RemoveEntities();
    if (!l) return;
    m_light = LightDef::Instantiate(l);
    m_light->SetEnabled(false);
}

// @0x47d954: the pressed buttons of this frame become "last frame's"
void MenuMgr::ACTION_Reset() {
    m_actionNumPressedButtonsLastFrame = (int)m_actionPressedButtons.size();
    m_actionReleasedButtons.clear();
    m_actionPressedButtons.clear();
}

// @0x47d92c: nothing counts while the touches are disabled
void MenuMgr::ACTION_AddReleasedButton(InterfaceButton *b) {
    if (m_disableTouchesTimer < 1) m_actionReleasedButtons.push_back(b);
}

// @0x47d9c8
void MenuMgr::DisableTouchesFor(int ms) {
    TouchMgr::GetInstance()->Invalidate();
    m_disableTouchesTimer = ms;
}

// @0x47d9fc
void MenuMgr::UpdateDisableTouchesTimer() {
    if (m_disableTouchesTimer > 0) m_disableTouchesTimer -= GameEntity::s_frameDt;
}

// @0x47da58: the three 3D layers wobble around z
void MenuMgr::UpdateNoise(int dt) {
    float noise = sinf(m_layer3DRotationAnim) * 0.05f;
    for (int i = 0; i < 3; i++) {
        math::vec3<float> e = m_layer3DRot[i].getAsEulerXYZ();
        e.z += noise;
        m_layer3DRot[i].setFromEulerXYZ(e);
    }
    m_layer3DRotationAnim += (float)dt * 0.001f;
    if (m_layer3DRotationAnim > 6.2831855f) m_layer3DRotationAnim -= 6.2831855f;
}

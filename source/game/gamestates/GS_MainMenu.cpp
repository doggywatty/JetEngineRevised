// source/game/gamestates/GS_MainMenu.cpp
#include "GS_MainMenu.h"
#include "GS_Loading.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/entities/LevelDef.h"
#include "game/gamemanagers/ClaraFilesMgr.h"
#include "game/interfaces/menus/Menu_MainMenu.h"
#include "game/bodypartdetection/BodyPartOwner.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/interfaces/GameSettings.h"
#include "sound/soundmgr.h"
#include "camera/cameramgr.h"
#include "scene/camera.h"
#include "scene/scenemgr.h"

const jet::String GS_MainMenu::k_stateName("GS_MainMenu");
const jet::String GS_MainMenu::k_standardMusicLabel("ev_main_menu");
const jet::String GS_MainMenu::k_standardAmbientLabel("ev_sfx_menu_screen_loop");

// @0x49336c: the menu scene is set up by the level script, the menu pushed and its intro movie started
GS_MainMenu::GS_MainMenu(const jet::String &extraMenu) : m_wasRenderCalled(false), m_touchedBPO(0), m_touch(0), m_music(-1) {
    LevelDef::GetInstance()->OnInitMainMenu();
    MenuMgr *mm = MenuMgr::GetInstance();
    mm->PushMenu(Menu_MainMenu::k_menuName);
    mm->GetMenu(Menu_MainMenu::k_menuName);
    Menu_MainMenu::StartMainMenuIntroMovie();
    // the world scene's camera (GameLevel+0x3b0): its vertical fov is widened to keep the design's horizontal one
    jet::scene::Camera *cam = GameLevel::GetSceneMgr(GameLevel::SceneMgr_World)->GetCamera();
    if (cam) {
        cam->SetHorizontalFov(Game::GetInstance()->GetHFovFor(cam->GetFov(), false));
        m_hfov = cam->GetFov();
    }
    // @0x5020f4: the music and the ambience are those of the location the game goes to, the standard ones when it has none
    StopMainMenuSfx();
    Game::GetInstance()->StopMenuMusic();
    const int location = GameLevel::GetInstance()->GetNextNodeGraphId();
    m_musicLabel = LocationsMgr::GetInstance()->GetMainMenuMusicLabel(location);
    if (m_musicLabel.empty()) m_musicLabel = k_standardMusicLabel;
    m_ambientLabel = LocationsMgr::GetInstance()->GetMainMenuAmbientLabel(location);
    if (m_ambientLabel.empty()) m_ambientLabel = k_standardAmbientLabel;
    if (!extraMenu.empty()) {
        mm->PushMenu(extraMenu);
        Game::GetInstance()->PlayMenuMusic(m_musicLabel);
    }
}

GS_MainMenu::~GS_MainMenu() {}

// @0x49365c
void GS_MainMenu::ResumeState() {
    if (Menu_Base *m = MenuMgr::GetInstance()->GetMenu(Menu_MainMenu::k_menuName)) m->OnEnter();
}

// @0x5022e0: the ambience stops
void GS_MainMenu::SuspendState() {
    m_touch = 0;
    SoundMgr::GetInstance()->Stop(m_music, 200);
}

// @0x4936e0: the level goes on, the menu has the camera it was given, and a pressed play starts the game
void GS_MainMenu::UpdateState() {
    // @0x502318: the music of the menu plays when nothing does, and so does the ambience, again when it is over
    Game *game = Game::GetInstance();
    SoundMgr *sm = SoundMgr::GetInstance();
    if (game->GetMenuMusic() < 0) game->PlayMenuMusic(m_musicLabel);
    if (m_music < 0) m_music = sm->Play(m_ambientLabel, 500);
    else if (!sm->IsValid(m_music) || !sm->IsPlaying(m_music)) m_music = -1;
    GameLevel *gl = GameLevel::GetInstance();
    gl->Update(false);
    if (jet::scene::Camera *cam = GameLevel::GetSceneMgr(GameLevel::SceneMgr_World)->GetCamera()) cam->SetHorizontalFov(Game::GetInstance()->GetHFovFor(cam->GetFov(), true));
    if (!MenuMgr::GetInstance()->Movie_IsPlaying()) UpdateNormal();
}

// @0x493918
void GS_MainMenu::UpdateNormal() {
    Menu_MainMenu *m = static_cast<Menu_MainMenu*>(MenuMgr::GetInstance()->GetMenu(Menu_MainMenu::k_menuName));
    if (!m->IsStartPlay()) return;
    StartNormalGamePlay();
    m->Reset();
}

// @0x4838e4
void GS_MainMenu::RenderState() {
    GameLevel::GetInstance()->Render();
    m_wasRenderCalled = true;
}

// @0x483c34: leaves the menu scene and loads the level to play
void GS_MainMenu::StartNormalGamePlay() {
    // @0x5028a8: the sounds of the menu are over, the music volume is the saved one again
    SoundMgr *sm = SoundMgr::GetInstance();
    sm->Stop(m_music, 200);
    sm->StopGroup(SoundMgr::k_musicGroupLabel, 0);
    sm->StopGroup(SoundMgr::k_sfxGroupLabel, 0);
    sm->SetGroupVolume(SoundMgr::k_musicGroupLabel, GameSettings::GetInstance()->GetSavedMusicVolume(), 0);
    LevelDef::GetInstance()->OnExitMainMenu();
    ClaraFilesMgr *f = ClaraFilesMgr::GetInstance();
    GameState::PushState(new GS_Loading(GS_Loading::GoTo_Gameplay, GS_Loading::Steps_Gameplay, f->GetLibFiles(), f->GetLevelFiles()), true);
}

// @0x50259c: the body part owner under the finger being followed (m_touch; the position argument is not
// read), first the ones of the menu scene (class 8), when none there the ones of the level scene (class 7)
BodyPartOwner *GS_MainMenu::GetTouchedBPOFor(const math::vec2<float>&) {
    math::vec3<float> hitPos(0, 0, 0);
    math::quat<float> hitRot(0, 0, 0, 1);
    jet::scene::Camera *cam = MenuMgr::GetInstance()->GetCamera();
    math::line3<float> ray = cam->GetRayFromScreenCoordinates(m_touch->pos);
    BodyPartOwner *bpo = BodyPartOwner::BPO_GetProjectileHit(*cam, ray.start, ray.end, hitPos, hitRot, 0, true, true, 8);
    if (!bpo) {
        cam = CameraMgr::GetInstance()->GetSceneCamera();
        ray = cam->GetRayFromScreenCoordinates(m_touch->pos);
        bpo = BodyPartOwner::BPO_GetProjectileHit(*cam, ray.start, ray.end, hitPos, hitRot, bpo, true, true, 7);
    }
    return bpo;
}

// @0x502a3c
void GS_MainMenu::StopMainMenuSfx() {
    SoundMgr *sm = SoundMgr::GetInstance();
    sm->Stop(m_music, 200);
    sm->StopGroup(SoundMgr::k_sfxGroupLabel, 500);
    sm->SetGroupVolume(SoundMgr::k_musicGroupLabel, GameSettings::GetInstance()->GetSavedMusicVolume(), 0);
}

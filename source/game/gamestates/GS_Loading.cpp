// source/game/gamestates/GS_Loading.cpp
#include "GS_Loading.h"
#include "GS_MainMenu.h"
#include "GS_Gameplay.h"
#include "GS_PreGameplay.h"
#include "game/interfaces/menus/Menu_Options.h"
#include "game/leveltemplate/LevelTemplateMgr.h"
#include "game/online/tracking/SessionTrackingMgr.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/actors/statemachine/StateDatabase.h"
#include "game/gamemanagers/ClaraFilesMgr.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/gamemanagers/PainterMgr.h"
#include "game/utils/GameUtils.h"
#include "anim/animationloader.h"
#include "core/util.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "project.h"
#include "scene/camera.h"
#include "scene/modelloader.h"
#include "stream/streammgr.h"
#include "../gamemanagers/PowerUpMgr.h"
#include "system.h"
#include "video/driver.h"
#include "video/textureloader.h"
#include <cstdio>
#include <cstdlib>

const jet::String GS_Loading::k_stateName("GS_Loading");
bool GS_Loading::s_showingOpenCredits = true;

static const jet::String k_firstTip("QUICK_TIP_01"), k_firstDownloadMessage("DOWNLOAD_MSG_WAIT_01");
static const int k_stepTimeCredits = 2000, k_tipTime = 10000, k_loadingUpdateTimeLeft = 10;

// the steps of each list (@0xb7dc24, 0xb7dc74, 0xb7dccc, 0xb7dd2c)
enum {
    S_WAIT, S_CLEAR_MENUS, S_NONE2, S_NONE3, S_NONE4, S_PAUSE_STREAMING, S_CREATE_MGRS, S_FREE_MGRS, S_UNLOAD, S_NEW_LEVEL, S_LOAD_LEVEL, S_PROJECT_INIT,
    S_PROJECT_POSTINIT, S_NONED, S_NONEE, S_NONEF, S_FREE_TEXTURES, S_LOADING_DONE, S_WAIT_SKIP, S_FREE_MODELS, S_LIBS, S_NONE15, S_STREAMS, S_STATES,
    S_MENUS, S_SOUNDPACK, S_UNLOAD_LIBS, S_WAIT_CREDITS
};
static const int k_stepsBoot[] = { 0, 0x1a, 0x15, 1, 4, 5, 0xd, 9, 6, 0xa, 0x10, 0xb, 0xc, 0xe, 0xf, 0x14, 0x18, 0x11, 0x1a, 0x1b };
static const int k_stepsMenu[] = { 0, 0x1a, 0x15, 3, 1, 4, 5, 8, 6, 9, 0xa, 0x10, 0x19, 0xb, 0xc, 0xe, 0xf, 0x14, 0x18, 0x11, 0x1a, 0x1b };
static const int k_stepsGame[] = { 0, 0x1a, 0x16, 0x15, 3, 1, 4, 5, 8, 6, 9, 0x17, 0x19, 0xa, 0x10, 0xb, 0xc, 0xe, 0xf, 0x14, 0x18, 0x11, 0x1a, 0x1b };
static const int k_stepsMovie[] = { 0, 0x1a, 0x16, 0x17, 0x19, 0x15, 1, 4, 8, 6, 9, 0xa, 0xb, 0xc, 0x18, 0x11, 0x1a, 0x1b };

// @0x4905a0
GS_Loading::GS_Loading(GoToEnum goTo, LoadingStepsEnum steps, std::map<jet::String, jet::stream::IStream*> &libs, jet::stream::IStream *level)
    : m_streamsAreOwned(true), m_postponeLoading(0), m_waitingSkip(false), m_skipPressed(false), m_doneLoading(false), m_screenChanged(false), m_tipDirty(true), m_noInternetTip(false) {
    m_totalTips = GameUtils::ElementsCount(k_firstTip);
    m_totalDownloadMessages = GameUtils::ElementsCount(k_firstDownloadMessage);
    m_libStreams = libs;
    m_levelStreams[jet::String("level.bclara")] = level;
    InitLoading(goTo, steps);
    game::common::SessionTrackingMgr::GetInstance()->AddSession("LOADING_SESSION");
}

// @0x490774: streams are made from the names as the steps come
GS_Loading::GS_Loading(GoToEnum goTo, LoadingStepsEnum steps, const std::vector<jet::String> &libs, const std::vector<jet::String> &levels)
    : m_streamsAreOwned(true), m_postponeLoading(0), m_waitingSkip(false), m_skipPressed(false), m_doneLoading(false), m_screenChanged(false), m_tipDirty(true), m_noInternetTip(false) {
    m_totalTips = GameUtils::ElementsCount(k_firstTip);
    m_totalDownloadMessages = GameUtils::ElementsCount(k_firstDownloadMessage);
    for (const jet::String &l : libs) m_libStreams[l] = 0;
    for (const jet::String &l : levels) m_levelStreams[l] = 0;
    InitLoading(goTo, steps);
    game::common::SessionTrackingMgr::GetInstance()->AddSession("LOADING_SESSION");
}

// @0x490cc0
GS_Loading::~GS_Loading() {
    Game *g = Game::GetInstance();
    g->SetDisplayedInitialScreens(false);
    if (m_streamsAreOwned) {
        for (auto &s : m_libStreams) delete s.second;
        for (auto &s : m_levelStreams) delete s.second;
    }
}

// @0x490a44
void GS_Loading::InitLoading(GoToEnum goTo, LoadingStepsEnum steps) {
    m_stateID = 2;
    m_goTo = goTo;
    switch (steps) {
    case Steps_Boot: m_loadingSteps = k_stepsBoot; m_loadingStepsLeft = 0x14; break;
    case Steps_Menu: m_loadingSteps = k_stepsMenu; m_loadingStepsLeft = 0x16; break;
    case Steps_Movie: m_loadingSteps = k_stepsMovie; m_loadingStepsLeft = 0x12; break;
    default: m_loadingSteps = k_stepsGame; m_loadingStepsLeft = 0x18; break;
    }
    // @0x4ff2ec: no minimum time is added for the boot (the wait step only needs one millisecond to pass)
    m_startTime = jet::System::GetTime();
    m_stepIndex = 0;
    m_loadingSubStep = 0;
    m_postponeLoading = 0;
    m_waitingSkip = false;
    m_skipPressed = false;
    MenuMgr::GetInstance()->SetCameraEntity(0);
    // not ported: WeeklyChallengeMgr::OnLoadingStarted (@0x42c768, WeeklyChallengeMgr.cpp is empty), called before InitScreens
    InitScreens();
    m_doneLoading = false;
    m_loadingUpdateTimeLeft = k_loadingUpdateTimeLeft;
    if (GameLevel *gl = GameLevel::GetInstance()) gl->SetEntityToSpawnCount(-1);
}

// @0x490c2c: the first launch shows the logos, any other loading goes straight to the tips
void GS_Loading::InitScreens() {
    InitFromGUILevel(jet::String("Loading"));
    m_screenChanged = true;
    m_screenTimer = jet::System::GetTime();
    LoadScreen(s_showingOpenCredits ? Screen_Universal : Screen_Loading);
}

static const char *k_logoNames[] = { "Logo_EN", "Logo_FR", "Logo_DE", "Logo_IT", "Logo_ES", "Logo_JA", "Logo_PT", "Logo_KO", "Logo_ZH", "Logo_RU", "Logo_TR", "Logo_BR", "Logo_AR" };

// @0x492a2c: the splash shows the logo of the language
void GS_Loading::LoadScreen_EnableSplashLogo() {
    InterfaceObject *parent = GetUIObject(jet::String("LogosParent"));
    for (unsigned i = 0; i < parent->GetChildCount(); i++) parent->GetChild(i)->SetVisible(false);
    const char *logo = "Logo_EN";
    switch (babel::Babel::GetInstance()->GetSelectedLanguage().value()) {
    case babel::Lang::IT: logo = "Logo_IT"; break;
    case babel::Lang::DE: logo = "Logo_DE"; break;
    case babel::Lang::AR: logo = "Logo_AR"; break;
    case babel::Lang::ES: logo = "Logo_ES"; break;
    case babel::Lang::FR: logo = "Logo_FR"; break;
    case babel::Lang::PT:
    case babel::Lang::BR: logo = "Logo_BR"; break;
    case babel::Lang::JA: logo = "Logo_JA"; break;
    case babel::Lang::KO: logo = "Logo_KO"; break;
    case babel::Lang::TR: logo = "Logo_TR"; break;
    case babel::Lang::ZH: logo = "Logo_ZH"; break;
    case babel::Lang::RU: logo = "Logo_RU"; break;
    default: break;
    }
    GetUIObject(jet::String(logo))->SetVisible(true);
}

// @0x4924dc
void GS_Loading::LoadScreen(EScreensStepsDef screen) {
    static bool s_canUnloadSpriteTextureData = false;
    s_canUnloadSpriteTextureData = true;
    Game *g = Game::GetInstance();
    g->SetLoadingScreen(screen < Screen_Loading);
    m_currentScreen = screen;
    switch (screen) {
    case Screen_Universal: InitFromGUILevel(jet::String("Loading_universal")); break;
    case Screen_Illumination: InitFromGUILevel(jet::String("Loading_illumination")); break;
    case Screen_Splash:
        InitFromGUILevel(jet::String("Loading_splash"));
        LoadScreen_EnableSplashLogo();
        break;
    case Screen_Loading: {
        g->SetDisplayedInitialScreens(true);
        InitFromGUILevel(jet::String("Loading"));
        bool tip = m_goTo == GoTo_MainMenuFromGame;
        GetUIObject(jet::String("Tip_BG"))->SetVisible(tip);
        GetUIObject(jet::String("NoTip_BG"))->SetVisible(!tip);
        GetUIObject(jet::String("Bonus_Sprite"))->SetVisible(false);
        if (tip)
            GetUIText(jet::String("LoadingTip"))->SetText(babel::Babel::GetInstance()->GetStringMgr()->Get(GameUtils::IncreaseKey(k_firstTip, jet::core::Rand2(0, m_totalTips))));
        InterfacePBar *bar = GetUIPBar(jet::String("downloading_bar"));
        bar->SetValue(0);
        bar->SetVisible(false);
        GetUIObject(jet::String("downloading_percent"))->SetVisible(false);
        GetUIObject(jet::String("downloadingText"))->SetVisible(false);
        break;
    }
    }
}

// @0x490e5c
void GS_Loading::ResumeState() {
    Game *g = Game::GetInstance();
    g->SetIsInLoadingPhase(true);
    g->SetPaused(false);
    for (int i = 0; i <= m_stepIndex; i++)
        if (m_loadingSteps[i] == S_LOADING_DONE) g->SetIsInLoadingPhase(false);
}

// @0x490eb8
void GS_Loading::SuspendState() { Game::GetInstance()->SetIsInLoadingPhase(false); }

// @0x491fb0: a screen every k_stepTimeCredits ms until the loading one
void GS_Loading::UpdateScreens() {
    EScreensStepsDef before = m_currentScreen;
    if (jet::System::GetTime() - m_screenTimer > k_stepTimeCredits) {
        m_screenChanged = true;
        m_currentScreen = (EScreensStepsDef)(m_currentScreen + 1);
        m_screenTimer = jet::System::GetTime();
    }
    if (before != m_currentScreen) {
        LoadScreen(m_currentScreen);
        if (m_currentScreen == Screen_Loading) s_showingOpenCredits = false;
    }
}

// @0x492058: a new tip every k_tipTime ms
void GS_Loading::UpdateLoadingTips() {
    if (!(jet::System::GetTime() - m_screenTimer > k_tipTime) && !(m_tipDirty && m_loadingSteps[m_stepIndex] != 0)) return;
    m_screenTimer = jet::System::GetTime();
    if (m_noInternetTip) return;
    if (m_goTo == GoTo_MainMenu) {
        m_tipDirty = false;
        GetUIObject(jet::String("Tip_BG"))->SetVisible(false);
        GetUIObject(jet::String("NoTip_BG"))->SetVisible(true);
    }
}

// @0x491068's streams: the files named but not opened are opened when the manager can
bool GS_Loading::LoadLevelStreams() {
    jet::stream::StreamMgr &sm = jet::stream::StreamMgr::GetInstance();
    bool all = true;
    for (auto *m : { &m_libStreams, &m_levelStreams })
        for (auto &s : *m) {
            if (!s.second && sm.CanCreateStream(s.first)) s.second = sm.CreateStream(s.first);
            if (!s.second) all = false;
        }
    return all;
}

// @0x491068: the work of the step the list is at; true when this frame's loading is over (postponed or finished)
bool GS_Loading::_UpdateLoadingStep() {
    if (m_postponeLoading > 0 || m_doneLoading) return true;
    if (m_stepIndex == 0) m_loadingUpdateTimeLeft = 0;
    Game *g = Game::GetInstance();
    clara::Project *proj = clara::Project::GetInstance();
    bool advance = true;
    switch (m_loadingSteps[m_stepIndex]) {
    case S_WAIT: advance = jet::System::GetTime() >= m_startTime; break;
    case S_CLEAR_MENUS:
        GameState::PopAllPrevStates();
        MenuMgr::GetInstance()->ClearStack();
        // not ported: IGNotificationMgr::ForceClearStack (@0x50c300, IGNotificationMgr.cpp is empty)
        m_loadingUpdateTimeLeft = 0;
        break;
    case S_UNLOAD:
        Game::OnUnloadClaraData();
        proj->UnloadClaraFile();
        delete GameLevel::GetInstance();
        jet::anim::AnimationLoader::GetInstance()->FreeAll();
        jet::scene::ModelLoader::GetInstance()->FreeAll();
        break;
    // @0x4ffc90: the managers of a run (the light, trail, collision mesh and shape definition managers are not ported or made when asked)
    case S_CREATE_MGRS: new PowerUpMgr; break;
    // @0x4ffc30
    case S_FREE_MGRS: delete PowerUpMgr::GetInstance(); break;
    case S_NEW_LEVEL: new GameLevel(m_goTo == GoTo_Movie); break;
    case S_LOAD_LEVEL: {
        LoadLevelStreams();
        GameLevel *gl = GameLevel::GetInstance();
        gl->Load(m_libStreams, m_levelStreams);
        gl->Init();
        break;
    }
    case S_PROJECT_INIT:
        proj->Init();
        m_loadingSubStep = 0;
        break;
    case S_PROJECT_POSTINIT:
        proj->PostInit();
        if (m_goTo != GoTo_Movie) LevelTemplateMgr::BuildLevelTemplates(false);
        GameLevel::GetInstance()->PostInit();
        jet::video::TextureLoader::GetInstance()->FreeUnreferenced();
        m_loadingSubStep = 0;
        break;
    case S_FREE_TEXTURES: jet::video::TextureLoader::GetInstance()->FreeUnreferenced(); break;
    case S_LOADING_DONE: g->SetIsInLoadingPhase(false); break;
    case S_WAIT_SKIP: advance = m_skipPressed; break;
    case S_FREE_MODELS:
        jet::scene::ModelLoader::GetInstance()->FreeAll();
        jet::anim::AnimationLoader::GetInstance()->FreeAll();
        break;
    case S_STREAMS:
        // @0x4ffef4: once the DLC is there its archives are added, then the level streams are made (the two flags,
        // +0xa8 and +0xa4 == 1, are never set on the way to the menu: the state is allocated zeroed)
        g->AddDlcFilesToFileSystem(false, false);
        advance = LoadLevelStreams();
        break;
    case S_STATES:
        StateDatabase::GetInstance()->Load(jet::String("states"));
        break;
    case S_MENUS: MenuMgr::GetInstance()->Loading_AddExtraMenus(); break;
    case S_UNLOAD_LIBS: ClaraFilesMgr::GetInstance()->UpdateUnloadingLibraries((int)(g->GetUpdateTimeUs() / 1000), -1, 0); break;
    case S_WAIT_CREDITS: advance = !s_showingOpenCredits; break;
    default: break;
    }
    if (advance) {
        if (getenv("JET_DBG")) fprintf(stderr, "loading step %d done (%d left)\n", m_loadingSteps[m_stepIndex], m_loadingStepsLeft - 1);
        m_loadingStepsLeft--;
        m_stepIndex++;
        m_doneLoading = m_loadingStepsLeft == 0;
    }
    m_loadingTime = jet::System::GetTime() - m_startTime;
    if (m_doneLoading) {
        Finish();
        return true;
    }
    return false;
}

// @0x491068's end: the model geometry is loaded and the state the loading was for is pushed
void GS_Loading::Finish() {
    jet::scene::ModelLoader::GetInstance()->LoadAllModelGeometry();
    // @0x500504 (GS_Loading::_UpdateLoadingStep): the level is spawned from its first entity (GameLevel::SetEntityToSpawnCount(1))
    GameLevel::GetInstance()->SetEntityToSpawnCount(1);
    Game *g = Game::GetInstance();
    switch (m_goTo) {
    case GoTo_MainMenu:
        g->SetGameplayStarted(false);
        GameState::PushState(new GS_MainMenu(jet::String()), true);
        break;
    // The .so pushes GS_LanguageScreen instead when Game+0x419 is set; that flag has no member in the port.
    case GoTo_MainMenuFromGame:
        g->SetDisplayedInitialScreens(false);
        g->OnMainMenuLoaded();
        GameState::PushState(new GS_MainMenu(jet::String()), true);
        break;
    // not ported: GoTo_Movie pushes GS_MoviePlayback (@0x502abc; GS_MoviePlayback.cpp is empty)
    case GoTo_Options:
        g->OnMainMenuLoaded();
        GameState::PushState(new GS_MainMenu(Menu_Options::k_menuName), true);
        break;
    // Verified @0x500560-0x5005c4 (GS_Loading::_UpdateLoadingStep): logs "level successfully loaded",
    // adds an "ACTION_PHASE_SESSION" tracking session, pushes GS_Gameplay, then pushes GS_PreGameplay
    // on top of it (so the intro movie/countdown plays first, then pops to reveal GS_Gameplay beneath).
    // The .so then removes the LOADING_SESSION added by the constructor (its GetSession result is unused).
    // The .so reuses GS_Gameplay's singleton when one exists; the port always makes a new one.
    case GoTo_Gameplay: {
        game::common::SessionTrackingMgr::GetInstance()->AddSession("ACTION_PHASE_SESSION");
        GameState::PushState(new GS_Gameplay(), true);
        GameState::PushState(new GS_PreGameplay(), true);
        game::common::SessionTrackingMgr::GetInstance()->RemoveSession("LOADING_SESSION");
        break;
    }
    default: break;
    }
}

// @0x490ee4
void GS_Loading::UpdateState() {
    if (!m_libStreams.size() || !m_levelStreams.size()) GameState::PopState();
    int dt = (int)(Game::GetInstance()->GetUpdateTimeUs() / 1000);
    jet::System::GetDriver()->AddTime(dt);
    UpdateBasicPage(dt);
    if (!s_showingOpenCredits) {
        if (m_currentScreen == Screen_Loading) UpdateLoadingTips();
    } else
        UpdateScreens();
    if (m_loadingUpdateTimeLeft > 0) {
        long long start = jet::System::GetTime();
        while (m_loadingUpdateTimeLeft > 0) {
            bool over = _UpdateLoadingStep();
            if (over || m_loadingUpdateTimeLeft <= jet::System::GetTime() - start) m_loadingUpdateTimeLeft = 0;
        }
    }
}

// @0x492958: the page seen from a camera at the origin, and the loading budget of the next update
void GS_Loading::RenderState() {
    m_loadingUpdateTimeLeft = k_loadingUpdateTimeLeft;
    jet::scene::Camera cam(math::vec3<float>(0, 0, 0), math::quat<float>(0, 0, 0, 1));
    jet::System::GetDriver()->SetCamera(&cam);
    RenderBasicPage(cam, PainterMgr::GetInstance()->Get2DPainter());
    if (m_screenChanged) {
        m_screenChanged = false;
        m_screenTimer = jet::System::GetTime();
    }
}

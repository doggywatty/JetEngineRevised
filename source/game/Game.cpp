// source/game/Game.cpp
#include "Game.h"
#include "colorprofiles.h"
#include "scene/modelloader.h"
#include "GameLevel.h"
#include "interfaces/GameSettings.h"
#include "gamemanagers/BappleMgr.h"
#include "gamemanagers/COPPAMgr.h"
#include "online/savegame/SaveVerifierMgr.h"
#include "Statistics.h"
#include "gamemanagers/TutorialMgr.h"
#include "gamemanagers/BonusUpgradeMgr.h"
#include "gamemanagers/BuyablesMgr.h"
#include "gamemanagers/MissionMgr.h"
#include "gamemanagers/AchievementsMgr.h"
#include "interfaces/PopupMgr.h"
#include "entities/LevelDef.h"
#include "gamemanagers/ClaraFilesMgr.h"
#include "gamemanagers/FontInfoMgr.h"
#include "gamemanagers/PainterMgr.h"
#include "gamestates/GameState.h"
#include "gamestates/GS_Loading.h"
#include "gamestates/GS_EndGame.h"
#include "interfaces/interfaceobjects/BasicPage.h"
#include "input/TouchMgr.h"
#include "input/InputMgr.h"
#include "challenges/ChallengeMgr.h"
#include "online/store/Store.h"
#include "gamemanagers/CostumeMgr.h"
#include "entities/MinionCostume.h"
#include "entities/BonusSet.h"
#include "entities/Bonus.h"
#include "entities/gameitem/GameItemType.h"
#include "online/store/Price.h"
#include "cheats/CheatsMgr.h"
#include "interfaces/aurora/AuroraLevelMgr.h"
#include "interfaces/interfaceobjects/InterfaceObject.h"
#include "interfaces/menus/Menu_Base.h"
#include "interfaces/menus/MenuMgr.h"
#include "interfaces/PopupMgr.h"
#include "online/tracking/SessionTrackingMgr.h"
#include "project.h"
#include "Player.h"
#include "localization/babel.h"
#include "sprites/spritemgr.h"
#include "sprites/fontmgr.h"
#include "text/fontloader.h"
#include "scene/camera.h"
#include "scene/scenemgr.h"
#include "stream/ifilesystem.h"
#include "stream/streammgr.h"
#include "stream/zipfilesystem.h"
#include "stream/streamutil.h"
#include <json/json.h>
#include <fstream>
#include <sstream>
#include "system.h"
#include "sound/soundmgr.h"
#include "gamemanagers/GameSoundMgr.h"
#include "video/display.h"
#include "video/driver.h"
#include "video/textureloader.h"
#include "scene/modelbase.h"
#include "folder.h"
#include "movie.h"
#include "group.h"
#include "video/shaderuniform.h"
#include <cmath>

// @0x501bcc
Game::Game()
    : m_gfxConfig(0), m_gfx(0), m_designWidthScale(1.0f), m_lastTime(jet::System::GetTime()), m_updateStep(k_updateTime), m_scaledStep(0), m_updateCounter(0), m_renderCounter(0),
      m_paused(false), m_oneUpdateOnly(false), m_keepPending(false), m_timeSpeed(1.0f, 1.0f, 500), m_debugTimeRatio(1.0f), m_timeSpeedHold(0), m_isInActionPhase(false),
      m_isInLoadingPhase(false), m_isCinematic(false), m_gameplayStarted(false), m_loadingScreen(false), m_showingInitialScreens(false), m_waitingForLoad(false), m_startMode(0), m_startDistance(0),
      m_curvature(0, 0), m_curvatureFrom(0, 0), m_curvatureTo(0, 0), m_curvatureTime(0), m_curvatureElapsed(0), m_curvatureAnim(false), m_shadowCamera(0) {
    // @0x526c40 (Game::Game): the profile is written XTEA encrypted, 5 times
    m_saveDB.SetEncryption(clara::RecordDB::EncryptionDef::XTEA);
    m_saveDB.SetRedundancyCount(5);
    m_saveDB.SetEncryptionKey(jet::String("ERROR: invalid entity"));
}

Game::~Game() {
    delete m_gfxConfig;
    delete m_shadowCamera;
}

void Game::OnDisplayAttached(unsigned) {}
void Game::OnDisplayDetached(unsigned) {}

// @0x50b0c4
void Game::AddZipFileSystem(const jet::String &path) {
    if (!jet::stream::StreamMgr::GetInstance().IsFile(path)) return;
    std::shared_ptr<jet::stream::IFileSystem> z(new jet::stream::ZipFileSystem(path));
    m_zipFileSystems.push_back(z);
}

const jet::String Game::k_dlcRootFolder("dlcs/");
const jet::String Game::k_dlc_file_extension(".jpk");

// The assets DLCManager::GetSortedOnlyAssetsFeedback reports, in the order of the manhattan table of contents for
// this game version ("1.0.0", Game::InitDlc): {file name, index, installed}. DLCManager (the manhattan download
// client) is not ported; an asset counts as installed when its archive is in the DLC folder, which is what the
// manhattan install task leaves behind.
struct DlcAssetFeedback { std::string name, fileName; int index; bool valid; unsigned size; };
static std::vector<DlcAssetFeedback> GetDlcAssetsFeedback(const jet::String &root) {
    std::vector<DlcAssetFeedback> out;
    std::ifstream idx((std::string(root.c_str()) + "mnhtn_index_android_gbs_global").c_str());
    std::string tocName;
    if (!(idx >> tocName)) return out;
    std::ifstream toc((std::string(root.c_str()) + tocName).c_str(), std::ios::binary);
    std::stringstream ss;
    ss << toc.rdbuf();
    Json::Value v;
    Json::Reader reader;
    if (!reader.parse(ss.str(), v)) return out;
    const Json::Value &assets = v["game_versions"]["1.0.0"];
    int index = 0;
    for (const std::string &name : assets.getMemberNames()) {
        const Json::Value &a = assets[name];
        DlcAssetFeedback f;
        f.name = name;
        f.size = (unsigned)a["default"]["size"].asUInt();
        f.fileName = a["default"]["iris_asset"].asString();
        f.index = a["metadata"]["mandatory"].asInt() ? 0 : ++index;
        f.valid = !f.fileName.empty() && jet::stream::IsFile(root + jet::String(f.fileName.c_str()) + Game::k_dlc_file_extension);
        out.push_back(f);
    }
    return out;
}

bool Game::AddDlcFilesToFileSystem(bool removeOld, bool stopAtMissingIndexed) {
    jet::stream::StreamMgr &sm = jet::stream::StreamMgr::GetInstance();
    std::vector<DlcAssetFeedback> feedback = GetDlcAssetsFeedback(k_dlcRootFolder);
    std::shared_ptr<jet::stream::IStreamFactory> root = sm.AddPath(k_dlcRootFolder, jet::String("*"));
    if (removeOld)
        for (const DlcAssetFeedback &f : feedback)
            if (std::shared_ptr<jet::stream::IStreamFactory> old = sm.FindStreamFactoryByPath(k_dlcRootFolder + jet::String(f.fileName.c_str()) + k_dlc_file_extension))
                sm.RemoveFactory(old);
    std::vector<jet::String> files;
    for (const DlcAssetFeedback &f : feedback) {
        if (f.valid) files.push_back(k_dlcRootFolder + jet::String(f.fileName.c_str()) + k_dlc_file_extension);
        else if (f.index > 0 && stopAtMissingIndexed) { files.clear(); break; }
    }
    bool added = false;
    for (const jet::String &file : files) {
        if (sm.FindStreamFactoryByPath(file)) continue;
        sm.AddPathFront(file, jet::String("*"));
        added = true;
    }
    if (root) sm.RemoveFactory(root);
    return added;
}

bool Game::isInRestrictedAccount = false;

// @0xd0e5cc (static initializer @0x5583b4)
const jet::String Game::k_onDemandDlcAssetNames[6] = { jet::String("dlc_font_ja"), jet::String("dlc_font_zh"), jet::String("dlc_font_ko"), jet::String("dlc_font_tr_ru"),
                                                       jet::String("dlc_apk_update"), jet::String("dlc_optional_costumes") };

// @0xd0e5e4 (static initializer @0x558498)
const jet::String Game::k_locationDlcAssetNames[6] = { jet::String("dlc_background_laboratory"), jet::String("dlc_background_residential_china_main"),
                                                       jet::String("dlc_background_macho_pack_2"), jet::String("dlc_background_beach_china"),
                                                       jet::String("dlc_background_residential_night_china"), jet::String("dlc_background_mall_china") };

// @0x5337dc: asked for from the download client (DLCManager::RequestAssetOnDemand), which is not ported
void Game::DownloadOnDemandDlc(const jet::String &asset, DlcTrackingLocation) { m_onDemandDlcRequested.insert(asset); }

// @0x533904: DLCManager::GetProgress is a percent
float Game::GetOnDemandDlcProgress(const jet::String &asset) const { return (IsOnDemandDlcReady(asset) ? 100.0f : 0.0f) * 0.01f; }

// @0x530b64
void Game::MaxDespPointsArray_Reset() { m_maxDespPoints.clear(); }

// @0x530b9c
void Game::MaxDespPointsArray_Add(float points) { m_maxDespPoints.push_back(jet::core::ProtectedStorage<float>(points)); }

// @0x530c48: the points of the current level of the despicable multiplier; past the last level the last one holds
float Game::MaxDespPointsArray_GetValue() {
    const unsigned level = (unsigned)Statistics::GetInstance()->GetIntValue(-1, StatisticsSaveData::k_statScope_despicableMultiplier, 0);
    if (m_maxDespPoints.empty()) return 0;
    return m_maxDespPoints[level < m_maxDespPoints.size() ? level : m_maxDespPoints.size() - 1].GetValue();
}

// DLCManager::IsAssetRequested @0x464084
bool Game::IsOnDemandDlcRequested(const jet::String &asset) const { return m_onDemandDlcRequested.count(asset) != 0; }

// DLCManager::IsOnDemandReady @0x463ff4
bool Game::IsOnDemandDlcReady(const jet::String &asset) const {
    for (const DlcAssetFeedback &f : GetDlcAssetsFeedback(k_dlcRootFolder))
        if (asset.Equals(f.name)) return f.valid;
    return false;
}

unsigned Game::GetOnDemandDlcSize(const jet::String &asset) const {
    for (const DlcAssetFeedback &f : GetDlcAssetsFeedback(k_dlcRootFolder))
        if (asset.Equals(f.name)) return f.size;
    return 0;
}

// @0x50b1ec: the path is added to every archive added, or to the real files when there are none
std::shared_ptr<jet::stream::IStreamFactory> Game::FileSystem_AddPath(const jet::String &path, const jet::String &ext) {
    jet::stream::StreamMgr &sm = jet::stream::StreamMgr::GetInstance();
    if (m_zipFileSystems.empty()) return sm.AddPath(path, ext);
    std::shared_ptr<jet::stream::IStreamFactory> f;
    for (auto &z : m_zipFileSystems)
        if (std::shared_ptr<jet::stream::IStreamFactory> n = sm.AddPath(z, path, ext)) f = n;
    return f;
}

// @0x50b330
std::shared_ptr<jet::stream::IStreamFactory> Game::FileSystem_AddPathInFront(const jet::String &path, const jet::String &ext) {
    jet::stream::StreamMgr &sm = jet::stream::StreamMgr::GetInstance();
    if (m_zipFileSystems.empty()) return sm.AddPathFront(path, ext);
    std::shared_ptr<jet::stream::IStreamFactory> f;
    for (auto &z : m_zipFileSystems)
        if (std::shared_ptr<jet::stream::IStreamFactory> n = sm.AddPathFront(z, path, ext)) f = n;
    return f;
}

// @0x50b4b4: from the last archive that has the path, else from the real files
std::shared_ptr<jet::stream::IStreamFactory> Game::FileSystem_CreateStreamFactoryFromPath(const jet::String &path, const jet::String &ext) {
    for (int i = (int)m_zipFileSystems.size() - 1; i >= 0; i--)
        if (m_zipFileSystems[i]->FindEntryByPath(0, path).id >= 0) return jet::stream::CreateStreamFactoryFromPath(m_zipFileSystems[i], path, ext);
    return jet::stream::CreateStreamFactoryFromPath(path, ext);
}

// @0x50b5d8
bool Game::FileSystem_IsFile(const jet::String &path) { return jet::stream::StreamMgr::GetInstance().IsFile(path); }
bool Game::FileSystem_IsDirectory(const jet::String &path) { return jet::stream::StreamMgr::GetInstance().IsDirectory(path); }
jet::stream::IStream *Game::FileSystem_CreateStreamFromPath(const jet::String &path) { return jet::stream::StreamMgr::GetInstance().CreateStreamByPath(path); }

// @0x505574: the file's content is not read here, the parameter holds the app version
void Game::SetGameParam(const jet::String &name, const jet::String&) { AddParam(name, m_versionName); }

// @0x502b04: the managers that have been ported
void Game::Init() {
    AddParam(jet::String("server"), jet::String("server"));
    FileSystem_AddPath(jet::String("color_profiles"), jet::String("conf"));
    LoadProfile();  // @0x527680
    jet::Application::Init();
    jet::video::Driver *drv = jet::System::GetDriver();
    const PlatformToggles &t = CGameConfig::Current();
    jet::video::TextureLoader::GetInstance()->SetDefaultTextureFilter(
        safe_enum<jet::video::Texture::FilterDef, unsigned char>((int)t.defaultTextureFiltering));
    jet::video::TextureLoader::GetInstance()->SetBestLod((unsigned)t.textureStartLOD);
    drv->SetSeparateAlphaTextures(true);
    for (const char *d : { ".", "shaders", "levels", "localizationText" }) FileSystem_AddPath(jet::String(d), jet::String("*"));
    new babel::Babel;
    new Player;
    new MissionMgr;
    new AchievementsMgr;
    new BonusUpgradeMgr;
    new TutorialMgr;
    new BuyablesMgr;
    new Statistics;
    new BappleMgr;
    new COPPAMgr;
    new GameSettings;
    new PainterMgr;
    new TouchMgr;
    new FontInfoMgr;
    new jet::text::FontLoader;
    new FontMgr;
    new ClaraFilesMgr;
    new clara::Project;
    new SpriteMgr;
    FileSystem_AddPath(jet::String("datalibs"), jet::String("*"));
    FileSystem_AddPath(jet::String("fonts_ttf"), jet::String("ttf"));
    FileSystem_AddPath(jet::String("fonts_cfg"), jet::String("cfg"));
    FontInfoMgr::GetInstance()->Load(jet::String("gui_fonts_info"));
    m_gfxConfig = new GFXConfig;
    m_gfxConfig->Load(jet::String("gfx_config"));
    new AuroraLevelMgr;
    FontMgr::Set2DRenderingWorldCoords(false);
    PostInitRefreshGFX();
    FileSystem_AddPath(jet::String("textures_android"), jet::String("tga"));
}

// @0x5050f8: the display's GUI configuration: the aurora layers, the sprite list and the palettes
void Game::PostInitRefreshGFX() {
    if (!m_gfxConfig || m_gfx) return;
    math::vec2<unsigned int> sz = jet::System::GetDisplay(0)->GetSize();
    m_gfx = m_gfxConfig->GetConfig(sz.x, sz.y);
    float dw = (float)m_gfx->designWidth, dh = (float)m_gfx->designHeight;
    float sw = (float)m_gfx->screenWidth, sh = (float)m_gfx->screenHeight;
    m_designWidth = dw;
    m_designWidthScale = dw / (sw / sh);
    InterfaceObject::Set3DAspectRatioScale(((float)sz.x / (float)sz.y) / (dw / dh));
    float scale = std::min((float)sz.x / dw, (float)sz.y / dh);
    Sprite::SetDefaultPositionScale(scale);
    FileSystem_AddPath(jet::String("datalibs/") + m_gfx->sprites, jet::String("*"));
    FileSystem_AddPath(jet::String("datalibs/") + m_gfx->fonts, jet::String("*"));
    FileSystem_AddPath(jet::String("datalibs/tilemaps"), jet::String("*"));
    std::shared_ptr<jet::stream::IStreamFactory> layers = FileSystem_CreateStreamFactoryFromPath(jet::String("datalibs/") + m_gfx->guiLayers, jet::String("*"));
    AuroraLevelMgr *a = AuroraLevelMgr::GetInstance();
    a->Load(boost::shared_ptr<jet::stream::IStreamFactory>(layers.get(), [layers](void*) {}), m_gfx->guiNames, false, dw, dh, scale, m_gfx->suffixes);
    a->LoadGraphItemSpriteNames(jet::String("gui_graph_item_sprites"));
    InterfaceObject::LoadPalettesValues(jet::String("gui_pal_list"));
    if (!m_gfx->spritePrefix.empty() && !m_gfx->spritePrefix.Equals("_")) SpriteMgr::GetInstance()->SetPrefix(m_gfx->spritePrefix);
}

// @0x503a68: the managers and the shader defines for the device
void Game::PostInit() {
    ValidateSaveData();  // @0x528768: before the settings are read
    jet::stream::StreamMgr::GetInstance().AddEncryptionPair(jet::String("ERROR: invalid stream"), jet::String(".xgraphml"));
    GameSettings::GetInstance()->LoadGameSettings();
    FileSystem_AddPath(jet::String("models"), jet::String("pig"));
    FileSystem_AddPath(jet::String("anims"), jet::String("anim"));
    FileSystem_AddPath(jet::String("collisions"), jet::String("*"));
    FileSystem_AddPath(jet::String("collisions"), jet::String("hull"));
    FileSystem_AddPath(jet::String("ps"), jet::String("ps"));  // @0x528bf8
    FileSystem_AddPath(jet::String("text"), jet::String("*"));
    FileSystem_AddPath(jet::String("sounds"), jet::String("*"));
    FileSystem_AddPath(jet::String("soundpack"), jet::String("*"));
    // @0x5286fc: the sound managers, and the sound system from the soundpack once its folders are known (@0x528a88)
    new SoundMgr;
    new GameSoundMgr;
    SoundMgr::GetInstance()->Initialize();
    SoundMgr::GetInstance()->SetMasterVolume(math::clamp(1.0f, 0.0f, 2.0f), 0);
    GameSoundMgr::GetInstance()->Init();
    ApplySoundSettings();
    const PlatformToggles &t = CGameConfig::Current();
    CGameConfig::AddShaderDefines(t, jet::System::GetDriver());
    if (t.useShadows > 0) InitShadowMapData();
    jet::scene::ModelBase::s_hwSkinningBoneLimit = 30;
    PainterMgr::GetInstance()->Init();
    new MenuMgr;
    MenuMgr::GetInstance()->Init();
    new PopupMgr;
    // Not placed here from disassembly (SessionTrackingMgr::SessionTrackingMgr() is trivial enough to
    // be fully inlined at every call site, so armdis.py -s finds no standalone constructor symbol to
    // locate real callers of) -- added because GS_Gameplay::InitSessions() and GS_Loading::Finish()'s
    // GoTo_Gameplay case both now call SessionTrackingMgr::GetInstance() and crash on a null singleton
    // otherwise. Every other Singleton<T> this tree uses is constructed right here the same way.
    new game::common::SessionTrackingMgr;
    // Same reasoning as SessionTrackingMgr above: InputMgr::UpdateSlideActions/Update are now real but
    // nothing constructed an instance anywhere in this tree. See InputMgr.h for why this uses
    // Singleton<T> instead of the real binary's glf::App::GetInputMgr() accessor.
    new InputMgr;
    // Same reasoning again: Menu_Pause::UpdateExit() (now real) calls ChallengeMgr::GetInstance() and
    // needs a non-null singleton.
    new ChallengeMgr;
    // Price entities register in the store's catalogue as the level loads
    new Store;
    new CostumeMgr;
    new SaveVerifierMgr;
    // @0x52921c: the cheats (the invincible cheat is read by the boss and the minion)
    new CheatsMgr;
    // @0x528770: the profile is read and the managers take their records (the host may have picked another user folder
    // since the constructor)
    m_saveDB.SetPath(jet::String::null);
    m_saveDB.SetName(jet::String("savegame"));
    m_saveDB.Load();
    Load();
    // @0x529254: from here on the profile can be written
    m_postInitDone = true;
}

int Game::GoToShop = 0;

// @0x52a040 (Game::Shutdown): the user data is saved before the game goes; the sound system ends with it (its threads, and the file of the audio output)
void Game::Shutdown() {
    OnSaveUserData();
    if (SoundMgr::HasInstance()) delete SoundMgr::GetInstance();
    jet::Application::Shutdown();
}

// @0x52ee00: the volume of the music is the saved one of the music, the one of the sfx and of the interface the saved one of the sound
void Game::ApplySoundSettings() {
    GameSettings *gs = GameSettings::GetInstance();
    SoundMgr *sm = SoundMgr::GetInstance();
    sm->SetGroupVolume(SoundMgr::k_musicGroupLabel, gs->GetSavedMusicVolume(), 0);
    sm->SetGroupVolume(SoundMgr::k_sfxGroupLabel, gs->GetSavedSoundVolume(), 0);
    sm->SetGroupVolume(SoundMgr::k_uiGroupLabel, gs->GetSavedSoundVolume(), 0);
    if (gs->GetSavedMusicVolume() > 0) OnGameMusicPlay();
}

// @0x532188
void Game::PlayMenuMusic(const jet::String &label) {
    StopMenuMusic();
    PushMenuMusic(label);
}

// @0x5321a8
void Game::SetMenuMusicState(const jet::String &state) {
    if (m_menuMusic.empty()) return;
    SoundMgr::GetInstance()->SetState(m_menuMusic.back(), state);
}

// @0x532228
void Game::StopMenuMusic() {
    while (!m_menuMusic.empty()) PopMenuMusic();
}

// @0x532254: the music that plays comes in over half a second
void Game::PushMenuMusic(const jet::String &label) {
    PauseMenuMusic();
    int handle = -1;
    if (SoundMgr::HasInstance()) handle = SoundMgr::GetInstance()->Play(label, 500);
    if (handle >= 0) m_menuMusic.push_back(handle);
}

// @0x532368
void Game::PopMenuMusic() {
    if (m_menuMusic.empty()) return;
    SoundMgr::GetInstance()->Stop(m_menuMusic.back(), 500);
    m_menuMusic.pop_back();
    ResumeMenuMusic();
}

// @0x532404
void Game::PauseMenuMusic() {
    if (!m_menuMusic.empty()) SoundMgr::GetInstance()->Pause(m_menuMusic.back(), 500);
}

// @0x532478
void Game::ResumeMenuMusic() {
    if (!m_menuMusic.empty()) SoundMgr::GetInstance()->Resume(m_menuMusic.back(), 500);
}

bool Game::GetPaintGUI() const { return MenuMgr::s_paintGUI; }
void Game::SetPaintGUI(bool b) { MenuMgr::s_paintGUI = b; }

// @0x508870
void Game::SetTimeSpeed(float target, int transitionMs, int holdMs, std::function<void()> onDone) {
    m_timeSpeed.SetDelay(transitionMs);
    m_timeSpeed.SetTargetValue(target);
    m_timeSpeedHold = holdMs < 1 ? -1 : holdMs + transitionMs;
    m_timeSpeedDone = onDone;
}

// @0x50b964: the device variant of an entity: its name plus a suffix, the first the project has, else itself
clara::DataEntity *Game::GetDeviceSpecificEntity(clara::DataEntity *e) {
    if (!e) return e;
    for (const jet::String &suffix : m_gfx->suffixes) {
        clara::Path p(e->GetName() + suffix);
        if (clara::DataEntity *v = clara::Project::GetInstance()->FindEntityByPath(p)) return v;
    }
    return e;
}

// @0x50b924
clara::DataEntity *Game::GetDeviceSpecificEntity(const jet::String &name) {
    return GetDeviceSpecificEntity(clara::Project::GetInstance()->FindEntityByName(name));
}

// @0x50ba80
clara::Movie *Game::GetDeviceSpecificMovie(clara::Movie *m) {
    if (!m) return m;
    for (const jet::String &suffix : m_gfx->suffixes)
        if (clara::Movie *v = clara::Project::GetInstance()->FindMovieByName(m->GetName() + suffix)) return v;
    return m;
}

// @0x50bb2c
clara::Group *Game::GetDeviceSpecificGroup(clara::Group *g) {
    if (!g) return g;
    for (const jet::String &suffix : m_gfx->suffixes)
        if (clara::Group *v = clara::Project::GetInstance()->FindGroupByName(g->GetName() + suffix)) return v;
    return g;
}

// @0x50bbd8
clara::Folder *Game::GetDeviceSpecificFolder(clara::Folder *f) {
    if (!f) return f;
    for (const jet::String &suffix : m_gfx->suffixes)
        if (clara::Folder *v = clara::Project::GetInstance()->FindFolderByName(f->GetName() + suffix)) return v;
    return f;
}

// @0x505504: the horizontal fov of a vertical one, for the design display or the 3:4 one
float Game::GetHFovFor(float fovY, bool useDesignRatio) const {
    float aspect = 0.75f;
    if (useDesignRatio && m_gfx) aspect = (float)m_gfx->designWidth / (float)m_gfx->designHeight;
    return atanf(tanf(fovY * 0.5f) * aspect) * 2.0f;
}

// @0x50bc84: the curvature jumps (no time) or eases to the new one
void Game::SetCurvatureParams(const math::vec2<float> &c, unsigned ms) {
    m_curvatureTime = (int)ms;
    if (ms == 0) {
        m_curvatureAnim = false;
        m_curvatureElapsed = 0;
        m_curvature = m_curvatureTo = c;
    } else if (!math::equals(m_curvatureTo, c, 1.1920929e-07f)) {
        m_curvatureFrom = m_curvature;
        m_curvatureTo = c;
        m_curvatureElapsed = 0;
        m_curvatureAnim = !math::equals(m_curvature, c, 1.1920929e-07f);
    }
    if (ms == 0)
        jet::System::GetDriver()->SetGlobalUniform(jet::String("CurvatureParams"), jet::video::ShaderUniform(m_curvature));
}

// @0x507810: the touches, the menus and the state, the frame's time to the driver, the curvature easing
void Game::FrameUpdate(unsigned dt) {
    InterfaceObject::UpdateAutoScrollText();
    jet::video::Driver *drv = jet::System::GetDriver();
    MenuMgr *mm = MenuMgr::GetInstance();
    mm->UpdateDisableTouchesTimer();
    if (!m_isInLoadingPhase) {
        if (!m_isCinematic) {
            TouchMgr::GetInstance()->Update();
            // Not verified from disassembly where exactly this real call sits relative to TouchMgr's
            // Update() in the real binary -- added here since this is the one place in this tree a
            // TouchMgr::Update() already runs every frame outside the loading phase, and InputMgr's own
            // Update() needs to run after TouchMgr's so it sees this frame's touch state, not last
            // frame's.
            InputMgr::GetInstance()->Update((int)dt);
            // @0x52c888: while a popup is up for the state and menu (and the level is there) only the popup is updated, the menus
            // and the state stand still. (GameLevel @0x66c, a flag not in this tree, is taken as clear.)
            if (PopupMgr::GetInstance()->IsEnabled(IsInActionPhase()) && GameLevel::GetInstance()) {
                PopupMgr::GetInstance()->Update((int)GetUpdateTimeNotScaled());
            } else {
                mm->Update();
                Menu_Base *menu = mm->GetCurrentPlayingMenu();
                GameState::UpdateCurrentState(menu ? menu->UpdateGameInBG() : true);
            }
        }
    } else
        GameState::UpdateCurrentState(true);
    drv->AddTime((int)(GetUpdateTimeUs() / 1000));
    // @0x52c970
    if (SoundMgr::HasInstance()) SoundMgr::GetInstance()->Update();
    if (GameSoundMgr::HasInstance()) GameSoundMgr::GetInstance()->Update();
    if (m_curvatureAnim) {
        m_curvatureElapsed += (int)dt;
        if (m_curvatureElapsed < m_curvatureTime) {
            if (m_curvatureElapsed < 0)
                m_curvature = m_curvatureFrom;
            else {
                float f = jet::core::QuadraticEaseInOut((float)m_curvatureElapsed / (float)m_curvatureTime).v;
                m_curvature = math::vec2<float>(f * m_curvatureTo.x + (1.0f - f) * m_curvatureFrom.x, f * m_curvatureTo.y + (1.0f - f) * m_curvatureFrom.y);
            }
        } else {
            m_curvature = m_curvatureTo;
            m_curvatureAnim = false;
        }
        drv->SetGlobalUniform(jet::String("CurvatureParams"), jet::video::ShaderUniform(m_curvature));
    }
}

// @0x508684: the state, then whatever the game draws over it
void Game::FrameRender() {
    jet::video::Driver *drv = jet::System::GetDriver();
    drv->SetDisplay(jet::System::GetDisplay(0));
    drv->BeginRendering();
    drv->SetCrtRenderTarget(jet::System::GetDisplay(0)->GetRenderTarget());
    GameState::RenderCurrentState();
    // @0x52d46c: the popups over the state (PopupMgr::SetRTClearColor, render state, is not ported)
    if (!m_isInLoadingPhase && PopupMgr::GetInstance()->IsEnabled(IsInActionPhase()) && GameLevel::GetInstance()) PopupMgr::GetInstance()->Render();
    PainterMgr::GetInstance()->Flush();  // the painters flush at the end of the frame
    drv->EndRendering();
}

// @0x506210: the game time passes in steps of k_updateTime (at most k_maxUpdates a frame), scaled by the time speed
void Game::OnUpdate() {
    long long now = jet::System::GetTime();
    PreUpdate();
    int pending = (int)(now - m_lastTime);
    if (GameState *s = GameState::GetCrtState()) pending = s->GetUpdateTimeDelta(pending);
    if (pending < 0) pending = 0;
    SetTotalFrameTime((unsigned)pending);
    if (m_timeSpeedHold > 0 && (m_timeSpeedHold -= pending) < 1) {
        m_timeSpeedHold = 0;
        m_timeSpeed.SetTargetValue(1.0f);
    }
    if (m_timeSpeed.IsUpdating()) m_timeSpeed.Update(pending);
    if (m_timeSpeedDone && !m_timeSpeed.IsUpdating()) {
        std::function<void()> f = m_timeSpeedDone;
        m_timeSpeedDone = nullptr;
        f();
    }
    float speed = m_timeSpeed.GetCurrentValue() * m_debugTimeRatio;
    int step = m_updateStep;
    m_scaledStep = speed < 1.0f ? (int)((float)step * speed) : step;
    if (speed > 1.0f) pending = (int)((float)pending * speed);
    int budget = (int)k_maxUpdates + 1;
    while (pending >= step) {
        if (--budget == 0) {
            if (!m_keepPending) pending = 0;
            break;
        }
        SetUpdateTime(m_paused ? 0 : (unsigned)m_scaledStep);
        FrameUpdate((unsigned)m_scaledStep);
        if (m_savePending) Save(true, m_savePendingFlag);  // @0x52afd0: what the managers asked to save is written
        m_updateCounter++;
        pending -= step;
        if (m_oneUpdateOnly) break;
    }
    m_lastTime = now - pending;
    SetRenderTime((unsigned)pending);
    FrameRender();
    if (m_frameDone) m_frameDone();
    jet::System::GetDisplay(0)->Swap();
    m_renderCounter++;
}

void Game::OnPointerPressed(int id, int x, int y) { TouchMgr::GetInstance()->OnTouchPress(id, (float)x, (float)y); }
void Game::OnPointerDragged(int id, int x, int y) { TouchMgr::GetInstance()->OnTouchDrag(id, (float)x, (float)y); }
void Game::OnPointerReleased(int id, int x, int y) { TouchMgr::GetInstance()->OnTouchRelease(id, (float)x, (float)y); }
void Game::OnPointerCancelled(int id, int x, int y) { TouchMgr::GetInstance()->OnTouchCanceled(id, (float)x, (float)y); }
void Game::OnKeyPressed(int) {}
void Game::OnKeyReleased(int) {}

const jet::String &Game::GetShadowTagName() { return GameLevel::GetShadowTagName(); }

// @0x504a70: the shadow map target (a power of two from the shadow quality), its tiny depth viewport and the camera
// that looks at the scene from the light
void Game::InitShadowMapData() {
    m_shadowRT = jet::video::RenderTarget::New();
    unsigned size = 0x80;
    for (int i = 1; i < CGameConfig::Current().useShadows; i++) size <<= 1;
    m_shadowRT->Create(jet::String("shadowMapTarget"), jet::String("T565"), math::vec2<unsigned int>(size, size));
    m_shadowRT->SetScissorTest(true);
    m_shadowRT->SetScissorRect(math::rect<unsigned int>(2, 2, size - 4, size - 4));
    m_shadowRT->SetClearPerFrame(true);
    jet::System::GetDriver()->AddRenderTarget(m_shadowRT);
    m_shadowCamera = new jet::scene::Camera(math::vec3<float>(0, 0, 0), math::quat<float>(0, 0, 0, 1));
    m_shadowCamera->SetOrthogonal(true);
    m_shadowCamera->SetOrthogonalZoom(std::max(m_shadowCamera->GetNearDistance(), 6.0f));  // the view is 12 units tall
    m_shadowCamera->SetFov(0.7853982f);
    m_shadowCamera->SetNearDistance(1.5f);
    m_shadowCamera->SetFarDistance(30.0f);
    jet::System::GetDriver()->SetGlobalUniform(jet::String("LightTextureMatrix"), jet::video::ShaderUniform(math::mat4<float>()));
}

// @0x504d08: the scene seen from the shadow camera into the shadow map, and the matrix that takes a world position to it
void Game::RenderShadowMap(jet::scene::SceneMgr *sm) {
    if (CGameConfig::Current().useShadows < 1) {
        sm->SetCamera(GameLevel::GetInstance()->GetCamera());
        return;
    }
    jet::scene::Camera *prev = sm->GetCamera();
    jet::video::Driver *drv = jet::System::GetDriver();
    drv->SetCrtRenderTarget(m_shadowRT);
    sm->SetCamera(m_shadowCamera);
    sm->Render(GetShadowTagName());
    drv->Flush();
    static const float k[16] = { 0.5f, 0, 0, 0, 0, 0.5f, 0, 0, 0, 0, 0.5f, 0, 0.5f, 0.5f, 0.5f, 1.0f };
    static math::mat4<float> bias(k);
    extern bool g_bIsTransitionMacho2Lab;
    if (g_bIsTransitionMacho2Lab) bias.m[12] = 0.492f;
    else if (bias.m[0] == 0.492f) bias.m[0] = 0.5f;
    math::mat4<float> lightMatrix = bias * m_shadowCamera->GetViewProjectionTransform();
    drv->SetGlobalUniform(jet::String("LightTextureMatrix"), jet::video::ShaderUniform(lightMatrix));
    drv->SetCrtRenderTarget(jet::System::GetDisplay(0)->GetRenderTarget());
    sm->SetCamera(prev);
}

bool g_bIsTransitionMacho2Lab = false;

// @0x50cdec
void Game::OnUnloadClaraData() {
    MenuMgr::GetInstance()->OnUnloadClaraData();
    BasicPage::OnUnloadClaraData();
}

// @0x50ce40: the main menu is up, the loading screen is over
void Game::OnMainMenuLoaded() { m_showingInitialScreens = false; }

// @0x50a514: reload the main menu through the loading screen
void Game::GoToMainMenu() {
    ClaraFilesMgr *f = ClaraFilesMgr::GetInstance();
    GameState::PushState(new GS_Loading(GS_Loading::GoTo_MainMenuFromGame, GS_Loading::Steps_Menu, f->GetLibFiles(), f->GetLevelFiles()), true);
}

// @0x50a39c
void Game::LoadFonts(babel::Lang lang) {
    FontMgr *f = FontMgr::GetInstance();
    f->ClearFonts();
    f->SetWrapOnAnyChar(lang.value() == babel::Lang::ZH);
    f->LoadFontDevicePolicy(jet::String(CGameConfig::Current().fontConfig.c_str()), lang);
    FontInfoMgr::GetInstance()->LoadFontsFor(lang);
}

// @0x50bf0c
void Game::FreeRandomizerModels() {
    m_randomizerModels.clear();
    jet::scene::ModelLoader::GetInstance()->FreeUnusedModelBases();
}

// @0x50d98c: the first and the third location are part of the game
bool Game::HasDLCDataFor(int location, bool) {
    if (location == 0 || location == 2) return true;
    return jet::stream::StreamMgr::GetInstance().CanCreateStream(ClaraFilesMgr::GetInstance()->LocationIdToDynamicLibFile(ELocation((ELocationDef::type)location)));
}

// @0x52edac: there is more than size bytes free where the saves go
bool Game::CheckFreeSpace(unsigned size) { return jet::System::GetFreeSpace() > (unsigned long long)size; }

// @0x52c5b0: the end of the run: the GS_EndGame state is pushed on top of the gameplay (the .so passes keepPrev = true for any argument)
void Game::GoToEndScreen(bool) { GameState::PushState(new GS_EndGame(), true); }

// @0x52ddf8
void Game::Save(bool immediate, bool) {
    if (!CheckFreeSpace(0x100000)) {
        m_savePending = m_savePendingFlag = false;
        return;
    }
    if (!m_saveEnabled || !m_postInitDone) {
        m_savePending = m_savePendingFlag = false;
        return;
    }
    if (!immediate) {
        m_savePending = m_savePendingFlag = true;
        return;
    }
    m_savePending = m_savePendingFlag = false;
    SaveToRecordDB(m_saveDB, false);
    m_saveDB.Save();
    // Not ported: CloudSaveGameMgr::SaveGame (the cloud copy of the profile when the cloud save is on) and
    // OnlineUsersMgr::Save (the cache of the online users)
}

// @0x52df14: the records of the managers, in this order (each manager is saved when it exists)
void Game::SaveToRecordDB(clara::RecordDB &db, bool backup) {
    SaveVerifierMgr::GetInstance()->WriteValidSave(db);
    if (Statistics::HasInstance()) Statistics::GetInstance()->SaveToRecordDB(db, jet::String("statistics"), backup);
    // FriendsMgr ("friendsMgr") and RedeemCodeMgr, not ported
    if (Player::HasInstance()) Player::GetInstance()->SaveToRecordDB(db, Player::k_saveRecordName, backup);
    // FacebookAtLaunchMgr, not ported
    if (TutorialMgr::HasInstance()) TutorialMgr::GetInstance()->SaveToRecordDB(db, TutorialMgr::k_saveRecordName, backup);
    if (MissionMgr::HasInstance()) MissionMgr::GetInstance()->SaveToRecordDB(db, MissionMgr::k_saveRecordName, backup);
    // WeeklyChallengeMgr, not ported
    if (CostumeMgr::HasInstance()) CostumeMgr::GetInstance()->SaveToRecordDB(db, CostumeMgr::k_saveRecordName, backup);
    // DailyTokenMgr, not ported
    if (AchievementsMgr::HasInstance()) AchievementsMgr::GetInstance()->SaveToRecordDB(db, AchievementsMgr::k_saveRecordName, backup);
    // TauntsMgr and OnlinePlayerData ("onlinePlayerData"), not ported
    if (BonusUpgradeMgr::HasInstance()) BonusUpgradeMgr::GetInstance()->SaveToRecordDB(db, BonusUpgradeMgr::k_saveRecordName, backup);
    // MessagesMgr ("messagesMgr"), AnticheatingManager ("anticheatingManager"), RandomUsersMgr ("randomUsersMgr") and
    // BappleMgr, not ported
}

// @0x52e65c: a profile that is not this device's is refused (SaveVerifierMgr), then the managers take their
// records in turn, each only when the ones before were read
bool Game::LoadFromRecordDB(clara::RecordDB &db, bool backup) {
    if (!backup && !SaveVerifierMgr::GetInstance()->IsValidSave(db)) return false;
    bool ok = true;
    if (Statistics::HasInstance()) ok = Statistics::GetInstance()->LoadFromRecordDB(db, jet::String("statistics"), backup);
    // FriendsMgr, RedeemCodeMgr and FacebookAtLaunchMgr, not ported
    if (TutorialMgr::HasInstance() && ok) ok = TutorialMgr::GetInstance()->LoadFromRecordDB(db, TutorialMgr::k_saveRecordName, backup);
    if (Player::HasInstance() && ok) ok = Player::GetInstance()->LoadFromRecordDB(db, Player::k_saveRecordName, backup);
    if (MissionMgr::HasInstance() && ok) ok = MissionMgr::GetInstance()->LoadFromRecordDB(db, MissionMgr::k_saveRecordName, backup);
    // WeeklyChallengeMgr, not ported
    if (CostumeMgr::HasInstance() && ok) ok = CostumeMgr::GetInstance()->LoadFromRecordDB(db, CostumeMgr::k_saveRecordName, backup);
    // DailyTokenMgr, not ported
    if (AchievementsMgr::HasInstance() && ok) ok = AchievementsMgr::GetInstance()->LoadFromRecordDB(db, AchievementsMgr::k_saveRecordName, backup);
    // TauntsMgr, OnlinePlayerData, MessagesMgr, AnticheatingManager and RandomUsersMgr, not ported
    if (BonusUpgradeMgr::HasInstance() && ok) ok = BonusUpgradeMgr::GetInstance()->LoadFromRecordDB(db, BonusUpgradeMgr::k_saveRecordName, backup);
    // BappleMgr, not ported
    return ok;
}

// @0x52e5e8
void Game::Load() {
    bool ok = LoadFromRecordDB(m_saveDB, false);
    // a profile that has a best score is of a player who is not a child by the COPPA's rules unless it failed to load
    if (Statistics::GetInstance()->GetBestScore() > 0) COPPAMgr::GetInstance()->SetFlag(0x20, ok);
    // Not ported: OnlineUsersMgr::Load (the cache of the online users)
}

// @0x52db68: the savegame and settings files are read; one that does not load is kept as <file>_cr and removed
void Game::ValidateSaveData() {
    clara::RecordDB save(jet::String("savegame"));
    save.SetFormat(clara::RecordDB::FormatDef::BINARY);
    save.SetEncryption(clara::RecordDB::EncryptionDef::XTEA);
    save.SetRedundancyCount(5);
    save.SetEncryptionKey(jet::String("ERROR: invalid entity"));
    clara::RecordDB settings(jet::String("settings"));
    settings.SetFormat(clara::RecordDB::FormatDef::BINARY);
    if (settings.Exists() && !settings.Load()) {
        printf("Invalid settings save files. Deleting.\n");
        jet::stream::Rename(settings.GetFilename(), settings.GetFilename() + jet::String("_cr"));
        settings.Delete();
    }
    if (save.Exists() && !save.Load()) {
        printf("Invalid save game files. Deleting.\n");
        jet::stream::Rename(save.GetFilename(), save.GetFilename() + jet::String("_cr"));
        save.Delete();
    }
}

// @0x52c210
void Game::OnSaveUserData() {
    printf("INFO: OnSaveUserData\n");
    if (GameSettings::HasInstance()) GameSettings::GetInstance()->SaveGameSettings();
    Save(true, false);
}

// The revive price (the .so's Game::DeathRevive_*). The revives of the run are counted against the free revives of the costume, the revive
// bonus (when it has a level) and a player's count the port does not keep (the .so's Player+0x964, read as 0 here, see the TODO notes).
// @0x530dd4
Price *Game::DeathRevive_GetRevivePrice() {
    const int reviveCount = Statistics::GetInstance()->GetIntValue(-1, StatisticsSaveData::k_statScope_reviveCount, 0);
    int free = CostumeMgr::GetInstance()->GetEquippedCostumeRef()->RevivesForFree();
    const Bonus *bonus = BonusSet::GetInstance()->GetBonus(12, true);
    if (bonus && bonus->GetCurrentLevel() >= 0) free++;
    const int rest = reviveCount - free;
    if (rest < 0) return m_deathReviveFreePrice;
    if (m_deathRevivePrices.empty()) return m_deathReviveFreePrice;
    return rest < (int)m_deathRevivePrices.size() ? m_deathRevivePrices[rest] : m_deathRevivePrices.back();
}

// @0x530eb8: 0 when the run's revives are the free ones, 1 for the bonus revive, 2 for the revives owned (the player's), 3 when a price is paid
int Game::DeathRevive_GetRevivePriceType() {
    const int reviveCount = Statistics::GetInstance()->GetIntValue(-1, StatisticsSaveData::k_statScope_reviveCount, 0);
    int left = reviveCount - CostumeMgr::GetInstance()->GetEquippedCostumeRef()->RevivesForFree();
    if (left < 0) return 0;
    const Bonus *bonus = BonusSet::GetInstance()->GetBonus(12, true);
    if (bonus && bonus->GetCurrentLevel() >= 0) {
        left--;
        if (left < 0) return 1;
    }
    return left >= 0 ? 3 : 2;
}

// @0x530cc0
void Game::DeathRevive_ResetPrices() { m_deathRevivePrices.clear(); }

void Game::SetDeathReviveIncentivePtg(int which, float v) {
    float *ptgs[5] = {&m_deathReviveIncentive.challengePtg, &m_deathReviveIncentive.topFriendPtg, &m_deathReviveIncentive.nextFriendPtg,
                      &m_deathReviveIncentive.missionPtg, &m_deathReviveIncentive.scorePtg};
    *ptgs[which] = v;
}


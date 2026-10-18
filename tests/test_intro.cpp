// tests/test_intro.cpp
#undef NDEBUG
// The game from launch: Game boots, the loading state shows the logos and the tips while the level loads, then the
// main menu runs; a frame is written as a PPM at the given times:
//   test_intro <data dir> <out prefix> [width height] [frame times in ms ...]   (run from the data dir)
#include "testdisplay.h"
#include "video/gles20/gles20driver.h"
#include "stream/streammgr.h"
#include "system.h"
#include "sound/soundmgr.h"
#include "game/Game.h"
#include "game/gamestates/GS_Loading.h"
#include "game/gamestates/GS_MainMenu.h"
#include "game/gamestates/GS_Gameplay.h"
#include "game/GameLevel.h"
#include "game/input/TouchMgr.h"
#include "scene/scenemgr.h"
#include "project.h"
#include "folder.h"
#include "entity.h"
#include "param.h"
#include "scene/camera.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/entities/Minion.h"
#include "game/entities/Object.h"
#include "game/entities/InteractiveObjectTemplate.h"
#include "game/entities/Pacesetter.h"
#include "game/leveltemplate/GameplayTemplateInstance.h"
#include "camera/cameramgr.h"
#include "game/entities/Deco3d.h"
#include "game/Player.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/entities/LocationInfoEntity.h"
#include "game/Statistics.h"
#include "game/entities/Mission.h"
#include "game/entities/MissionCondition.h"
#include "game/gamemanagers/MissionMgr.h"
#include "project.h"
#include "game/entities/Achievement.h"
#include "game/entities/AchievementSet.h"
#include <cstring>
#include "game/leveltemplate/LevelTemplateMgr.h"
#include "game/gamemanagers/ClaraFilesMgr.h"
#include "game/entities/GameEntity.h"
#include "game/entities/Bonus.h"
#include "game/entities/BonusSet.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/cheats/CheatsMgr.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/entities/gameitem/GameItem.h"
#include "game/gamemanagers/BonusUpgradeMgr.h"
#include "game/gamemanagers/AchievementsMgr.h"
#include "game/entities/MinionCostume.h"
#include "game/interfaces/GameSettings.h"
#include "game/gamemanagers/COPPAMgr.h"
#include "unistd.h"
#include "localization/babel.h"
#include <cassert>
#include <typeinfo>
#include "particlemgr.h"
#include "camera/cameramgr.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

using namespace jet;

// JET_RUN=speed: stand-in for the minion's run (the Minion port moves the pacesetter and the camera follows it): once the gameplay
// state is up the minion's pacesetter is set on the current template at that speed, the minion stands where it is and a camera follows it
// from behind and above
namespace {
struct RunCamera : CameraController {
    math::vec3<float> pos;
    math::quat<float> rot;
    jet::String type = jet::String("RunCamera");
    math::vec3<float> GetPosition() const override { return pos; }
    math::quat<float> GetRotation() const override { return rot; }
    float GetFov() const override { return 1.0f; }
    const jet::String &GetType() const override { return type; }
    void Update(const unsigned) override {}
    void Serialize(jet::stream::IStreamW&) const override {}
    void Deserialize(const jet::stream::IStream&) override {}
};
RunCamera s_runCamera;
bool s_runStarted = false;
void DriveRun(GameLevel *gl, float speed) {
    Minion *m = gl->GetMinion();
    Pacesetter *p = gl->GetPacesetter();
    LevelTemplateMgr *t = gl->GetLevelTemplateMgr();
    if (!m || !p || !t || !t->GetCurrentLevelTemplateInstance()) return;
    if (!s_runStarted) {
        s_runStarted = true;
        Game::GetInstance()->SetIsInActionPhase(true);
        if (getenv("JET_RUNNOGUI")) Game::GetInstance()->SetPaintGUI(false);
        m->SetPacesetter(p);
        p->SetCurrentGameplayTemplateInstance(t->GetCurrentLevelTemplateInstance());
        p->ApplyCurrentLaneTransform();
        p->SetSpeed(speed, 0);
        gl->GetCameraMgr()->SetCamera(&s_runCamera, 0);
    }
    m->SetPosition(p->GetPosition());
    m->SetRotation(p->GetRotation());
    if (getenv("JET_DBGRUN")) {
        static int n = 0;
        if (++n % 10 == 0) {
            GameplayTemplateInstance *c = t->GetCurrentLevelTemplateInstance();
            int obstacles = 0, bananaPaths = 0, bananas = 0, interactive = 0;
            for (GameEntity *e : gl->GetVolatileEntities()) {
                const std::string &tn = e->GetTemplateName().c_str();
                if (tn == "ObstacleTemplate") obstacles++;
                else if (tn == "BananaTemplatePath") bananaPaths++;
                else if (tn == "BananaTemplate") bananas++;
                else if (dynamic_cast<InteractiveObjectTemplate*>(e)) interactive++;
            }
            fprintf(stderr, "world volatile %d obstacles %d bananaPaths %d bananas %d interactive %d\n", (int)gl->GetVolatileEntities().size(), obstacles, bananaPaths, bananas, interactive);
            fprintf(stderr, "run pace (%.1f %.1f %.1f) seg %u t %.2f lane %d tmpl %d loc %s list %d cache %d dist %u+%u pace.gti %p cur %p ents %d bif %d decided %d valid %d%d%d\n", p->GetPosition().x, p->GetPosition().y, p->GetPosition().z, p->GetCurrentSegment(),
                    p->GetCurrentSegmentT(), (int)p->GetCurrentLane(), c->GetTemplateID(), c->GetLocation().c_str(), (int)t->GetLevelTemplateInstanceList().size(), (int)t->GetLevelTemplateCache().size(),
                    m->GetDistanceBase(), m->GetDistanceRun(), (void*)p->GetCurrentGameplayTemplateInstance(), (void*)c, (int)c->GetCreatedEntities().size(), (int)c->GetBifurcation(), (int)c->IsBifurcationDecided(), (int)c->IsValidLane((LaneIndex)0), (int)c->IsValidLane((LaneIndex)1), (int)c->IsValidLane((LaneIndex)2));
        }
    }
    // a pause template ends when the minion is pushed out of it (Minion::OnInteractiveAnimatedPusherStop calls ResumeCreation)
    if (GameplayTemplateInstance *c = t->GetCurrentLevelTemplateInstance())
        if (c->IsPauseTemplate() && p->GetCurrentSegment() > 40) t->ResumeCreation();
    // a tee waits for the player's lane: the stand-in takes the left one
    if (GameplayTemplateInstance *c = t->GetCurrentLevelTemplateInstance())
        if (c->GetBifurcation() == Bifurcation_Tee && !c->IsBifurcationDecided() && p->GetCurrentLane() != 0 && p->CanReachLane((LaneIndex)0, false)) p->GoToLane((LaneIndex)0, 300);
    math::vec3<float> a, b;
    math::quat<float> r;
    p->GetAdvancedTransformInPath(6.0f, b, r);
    p->GetAdvancedTransformInPath(0.0f, a, r);
    math::vec3<float> dir = b - a;
    dir.z = 0;
    if (dir.getLength() < 1e-4f) return;
    dir.normalize();
    s_runCamera.pos = a - dir * 7.0f + math::vec3<float>(0, 0, 4.0f);
    math::vec3<float> f = (a + dir * 8.0f) - s_runCamera.pos;
    f.normalize();
    const float yaw = std::atan2(f.x, -f.y), pitch = std::asin(f.z);
    s_runCamera.rot = math::quat<float>(math::quat<float>::axisZ, yaw) * math::quat<float>(math::quat<float>::axisX, pitch);
}
}

// sink is given every finished frame (RGBA, bottom row first) and says whether to go on; without one the frames at the
// given times are written as PPMs
int IntroMain(int argc, char **argv, bool (*sink)(const unsigned char*, unsigned, unsigned, void*), void *ud) {
    std::string data = argc > 1 ? argv[1] : ".";
    std::string out = argc > 2 ? argv[2] : "intro";
    const unsigned W = argc > 4 ? atoi(argv[3]) : 768, H = argc > 4 ? atoi(argv[4]) : 1024;
    std::vector<int> times;
    for (int i = 5; i < argc; i++) times.push_back(atoi(argv[i]));
    if (times.empty()) times = { 500, 2500, 4500, 7000, 12000 };
    setvbuf(stdout, 0, _IONBF, 0);
    if (chdir(data.c_str())) return 1;
    // without the game data the game's init reaches the renderer with no state and crashes; refuse the folder here
    if (access("datalibs", F_OK)) {
        fprintf(stderr, "%s is not a MinionRush data folder (no datalibs/): unzip the game data into data/\n", data.c_str());
        return 1;
    }

    video::DisplayMode mode;
    mode.size = math::vec2<unsigned int>(W, H);
    TestDisplay display;
    assert(display.Init(mode));
    video::GLES20Driver driver;
    assert(driver.Init(&display));
    System::Init(&driver, &display);
    driver.SetCrtRenderTarget(display.GetRenderTarget());

    {
        std::ifstream f("profiles.json", std::ios::binary);
        std::stringstream ss;
        ss << f.rdbuf();
        CGameConfig::ComputePlatformToggles(ss.str(), getenv("JET_DEVICE") ? getenv("JET_DEVICE") : "GT-N7000", CGameConfig::Current());
    }
    // what the phone reports (the model picks the colour profile files in Game::Init)
    CGameConfig::SetPhoneInfo("samsung", getenv("JET_DEVICE") ? getenv("JET_DEVICE") : "GT-N7000", "4.2.2");
    // JET_SAVEDIR: where the profile is kept (the folder the save game and the settings are written in); without it a fresh
    // folder is used, so a run does not start from what an earlier one saved
    {
        std::string saveDir = getenv("JET_SAVEDIR") ? getenv("JET_SAVEDIR") : "/tmp/jet_savegame_" + std::to_string((int)getpid());
        if (saveDir[0] != '/') saveDir = std::string(getenv("PWD") ? getenv("PWD") : ".") + "/" + saveDir;
        System::SetUserFolder(String(saveDir.c_str()));
    }
    Game game;
    System::SetApplication(String("."), &game, String("Despicable Me"));
    stream::StreamMgr::GetInstance().AddPath(String("."), String("*"));
    game.Init();
    game.PostInit();
    if (getenv("JET_NOGUI")) game.SetPaintGUI(false);
    babel::Babel::GetInstance()->SelectLanguage(babel::Lang::EN, true);
    game.LoadFonts(babel::Lang::EN);
    GameState::InitStatesStack();
    ClaraFilesMgr *f = ClaraFilesMgr::GetInstance();
    GameState::PushState(new GS_Loading(GS_Loading::GoTo_MainMenuFromGame, GS_Loading::Steps_Gameplay, f->GetLibFiles(), f->GetLevelFiles()), true);

    std::vector<unsigned char> px(W * H * 4);
    long long start = System::GetTime();
    size_t next = 0;
    bool run = true;
    long long playAt = getenv("JET_PLAY") ? atoi(getenv("JET_PLAY")) : -1;
    // JET_CLICKS="ms:x:y,..." taps the screen; JET_MENU="Name@ms" opens a menu
    // a click, or a drag (JET_DRAGS="ms:x0:y0:x1:y1:durationMs,...": press, move to the end over the duration, release)
    // a drag whose time starts with "x" (JET_DRAGS="x187:...") starts when the minion's x reaches that distance, not at a harness time
    struct Click { long long at; float x, y; int stage; float x1, y1; int duration; bool atX; long long pressAt; };
    std::vector<Click> clicks;
    if (const char *c = getenv("JET_CLICKS")) {
        std::stringstream cs(c);
        std::string item;
        while (std::getline(cs, item, ',')) {
            long long at; float x, y;
            if (sscanf(item.c_str(), "%lld:%f:%f", &at, &x, &y) == 3) clicks.push_back({ at, x, y, 0, x, y, 120, false, 0 });
        }
    }
    if (const char *c = getenv("JET_DRAGS")) {
        std::stringstream cs(c);
        std::string item;
        while (std::getline(cs, item, ',')) {
            long long at; float x, y, x1, y1; int d;
            float atX;
            if (sscanf(item.c_str(), "x%f:%f:%f:%f:%f:%d", &atX, &x, &y, &x1, &y1, &d) == 6) clicks.push_back({ (long long)atX, x, y, 0, x1, y1, d, true, 0 });
            else if (sscanf(item.c_str(), "%lld:%f:%f:%f:%f:%d", &at, &x, &y, &x1, &y1, &d) == 6) clicks.push_back({ at, x, y, 0, x1, y1, d, false, 0 });
        }
    }
    std::string menuName; long long menuAt = -1;
    if (const char *m = getenv("JET_MENU")) { std::string v(m); size_t k = v.find('@'); menuName = v.substr(0, k); menuAt = k == std::string::npos ? 0 : atoll(v.c_str() + k + 1); }
    // JET_STAT="scope:value:ms,..." sets a statistic of the run (the missions and achievements watch them); JET_GRANT="bananas:n:ms,tokens:n:ms" gives currency
    struct Stat { std::string scope; long long value; long long at; bool done; };
    std::vector<Stat> stats;
    if (const char *c = getenv("JET_STAT")) {
        std::stringstream cs(c);
        std::string item;
        while (std::getline(cs, item, ',')) {
            char scope[64]; long long v; long long at;
            if (sscanf(item.c_str(), "%63[^:]:%lld:%lld", scope, &v, &at) == 3) stats.push_back({ scope, v, at, false });
        }
    }
    std::vector<Stat> grants;
    if (const char *c = getenv("JET_GRANT")) {
        std::stringstream cs(c);
        std::string item;
        while (std::getline(cs, item, ',')) {
            char scope[64]; int v; long long at;
            if (sscanf(item.c_str(), "%63[^:]:%d:%lld", scope, &v, &at) == 3) grants.push_back({ scope, v, at, false });
        }
    }
    // JET_MULT="level@ms" sets the level of the missions' multiplier (the unlock of the locations goes by it)
    long long multAt = -1;
    int multLevel = 0;
    if (const char *m = getenv("JET_MULT")) {
        std::string v(m);
        size_t k = v.find('@');
        multLevel = atoi(v.c_str());
        multAt = k == std::string::npos ? 0 : atoll(v.c_str() + k + 1);
    }
    // The save game: JET_BUY="costume:n@ms" (buys and wears the n-th costume of the data), "bonus:n@ms" (buys the next level of
    // the n-th bonus), "settings:v@ms" (sets the music volume to v/100), "coppa:f@ms" (sets the age gate's flag f); JET_SAVE="ms" saves at once; JET_PROFILE="ms"
    // prints what the profile holds (a line "PROFILE key=value ..."); JET_SHUTDOWN="ms" ends the run through Game::Shutdown
    // (OnSaveUserData), as the host does when the application stops; JET_QUIT="ms" ends it at once (a killed process)
    struct Buy { std::string what; int n; long long at; bool done; };
    std::vector<Buy> buys;
    if (const char *c = getenv("JET_BUY")) {
        std::stringstream cs(c);
        std::string item;
        while (std::getline(cs, item, ',')) {
            char what[32]; int n; long long at;
            if (sscanf(item.c_str(), "%31[^:]:%d@%lld", what, &n, &at) == 3) buys.push_back({ what, n, at, false });
        }
    }
    long long saveAt = getenv("JET_SAVE") ? atoll(getenv("JET_SAVE")) : -1;
    long long profileAt = getenv("JET_PROFILE") ? atoll(getenv("JET_PROFILE")) : -1;
    long long shutdownAt = getenv("JET_SHUTDOWN") ? atoll(getenv("JET_SHUTDOWN")) : -1;
    long long quitAt = getenv("JET_QUIT") ? atoll(getenv("JET_QUIT")) : -1;
    bool dumped = false;
    game.SetFrameDoneCallback([&]() {
        {
            const long long t = System::GetTime() - start;
            for (Buy &b : buys)
                if (!b.done && t >= b.at) {
                    b.done = true;
                    if (b.what == "costume") {
                        MinionCostume *c = CostumeMgr::GetInstance()->GetCostumeRef((unsigned)b.n);
                        CostumeMgr::GetInstance()->MarkAsBought(*c);
                        CostumeMgr::GetInstance()->SetEquippedCostume(c, true);
                        printf("BOUGHT costume %s\n", c->GetClaraEntityName().c_str());
                    } else if (b.what == "bonus") {
                        Bonus *bo = BonusSet::GetInstance()->GetBonusesArray()[(size_t)b.n];
                        bo->OnCurrentLevelPurchased(0, 0);
                        printf("BOUGHT bonus %s level %d\n", bo->GetName().c_str(), bo->GetCurrentLevel());
                    } else if (b.what == "coppa") {
                        COPPAMgr::GetInstance()->SetFlag((unsigned)b.n, true);
                        printf("SET coppa flag %d\n", b.n);
                    } else if (b.what == "settings") {
                        GameSettings::GetInstance()->SetMusicVolume((float)b.n / 100.0f);
                        printf("SET music volume %.2f\n", (float)b.n / 100.0f);
                    }
                }
            if (saveAt >= 0 && t >= saveAt) {
                saveAt = -1;
                Game::GetInstance()->Save(true, false);
                GameSettings::GetInstance()->SaveGameSettings();
                printf("SAVED\n");
            }
            if (profileAt >= 0 && t >= profileAt) {
                profileAt = -1;
                Player *pl = Player::GetInstance();
                CostumeMgr *cm = CostumeMgr::GetInstance();
                std::string bought;
                for (unsigned i = 0; i < cm->GetCostumeRefCount(); i++)
                    if (cm->WasBought(*cm->GetCostumeRef(i))) bought += std::string(bought.empty() ? "" : "+") + cm->GetCostumeRef(i)->GetClaraEntityName().c_str();
                std::string bonuses;
                for (Bonus *bo : BonusSet::GetInstance()->GetBonusesArray()) bonuses += std::to_string(bo->GetCurrentLevel()) + ",";
                MissionMgr *mm = MissionMgr::GetInstance();
                std::string missions;
                for (unsigned i = 0; i < mm->GetActiveMissionCount(); i++) missions += std::string(mm->GetActiveMissionInfo(i).id.c_str()) + ":" + std::to_string(mm->GetActiveMissionInfo(i).state) + ",";
                Statistics *st = Statistics::GetInstance();
                printf("PROFILE bananas=%d tokens=%d location=%d unlocked=%d equipped=%s bought=%s bonuses=%s multiplier=%u/%u missions=%s completedMissions=%u achievements=%u tutorialFirstPlay=%d bestScore=%lld statBananas=%d musicVolume=%.2f coppa=%x\n",
                       pl->GetBananas(), pl->GetTokens(), pl->GetCurrentLocation(), pl->IsLocationUnlocked(1), cm->GetEquippedCostumeName().c_str(), bought.c_str(), bonuses.c_str(),
                       mm->GetData().multiplierLevel, mm->GetData().multiplierPoints, missions.c_str(), (unsigned)mm->GetData().completedMissionIds.size(),
                       AchievementsMgr::GetInstance()->GetCountCompletedAchievements(), (int)TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_firstPlay),
                       (long long)st->GetBestScore(), st->GetIntValue(-1, StatisticsSaveData::k_statScope_bananas, 1), GameSettings::GetInstance()->GetSavedMusicVolume(), COPPAMgr::GetInstance()->GetData().flags);
            }
            if (quitAt >= 0 && t >= quitAt) {
                printf("QUIT (save pending: %d)\n", (int)Game::GetInstance()->IsSavePending());
                fflush(stdout);
                _exit(0);
            }
            if (shutdownAt >= 0 && t >= shutdownAt) {
                shutdownAt = -1;
                game.Shutdown();
                printf("SHUTDOWN\n");
                run = false;
            }
        }
        if (getenv("JET_DBGLOC") && System::GetTime() - start >= atoll(getenv("JET_DBGLOC"))) {
            unsetenv("JET_DBGLOC");
            for (int l = 0; l < 6; l++) {
                LocationInfoEntity *li = LocationsMgr::GetInstance()->GetLocationInfo(l);
                fprintf(stderr, "loc %d unlocked %d info %p condition %d hidden %d mult %d base %d\n", l, (int)Player::GetInstance()->IsLocationUnlocked(l), (void*)li, li ? (int)li->IsLocationUnlocked() : -1, li ? (int)li->IsHidden() : -1,
                        li ? li->GetScoreMultiplierValue() : -1, Statistics::GetInstance()->GetBaseScoreMultiplier());
            }
        }
        if (multAt >= 0 && System::GetTime() - start >= multAt) {
            multAt = -1;
            MissionMgr::GetInstance()->GetData().multiplierLevel = (unsigned)multLevel;
            if (getenv("JET_TUTO")) TutorialMgr::GetInstance()->SetTutorialFinished(ETutorialId::k_firstPlay);
        }
        if (getenv("JET_DBGACH") && AchievementSet::HasInstance() && System::GetTime() - start > 15000) {
            unsetenv("JET_DBGACH");
            for (Achievement *a : AchievementSet::GetInstance()->GetAchievements()) {
                int d = 0, t = 0;
                a->GetCompletedInt(d, t);
                fprintf(stderr, "ach %s %s scope %s done %d/%d completed %d\n", a->GetName().c_str(), a->GetAchievementName().c_str(), a->GetConditionsScope().c_str(), d, t, (int)a->IsCompleted());
                for (MissionCondition *c : a->GetConditions()) fprintf(stderr, "    stat %s value %d loc %d type %d\n", c->GetConditionStatScope().c_str(), c->GetIntValue(), c->m_location, c->m_type);
            }
        }
        for (Stat &st : stats)
            if (!st.done && System::GetTime() - start >= st.at) {
                st.done = true;
                Statistics::GetInstance()->SetStatValue(String(st.scope.c_str()), st.value);
                if (getenv("JET_DBGMISSION"))
                    for (unsigned i = 0; i < MissionMgr::GetInstance()->GetActiveMissionCount(); i++) {
                        const MissionMgrSaveData::ActiveMissionInfo &a = MissionMgr::GetInstance()->GetActiveMissionInfo(i);
                        Mission *m = dynamic_cast<Mission*>(clara::Project::GetInstance()->FindEntityByName(a.id));
                        fprintf(stderr, "mission %u %s state %d completed %d loc %d slot %d\n", i, a.id.c_str(), a.state, m ? (int)m->IsCompleted() : -1, m ? m->GetLocation() : -9, m ? m->GetActiveMissionSlot() : -9);
                        if (m) for (MissionCondition *c : m->GetConditions())
                            fprintf(stderr, "  cond scope %s registered %d type %d loc %d completed %d value %d pct %f\n", c->GetConditionStatScope().c_str(), (int)c->m_registered, c->m_type, c->m_location, (int)c->IsCompleted(), c->GetIntValue(), c->GetCompletedPercentage());
                    }
            }
        for (Stat &g : grants)
            if (!g.done && System::GetTime() - start >= g.at) { g.done = true; if (g.scope == "bananas") Player::GetInstance()->AddBananas(g.value); else Player::GetInstance()->AddTokens(g.value); }
        const long long now = System::GetTime() - start;
        // JET_AUTO=1: a swipe pilot for the run. It reads the obstacles ahead of the minion (the entities of the level, by their y lane
        // and x distance), and sends a swipe to the free lane next to it (left = +y, right = -y), or a jump when no lane is free. The
        // swipes are the same drags as JET_DRAGS (they go through the touch manager and the input manager)
        static long long autoNext = 0;
        if (getenv("JET_AUTO") && GameState::GetCrtState() && GameState::GetCrtState()->GetStateName().Equals("GS_Gameplay") && GameLevel::GetInstance() &&
            GameLevel::GetInstance()->GetMinion() && GameLevel::GetInstance()->GetMinion()->GetPacesetter() && now >= autoNext) {
            bool busy = false;
            for (const Click &c : clicks) busy = busy || c.stage != 2;
            GameLevel *gl = GameLevel::GetInstance();
            const math::vec3<float> mp = gl->GetMinion()->GetPosition();
            auto blocked = [&](float laneY) {
                for (GameEntity *e : gl->GetVolatileEntities()) {
                    if (e->GetTemplateName().c_str() != std::string("ObstacleTemplate")) continue;
                    const math::vec3<float> op = e->GetPosition();
                    if (op.x > mp.x + 1.0f && op.x < mp.x + 20.0f && std::fabs(op.y - laneY) < 0.8f) return true;
                }
                return false;
            };
            const float laneY = std::floor(mp.y + 0.5f);
            if (getenv("JET_DBGAUTO"))
                for (GameEntity *e : gl->GetVolatileEntities())
                    if (e->GetTemplateName().c_str() == std::string("ObstacleTemplate") && e->GetPosition().x > mp.x - 2.0f && e->GetPosition().x < mp.x + 14.0f)
                        fprintf(stderr, "auto obstacle at (%.1f %.1f) minion (%.1f %.1f) now %lld\n", e->GetPosition().x, e->GetPosition().y, mp.x, mp.y, now);
            // no new swipe while the minion is still changing lane (it is between two lanes)
            if (!busy && std::fabs(mp.y - laneY) < 0.15f && blocked(laneY)) {
                autoNext = now + 700;
                if (laneY < 0.5f && !blocked(laneY + 1.0f)) clicks.push_back({ now, 300, 400, 0, 100, 400, 150, false, 0 });
                else if (laneY > -0.5f && !blocked(laneY - 1.0f)) clicks.push_back({ now, 100, 400, 0, 300, 400, 150, false, 0 });
                else clicks.push_back({ now, 240, 600, 0, 240, 300, 150, false, 0 });
                fprintf(stderr, "auto swipe at %lld lane %.0f x %.1f\n", now, laneY, mp.x);
            } else if (!busy && std::fabs(mp.y - laneY) < 0.15f) {
                // nothing to dodge: steer to the lane of the next game item ahead (within 30 m), when that lane is free
                for (GameEntity *e : gl->GetVolatileEntities()) {
                    if (e->GetTemplateName().c_str() != std::string("GameItem")) continue;
                    // JET_AUTOITEM=n: only the items of the game item type n are steered to (the large minion is 4)
                    if (const char *want = getenv("JET_AUTOITEM"))
                        if (dynamic_cast<GameItem*>(e) && (int)dynamic_cast<GameItem*>(e)->GetGameItemType() != atoi(want)) continue;
                    const math::vec3<float> ip = e->GetPosition();
                    if (ip.x <= mp.x + 1.0f || ip.x > mp.x + 30.0f || std::fabs(ip.y - laneY) < 0.8f) continue;
                    const float target = std::floor(ip.y + 0.5f);
                    if (std::fabs(target - laneY) > 1.5f || blocked(target)) continue;
                    autoNext = now + 700;
                    if (target > laneY) clicks.push_back({ now, 300, 400, 0, 100, 400, 150, false, 0 });
                    else clicks.push_back({ now, 100, 400, 0, 300, 400, 150, false, 0 });
                    fprintf(stderr, "auto steer to item lane %.0f at x %.1f from %.0f\n", target, ip.x, laneY);
                    break;
                }
            }
        }
        for (Click &c : clicks) {
            // the minion's distance (x) for a distance drag, the harness clock otherwise
            const float crtX = (GameLevel::GetInstance() && GameLevel::GetInstance()->GetMinion()) ? GameLevel::GetInstance()->GetMinion()->GetPosition().x : -1.0f;
            const bool due = c.atX ? crtX >= (float)c.at : now >= c.at;
            if (c.stage == 0 && due) {
                c.pressAt = now;
                fprintf(stderr, "drag press harness=%lld sys=%lld x=%.1f (%.0f,%.0f)->(%.0f,%.0f)\n", now, (long long)System::GetTime(), crtX, c.x, c.y, c.x1, c.y1);
                TouchMgr::GetInstance()->OnTouchPress(7, c.x, c.y);
                c.stage = 1;
            }
            else if (c.stage == 1 && now >= c.pressAt + c.duration) { TouchMgr::GetInstance()->OnTouchRelease(7, c.x1, c.y1); c.stage = 2; }
            else if (c.stage == 1 && (c.x1 != c.x || c.y1 != c.y)) {
                const float f = (float)(now - c.pressAt) / (float)c.duration;
                TouchMgr::GetInstance()->OnTouchDrag(7, c.x + (c.x1 - c.x) * f, c.y + (c.y1 - c.y) * f);
            }
        }
        // JET_PS="file.ps@ms@distance": a particle system placed that far in front of the main camera at ms (and updated by
        // 33 ms every frame: the slow frames of the headless run leave the manager no time to update it)
        static boost::shared_ptr<ps::ParticleSystem> s_ps;
        if (s_ps) s_ps->Update(33);
        if (const char *pv = getenv("JET_PS")) if (GameLevel *gl = GameLevel::GetInstance()) {
            std::string v(pv);
            size_t a = v.find('@'), b = v.find('@', a + 1);
            if (a != std::string::npos && now >= atoll(v.substr(a + 1, b - a - 1).c_str())) {
                unsetenv("JET_PS");
                s_ps = ps::ParticleMgr::GetInstance()->Load(jet::String(v.substr(0, a).c_str()));
                if (s_ps) {
                    s_ps->Reset();
                    s_ps->SetAutoRendering(true);
                    const float dist = b == std::string::npos ? 3.0f : (float)atof(v.c_str() + b + 1);
                    s_ps->SetPosition(gl->GetCameraMgr()->GetSceneCamera()->GetPosition() + gl->GetCameraMgr()->GetSceneCamera()->GetFrontVector() * dist);
                }
            }
        }
        if (getenv("JET_DBGFOV")) if (GameLevel *gl = GameLevel::GetInstance()) {
            static int n = 0;
            if (++n % 200 == 0) fprintf(stderr, "pos %.2f %.2f %.2f near %.2f far %.0f\n", gl->GetCamera()->GetPosition().x, gl->GetCamera()->GetPosition().y, gl->GetCamera()->GetPosition().z, gl->GetCamera()->GetNearDistance(), gl->GetCamera()->GetFarDistance());
            fprintf(stderr, "fov world %.4f menuScene %.4f menu %.4f aspect %.4f\n", gl->GetCamera()->GetFov(), gl->GetMenuSceneMgr()->GetCamera() ? gl->GetMenuSceneMgr()->GetCamera()->GetFov() : -1.0f, MenuMgr::GetInstance()->GetCamera()->GetFov(), gl->GetMenuSceneMgr()->GetCamera() ? gl->GetMenuSceneMgr()->GetCamera()->GetAspectRatio() : -1.0f);
        }
        if (const char *nm = getenv("JET_NAMES")) if (Menu_Base *mb = MenuMgr::GetInstance()->GetCurrentPlayingMenu()) if (now > 12000) {
            unsetenv("JET_NAMES");
            std::stringstream ns(nm); std::string n;
            while (std::getline(ns, n, ',')) if (InterfaceObject *o = mb->GetUIObject(jet::String(n.c_str()))) for (unsigned i = 0; i < mb->GetPage()->GetUIObjectCount(); i++) if (mb->GetPage()->GetUIObject((int)i) == o) fprintf(stderr, "name %u %s\n", i, n.c_str());
        }
        if (const char *fn = getenv("JET_DUMPFOLDER")) if (now > 12000) {
            unsetenv("JET_DUMPFOLDER");
            if (clara::Folder *fo = clara::Project::GetInstance()->FindFolderByName(jet::String(fn)))
                for (unsigned i = 0; i < fo->GetEntityCount() && i < 40; i++) if (auto *e = dynamic_cast<clara::Entity*>(fo->GetEntity(i))) {
                    fprintf(stderr, "ent %s tmpl %s\n", e->GetName().c_str(), e->GetTemplateName().c_str());
                    for (unsigned k = 0; k < e->GetParamCount(); k++) { const clara::Param *pr = e->GetParam(k); fprintf(stderr, "  %s n=%u", pr->GetName().c_str(), pr->GetComponentCount()); if (pr->GetComponentCount()) { if (pr->GetName().Equals("StateMachine")) fprintf(stderr, " sm=%s|%s", pr->GetAsState(0).a.c_str(), pr->GetAsState(0).b.c_str()); else if (pr->GetName().Equals("objectDeco")) fprintf(stderr, " deco=%s", pr->GetAsPath(0).GetAsString().c_str()); else if (pr->GetName().Equals("name") || pr->GetName().Equals("costumeId") || pr->GetName().Equals("isShopElement")|| pr->GetName().Equals("buyType")) fprintf(stderr, " f=%g", pr->GetAsDouble(0)); } fprintf(stderr, "\n"); }
                }
            fprintf(stderr, "folder %s done\n", fn);
        }
        if (getenv("JET_DBGUI") && now >= atoll(getenv("JET_DBGUI"))) {
            unsetenv("JET_DBGUI");
            if (Menu_Base *mb = MenuMgr::GetInstance()->GetCurrentPlayingMenu())
                for (unsigned i = 0; i < mb->GetPage()->GetUIObjectCount(); i++) {
                    InterfaceObject *o = mb->GetPage()->GetUIObject((int)i);
                    if (o && o->IsVisible() && true) fprintf(stderr, "ui %u %s %s %s size %.0fx%.0f pos %.0f,%.0f\n", i, mb->GetPage()->GetUIObjectName(i).c_str(), typeid(*o).name(), dynamic_cast<InterfaceGraph*>(o) ? (std::to_string(((InterfaceGraph*)o)->GetSprite()) + "/" + std::to_string(((InterfaceGraph*)o)->GetAnim()) + "/" + std::to_string(((InterfaceGraph*)o)->GetFrame())).c_str() : dynamic_cast<InterfaceText*>(o) ? ((InterfaceText*)o)->GetText().c_str() : "", o->GetSize().x, o->GetSize().y, o->GetCombinedPos().x, o->GetCombinedPos().y);
                }
        }
        if (const char *hv = getenv("JET_HIDE")) {
            int lo = 0, hi = 0;
            sscanf(hv, "%d:%d", &lo, &hi);
            if (Menu_Base *mb = MenuMgr::GetInstance()->GetCurrentPlayingMenu())
                for (int i = lo; i < hi && i < (int)mb->GetPage()->GetUIObjectCount(); i++)
                    if (InterfaceObject *o = mb->GetPage()->GetUIObject(i)) o->SetVisible(false);
        }
        // JET_INVINCIBLE=1 (test only): the cheat invincibility of the .so (CheatsMgr: the cheats enabled and the invincible entry on, the state
        // CheatsMgr::IsInvincible reads @0x55e080). The obstacles then do not crash the minion. The shipped build keeps the cheats off.
        static bool invincibleSet = false;
        if (!invincibleSet && getenv("JET_INVINCIBLE") && CheatsMgr::HasInstance()) {
            invincibleSet = true;
            CheatsMgr::GetInstance()->SetCheatsEnabled(true);
            CheatsMgr::GetInstance()->SetInvincible(true);
            fprintf(stderr, "JET_INVINCIBLE: cheat invincibility on (IsInvincible %d)\n", (int)CheatsMgr::GetInstance()->IsInvincible());
        }
        // JET_LARGEAT=ms (test only): the large minion starts at that time of the run, as the pickup of its game item starts it
        // (GS_Gameplay::BeginLargeMinionGameplay @0x4fd420), so the mode can be checked without waiting for the item
        static bool largeStarted = false;
        if (!largeStarted && getenv("JET_LARGEAT") && now >= atoll(getenv("JET_LARGEAT")) && GS_Gameplay::HasInstance() && GameLevel::GetInstance() &&
            GameState::GetCrtState() && GameState::GetCrtState()->GetStateName().Equals("GS_Gameplay")) {
            largeStarted = true;
            fprintf(stderr, "JET_LARGEAT: large minion begins at %lld\n", now);
            GS_Gameplay::GetInstance()->BeginLargeMinionGameplay();
        }
        // JET_DBGBONUS=1: the bonuses by index, with the game item type each one is for and its level (the JET_BUY "bonus:n" index)
        static bool bonusDumped = false;
        if (getenv("JET_DBGBONUS") && !bonusDumped && now > 3000 && BonusSet::HasInstance()) {
            bonusDumped = true;
            for (size_t i = 0; i < BonusSet::GetInstance()->GetBonusesArray().size(); i++) {
                Bonus *bo = BonusSet::GetInstance()->GetBonusesArray()[i];
                fprintf(stderr, "bonus %zu %s type %d golden %d level %d\n", i, bo->GetName().c_str(), (int)bo->GetGameItemType(), (int)bo->IsGolden(), bo->GetCurrentLevel());
            }
        }
        // JET_TUTORIAL=1: the first run is done (the menus show their buttons for it)
        if (getenv("JET_TUTORIAL") && TutorialMgr::HasInstance()) TutorialMgr::GetInstance()->SetTutorialFinished(ETutorialId::k_firstPlay);
        // JET_LOCATION=n: the run starts in that location (2 is the macho lab)
        if (getenv("JET_LOCATION") && Player::HasInstance() && GameLevel::GetInstance() && GameLevel::GetInstance()->GetMinion() && !GameLevel::GetInstance()->GetMinion()->GetPacesetter()) Player::GetInstance()->SetCurrentLocation(atoi(getenv("JET_LOCATION")));
        // JET_ONLINE=1: the online services are up (the menus that need them stay open)
        if (getenv("JET_ONLINE")) Game::GetInstance()->SetOnlineServicesReady(true);
        if (menuAt >= 0 && now >= menuAt && dynamic_cast<GS_MainMenu*>(GameState::GetCrtState())) { menuAt = -1; MenuMgr::GetInstance()->PushMenu(jet::String(menuName.c_str())); }
        if (playAt >= 0 && System::GetTime() - start >= playAt)
            if (GS_MainMenu *mm = dynamic_cast<GS_MainMenu*>(GameState::GetCrtState())) { playAt = -1; mm->StartNormalGamePlay(); }
        if (const char *dd = getenv("JET_DUMPDECO")) if (now > 14000 && !dumped && GameLevel::GetInstance()) {
            dumped = true;
            for (GameEntity *e : GameLevel::GetInstance()->GetEntitiesToRender())
                if (Deco3d *d = dynamic_cast<Deco3d*>(e)) {
                    if (!strstr(d->GetModelName().c_str(), dd)) continue;
                    math::vec3<float> p = d->GetPosition();
                    fprintf(stderr, "deco %s (%.2f %.2f %.2f) %s\n", d->GetModelName().c_str(), p.x, p.y, p.z, d->GetName().c_str());
                }
        }
        if (const char *h3 = getenv("JET_HIDE3D")) {
            std::vector<std::string> pats; std::stringstream hs(h3); std::string it; while (std::getline(hs, it, ',')) pats.push_back(it);
            std::vector<GameEntity*> v; if (GameLevel::GetInstance()) v = GameLevel::GetInstance()->GetEntitiesToRender();
            for (GameEntity *e : v)
                if (Deco3d *d = dynamic_cast<Deco3d*>(e))
                    for (auto &pt : pats)
                        if (strstr(d->GetModelName().c_str(), pt.c_str()) || strstr(d->GetName().c_str(), pt.c_str())) d->RegisterForRender(false);
        }
        if (getenv("JET_RUN") && GameState::GetCrtState() && GameState::GetCrtState()->GetStateName().Equals("GS_Gameplay") && GameLevel::GetInstance())
            DriveRun(GameLevel::GetInstance(), (float)atof(getenv("JET_RUN")));
        // JET_DBGENT=1: the entities of the run, by template name, every 3 s of the run (the obstacles, the jump pads, the items)
        static long long entBucket = -1;
        if (getenv("JET_DBGENT") && GameState::GetCrtState() && GameState::GetCrtState()->GetStateName().Equals("GS_Gameplay") && GameLevel::GetInstance() && now / 3000 != entBucket) {
            entBucket = now / 3000;
            std::map<std::string, int> counts;
            for (GameEntity *e : GameLevel::GetInstance()->GetVolatileEntities()) counts[e->GetTemplateName().c_str()]++;
            std::string line;
            for (const auto &kv : counts) line += kv.first + "=" + std::to_string(kv.second) + " ";
            fprintf(stderr, "entities %lld: %s\n", now, line.c_str());
            // the positions of the items and the jump pads (x along the run, y the lane side, z the height), and whether their collision is in the world
            for (GameEntity *e : GameLevel::GetInstance()->GetVolatileEntities()) {
                const std::string &tn = e->GetTemplateName().c_str();
                if (tn != "GameItem" && tn != "JumperTemplate" && tn != "SpeederTemplate" && tn != "ObstacleTemplate") continue;
                Object *o = dynamic_cast<Object*>(e);
                const bool hasCol = o && o->GetCollision();
                const GameItem *gi = dynamic_cast<const GameItem*>(e);
                fprintf(stderr, "  %s%s at (%.1f %.1f %.1f) collision=%d render=%d\n", tn.c_str(), gi ? ("#type" + std::to_string((int)gi->GetGameItemType())).c_str() : "", e->GetPosition().x, e->GetPosition().y, e->GetPosition().z, (int)hasCol, (int)e->IsRegisteredForRender());
            }
        }
        if (getenv("JET_DBGLVL")) {
            GameLevel *gl = GameLevel::GetInstance();
            Minion *m = gl ? gl->GetMinion() : 0;
            LevelTemplateMgr *t = gl ? gl->GetLevelTemplateMgr() : 0;
            if (m) fprintf(stderr, "dbg %s minion (%.1f %.1f %.1f) inst %d dist %u\n", GameState::GetCrtState()->GetStateName().c_str(), m->GetPosition().x, m->GetPosition().y, m->GetPosition().z, t ? (int)t->GetLevelTemplateInstanceList().size() : -1, m->GetTraveledDistance());
        }
        if (sink) {
            glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
            run = sink(px.data(), W, H, ud);
        } else if (next < times.size() && System::GetTime() - start >= times[next]) {
            glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
            char name[256];
            snprintf(name, sizeof(name), "%s_%d.ppm", out.c_str(), times[next]);
            FILE *o = fopen(name, "wb");
            fprintf(o, "P6\n%u %u\n255\n", W, H);
            for (int y = (int)H - 1; y >= 0; y--)
                for (unsigned x = 0; x < W; x++) fwrite(&px[(y * W + x) * 4], 1, 3, o);
            fclose(o);
            printf("wrote %s (state %s)\n", name, GameState::GetCrtState() ? GameState::GetCrtState()->GetStateName().c_str() : "none");
            next++;
        }
    });
    while (run && (sink || next < times.size())) game.OnUpdate();
    // the sound system ends first: its threads read the game's files, and the WAV of JET_AUDIO_WAV is closed
    if (SoundMgr::HasInstance()) delete SoundMgr::GetInstance();
    display.Cleanup();
    return 0;
}

#ifndef JET_INTRO_NO_MAIN
int main(int argc, char **argv) { return IntroMain(argc, argv, 0, 0); }
#endif

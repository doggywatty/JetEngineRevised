// source/game/entities/LocationConfig.cpp
#include "LocationConfig.h"
#include "param.h"
#include "project.h"
#include "Runner.h"

const jet::String LocationConfig::k_tmplName("LocationConfig"), LocationConfig::k_param_LocationId("LocationId"),
    LocationConfig::k_param_MainMenuMusic("MainMenuMusic"), LocationConfig::k_param_MainMenuAmbient("MainMenuAmbient"),
    LocationConfig::k_param_RenderPlatforms("RenderPlatforms"), LocationConfig::k_param_FluffyBackground("FluffyBackground"),
    LocationConfig::k_param_CannonBackgrounds("CannonBackgrounds"), LocationConfig::k_param_CannonTemplates("CannonTemplates"),
    LocationConfig::k_param_hasBoss("HasBoss"), LocationConfig::k_param_bossReference("BossReference");
// the large minion configuration's params (LocationConfig::LoadLargeMinionConfig @0x57283c and its loaders)
static const jet::String k_param_LargeMinionConfig("LargeMinionConfig"), k_param_LargeMinionConfig_Sequences("Sequences"),
    k_param_LargeMinionConfig_Runners("Runners"), k_paramGameplayTemplateIndex("GameplayTemplateIndex"), k_paramBackground("Background"),
    k_paramWaves("Waves"), k_paramLane("Lane"), k_paramNextWaveTime("nextWaveTime"), k_paramNumMinions("NumMinions"), k_paramSpeedPgt("SpeedPgt"),
    k_paramFirstRunnerDistance("firstRunnerDistance"), k_paramDistanceBetweenRunner("DistanceBetweenRunner");
static const float k_speedPgtScale = 0.01f;  // @0x572bcc (LoadWave: the speed factor's scale, as the runners')

// @0x4e5cfc. The large minion configuration (LoadLargeMinionConfig) comes with LargeMinionGameplay.
void LocationConfig::PostInit() {
    clara::Entity::PostInit();
    GetParam(k_param_MainMenuMusic, m_mainMenuMusic);
    GetParam(k_param_MainMenuAmbient, m_mainMenuAmbient);
    m_locationId = FindParamByName(k_param_LocationId)->GetAsIdEnum(0).value;
    GetParam(k_param_RenderPlatforms, m_renderPlatforms);
    m_fluffyBackground = FindParamByName(k_param_FluffyBackground)->GetAsEntity(0);
    m_cannonBackgrounds = FindParamByName(k_param_CannonBackgrounds)->GetAsEntity(0);
    m_cannonTemplates = FindParamByName(k_param_CannonTemplates)->GetAsEntity(0);
    bool hasBoss = false;
    GetParam(k_param_hasBoss, hasBoss);
    if (hasBoss) m_boss = clara::Project::GetInstance()->FindEntityByPath(FindParamByName(k_param_bossReference)->GetAsPath(0));
    LoadLargeMinionConfig();
}

// @0x57283c: the sequences of the mode, then its runners, from the location's large minion config
void LocationConfig::LoadLargeMinionConfig() {
    const clara::Param *cfgParam = FindParamByName(k_param_LargeMinionConfig);
    if (!cfgParam) return;
    clara::DataEntity *cfg = cfgParam->GetAsEntity(0);
    if (const clara::Param *seqs = cfg->FindParamByName(k_param_LargeMinionConfig_Sequences)) {
        for (unsigned c = 0; c < seqs->GetComponentCount(); c++) {
            m_largeMinionConfig.sequences.push_back(LargeMinionGameplay::LargeMinionGameplaySequence());
            LoadSequenceForLargeMinion(seqs->GetAsEntity(c), &m_largeMinionConfig.sequences.back());
        }
    }
    if (const clara::Param *runners = cfg->FindParamByName(k_param_LargeMinionConfig_Runners)) {
        for (unsigned c = 0; c < runners->GetComponentCount(); c++) {
            clara::DataEntity *r = clara::Project::GetInstance()->FindEntityByPath(runners->GetAsPath(c));
            if (r) m_largeMinionConfig.runners.push_back(static_cast<Runner*>(r));
        }
    }
}

// @0x572a00: a sequence: its template index, its background, and its waves
void LocationConfig::LoadSequenceForLargeMinion(clara::DataEntity *e, LargeMinionGameplay::LargeMinionGameplaySequence *seq) {
    e->GetParam(k_paramGameplayTemplateIndex, seq->gameplayTemplateIndex);
    if (const clara::Param *bg = e->FindParamByName(k_paramBackground)) seq->background = bg->GetAsPath(0);
    if (const clara::Param *waves = e->FindParamByName(k_paramWaves)) {
        for (unsigned c = 0; c < waves->GetComponentCount(); c++) {
            seq->waves.push_back(LargeMinionGameplay::LargeMinionGameplayWave());
            LoadWave(waves->GetAsEntity(c), &seq->waves.back());
        }
    }
}

// @0x572b08: a wave: its lane, its time to the next wave, its count of runners, the speed factor (scaled) and the distances along the path
void LocationConfig::LoadWave(clara::DataEntity *e, LargeMinionGameplay::LargeMinionGameplayWave *wave) {
    e->GetParam(k_paramLane, wave->lane);
    e->GetParam(k_paramNextWaveTime, wave->nextWaveTime);
    e->GetParam(k_paramNumMinions, wave->numMinions);
    e->GetParam(k_paramSpeedPgt, wave->speedPgt);
    wave->speedPgt.x *= k_speedPgtScale;
    wave->speedPgt.y *= k_speedPgtScale;
    e->GetParam(k_paramFirstRunnerDistance, wave->firstRunnerDistance);
    e->GetParam(k_paramDistanceBetweenRunner, wave->distanceBetweenRunner);
}

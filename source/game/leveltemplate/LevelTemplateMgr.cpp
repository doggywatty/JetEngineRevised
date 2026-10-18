// source/game/leveltemplate/LevelTemplateMgr.cpp
#include "game/leveltemplate/LevelTemplateMgr.h"
#include <cstdio>
#include <iterator>
#include <cstdlib>
#include <cstring>
#include "game/GameLevel.h"
#include "game/Statistics.h"
#include "game/entities/BackgroundData.h"
#include "game/entities/PlatformInstance.h"
#include "game/entities/PlatformTemplate.h"
#include "game/entities/TemplateData.h"
#include "game/gamemanagers/BackgroundMgr.h"
#include "game/leveltemplate/GameplayTemplateInstance.h"
#include "game/leveltemplate/LevelTemplateDef.h"
#include "clarafile.h"
#include "folder.h"
#include "project.h"
#include "camera/cameramgr.h"
#include "scene/camera.h"
#include "system.h"
#include "video/driver.h"
#include "game/entities/Minion.h"
#include "game/gamemanagers/MinionSpeedMgr.h"
#include "game/entities/Pacesetter.h"
#include "game/Game.h"
#include "game/CGameConfig.h"
#include "game/utils/ControlledRandom.h"
#include <cmath>

std::map<jet::String, LevelTemplateDef*> LevelTemplateMgr::s_levelTemplates, LevelTemplateMgr::s_backgroundTemplates;
bool LevelTemplateMgr::s_areLibrariesProcessed = false;
const jet::String LevelTemplateMgr::k__leftString("_left"), LevelTemplateMgr::k__rightString("_right"), LevelTemplateMgr::k__teeString("_tee"),
    LevelTemplateMgr::k__secretLeftString("_SecretLeft"), LevelTemplateMgr::k__secretRightString("_SecretRight"), LevelTemplateMgr::k__specialString("_Special"),
    LevelTemplateMgr::k__MacholabSpecialLeft("Macholab_Special_Left"), LevelTemplateMgr::k__MacholabSpecialRight("Macholab_Special_Right");

static jet::String TemplateKey(int id) {
    char b[20];
    snprintf(b, sizeof b, "%03d", id);
    return jet::String(b);
}

// the library name up to its first "_": "001_LAB" is template "001"
static jet::String TemplateNameOf(const jet::String &libName) {
    size_t u = libName.find_first_of('_', 0);
    return libName.substr(0, u);
}

static bool StartsWithDigit(const jet::String &s) { return s.size() && s.c_str()[0] >= '0' && s.c_str()[0] <= '9'; }

// @0x3e986c
void LevelTemplateMgr::LevelTemplateData::SetData(LevelTemplateSequenceProvider *p) {
    templateId = p->GetNextSequenceNumber();
    camera = p->GetNextCamera();
    cameraTransitionTime = p->GetNextCameraTransitionTime();
    speed = p->GetNextSpeed();
    location = p->GetNextLocation();
    bifurcation = LevelTemplateMgr::GetBifurcation(templateId);
    pause = LevelTemplateMgr::GetPause(templateId);
    nodeLabel = p->GetNextNodeLabel();
    nodeId = p->GetNextNodeId();
    nodeGraphId = p->GetNextNodeGraphId();
}

// @0x3e994c
LevelTemplateMgr::LevelTemplateMgr(LevelTemplateSequenceProvider *p)
    : m_provider(p), m_current(0), m_secretExitTaken(false), m_frameIndex(0), m_distance(0), m_cacheStarted(false), m_bifurcationBlocked(false), m_teeInCache(false), m_pausedCreation(false),
      m_pauseInCache(false), m_totalDistance(0), m_fastForward(false) {}

// @0x3e9bd4
LevelTemplateMgr::~LevelTemplateMgr() {
    for (LevelTemplateData *d : m_cache) delete d;
    for (GameplayTemplateInstance *g : m_instances) delete g;
}

// @0x3e9c90
void LevelTemplateMgr::BuildLevelTemplates(bool force) {
    if (s_areLibrariesProcessed && !force) return;
    clara::Project *prj = clara::Project::GetInstance();
    for (unsigned i = 0; i < prj->GetLibraryCount(); i++) {
        clara::ClaraFile *lib = prj->GetLibrary(i);
        const jet::String &name = lib->GetName();
        const clara::Folder &root = lib->GetRoot();
        if (StartsWithDigit(name)) {
            const jet::String key = TemplateNameOf(root.GetName());
            if (s_levelTemplates.find(key) != s_levelTemplates.end()) continue;
            LevelTemplateDef *def = new LevelTemplateDef;
            def->SetTemplateID((unsigned)atoi(key.c_str()));
            ProcessTemplateDefFolder(def, root);
            s_levelTemplates[key] = def;
        } else {
            const jet::String &prefix = BackgroundMgr::k_backgroundPrefix;
            if (prefix.size() && (!name.size() || !jet::detail::stristr(name.c_str(), prefix.c_str()))) continue;
            if (s_backgroundTemplates.find(root.GetName()) != s_backgroundTemplates.end()) continue;
            LevelTemplateDef *def = new LevelTemplateDef;
            ProcessTemplateDefFolder(def, root);
            s_backgroundTemplates[root.GetName()] = def;
        }
    }
    SetLevelTemplatesData();
    s_areLibrariesProcessed = true;
}

// @0x3e9f40
void LevelTemplateMgr::SetLevelTemplatesData() {
    for (auto &kv : s_levelTemplates) kv.second->ProcessTemplateData();
}

// @0x3e9fc8
void LevelTemplateMgr::CleanLevelTemplates(const std::vector<jet::String> &libraries) {
    for (const jet::String &lib : libraries) {
        std::map<jet::String, LevelTemplateDef*> &m = StartsWithDigit(lib) ? s_levelTemplates : s_backgroundTemplates;
        auto it = m.find(StartsWithDigit(lib) ? TemplateNameOf(lib) : lib);
        if (it == m.end()) continue;
        delete it->second;
        m.erase(it);
    }
}

// @0x3ea154
void LevelTemplateMgr::CleanLevelTemplates() {
    for (auto &kv : s_levelTemplates) delete kv.second;
    s_levelTemplates.clear();
    for (auto &kv : s_backgroundTemplates) delete kv.second;
    s_backgroundTemplates.clear();
    s_areLibrariesProcessed = false;
}

// @0x3eabb4
void LevelTemplateMgr::ProcessTemplateDefFolder(LevelTemplateDef *def, const clara::Folder &f) {
    for (unsigned i = 0; i < f.GetEntityCount(); i++) def->AddEntity(f.GetEntity(i));
    for (unsigned i = 0; i < f.GetFolderCount(); i++) ProcessTemplateDefFolder(def, f.GetFolder(i));
}

// @0x3ebb90, @0x3ebcf0, @0x3ebc4c
Bifurcation LevelTemplateMgr::GetBifurcation(int id) {
    auto it = s_levelTemplates.find(TemplateKey(id));
    return it == s_levelTemplates.end() ? Bifurcation_Center : it->second->GetTemplateData()->GetGroundPlatform(false)->GetDefinition()->GetBifurcation();
}
bool LevelTemplateMgr::GetPause(int id) {
    auto it = s_levelTemplates.find(TemplateKey(id));
    return it != s_levelTemplates.end() && it->second->IsPauseTemplate();
}
LevelTemplateDef *LevelTemplateMgr::GetLevelTemplateDef(int id) {
    auto it = s_levelTemplates.find(TemplateKey(id));
    return it == s_levelTemplates.end() ? 0 : it->second;
}

// @0x3ec2a8
int LevelTemplateMgr::GetBGLocation(const jet::String &name) {
    auto it = s_backgroundTemplates.find(BackgroundMgr::k_backgroundPrefix + name);
    return it == s_backgroundTemplates.end() ? -1 : it->second->GetBackgroundData()->GetLocation();
}

// @0x3dbb18
GameplayTemplateInstance *LevelTemplateMgr::BuildLevelPart(const jet::String &name) {
    auto it = s_levelTemplates.find(name);
    return it == s_levelTemplates.end() ? 0 : new GameplayTemplateInstance(it->second);
}

// @0x3da7c0: the next template of the sequence joins the cache of four
void LevelTemplateMgr::CacheLevelSequence() {
    if (!m_cacheStarted) m_cacheStarted = true;
    else m_provider->IncSequenceNumber();
    GameLevel::GetInstance()->UpdateUsedDynamicLibraries();
    LevelTemplateData *d;
    if (m_cache.size() < 4) d = new LevelTemplateData(m_provider);
    else {
        d = m_cache.front();
        d->SetData(m_provider);
        m_cache.pop_front();
    }
    if (d->bifurcation == Bifurcation_Tee) m_teeInCache = true;
    if (d->pause) m_pauseInCache = true;
    m_cache.push_back(d);
}

// @0x3da8b8: a template of the cache was made, the cache is refilled unless a tee or a pause stops it
void LevelTemplateMgr::ConsumeCacheLevelSequence() {
    if (!m_teeInCache && !m_pauseInCache) {
        CacheLevelSequence();
        return;
    }
    if (m_cache.empty()) return;
    delete m_cache.front();
    m_cache.pop_front();
    GameLevel::GetInstance()->UpdateUsedDynamicLibraries();
}

// @0x3da954
void LevelTemplateMgr::CacheInitialLevelSequence() {
    for (int i = 0; i < 4; i++)
        if (!m_teeInCache && !m_pauseInCache) CacheLevelSequence();
}

// @0x3da990
void LevelTemplateMgr::PopulateCacheSequenceBuffer() {
    for (size_t n = m_cache.size(); n < 4; n++)
        if (!m_teeInCache && !m_pauseInCache) CacheLevelSequence();
}

// @0x3da9f4
void LevelTemplateMgr::ResumeCreation() {
    if (!m_pausedCreation || !m_pauseInCache) return;
    m_pausedCreation = false;
    m_pauseInCache = false;
    PopulateCacheSequenceBuffer();
}

// @0x3daa1c
unsigned LevelTemplateMgr::AdvanceLevelSequence(unsigned distance) {
    unsigned covered = 0;
    for (;;) {
        LevelTemplateData *d = m_cache.front();
        LevelTemplateDef *def = s_levelTemplates[TemplateKey(d->templateId)];
        const float size = def->GetSizeY();
        const unsigned next = (size > 0 ? (unsigned)size : 0) + covered;
        if (distance < next) break;
        HistoryData h = { def, d->location };
        switch (def->GetGroundPlatform()->GetDefinition()->GetBifurcation()) {
        case Bifurcation_Left: h.location.append(k__leftString); break;
        case Bifurcation_Right: h.location.append(k__rightString); break;
        case Bifurcation_Tee: h.location.append(k__teeString); break;
        default: break;
        }
        m_history.push_back(h);
        CacheLevelSequence();
        covered = next;
    }
    return covered;
}

// @0x3dac30: the first template of the cache is made, placed at the end of the one before
GameplayTemplateInstance *LevelTemplateMgr::CreateGameplayTemplateInstance() {
    LevelTemplateData *d = m_cache.front();
    GameplayTemplateInstance *g = BuildLevelPart(TemplateKey(d->templateId));
    g->SetCamera(d->camera, d->cameraTransitionTime);
    g->SetSpeed(d->speed);
    g->SetCreationNodeLabel(d->nodeLabel);
    g->SetCreationNodeId(d->nodeId);
    g->SetCreationNodeGraphId(d->nodeGraphId);
    jet::String location = d->location;
    if (!g->HasSecretExit()) {
        switch (g->GetBifurcation()) {
        case Bifurcation_Left:
            if (!location.Equals(k__MacholabSpecialLeft)) location.append(k__leftString);
            break;
        case Bifurcation_Right:
            if (!location.Equals(k__MacholabSpecialRight)) location.append(k__rightString);
            break;
        case Bifurcation_Tee: location.append(k__teeString); break;
        default:
            if (m_secretExitTaken) {
                location.append(k__specialString);
                Statistics::GetInstance()->IncreaseStatistic(StatisticsSaveData::k_statScope_secreatAreaCount, 1);
                m_secretExitTaken = false;
            }
        }
    } else {
        const Bifurcation dir = g->GetSecretExitDirection();
        if (dir == Bifurcation_Left) location.append(k__secretLeftString);
        else if (dir == Bifurcation_Right) location.append(k__secretRightString);
    }
    g->SetLocation(location);
    g->SetTemplateID(d->templateId);
    HistoryData h = { g->GetDefinition(), g->GetLocation() };
    m_history.push_back(h);
    const Bifurcation bif = g->GetBifurcation();
    if (bif < Bifurcation_Tee) g->ResolveBifurcation(bif);
    else m_bifurcationBlocked = true;
    if (g->IsPauseTemplate()) m_pausedCreation = true;
    GameplayTemplateInstance *previous = m_current;
    if (m_instances.empty()) {
        if (m_current && (m_current->GetBifurcation() == Bifurcation_Tee || m_current->IsPauseTemplate())) {
            math::vec3<float> p(0, 0, 0);
            float a = 0;
            m_current->GetEndAnchor(p, a);
            g->SetStartAnchor(p, a);
        }
    } else {
        previous = m_instances.back();
        math::vec3<float> p(0, 0, 0);
        float a = 0;
        previous->GetEndAnchor(p, a);
        g->SetStartAnchor(p, a);
    }
    GameLevel *gl = GameLevel::GetInstance();
    g->SpawnEntities(gl->GetEntityToSpawnCount(), previous);
    gl->RemoveEntities();
    m_instances.push_back(g);
    const float size = g->GetDefinition()->GetSizeY();
    m_totalDistance += size > 0 ? (int)size : 0;
    g->SetDistanceToRemove((unsigned)m_totalDistance);
    ConsumeCacheLevelSequence();
    g->SetStartDistance(m_distance);
    m_distance += g->GetDefinition()->GetSizeY();
    return g;
}

// @0x3ebabc
void LevelTemplateMgr::OnCreateGameplayTemplateInstance() { BackgroundMgr::GetInstance()->OnCreateGameplayTemplateInstance(); }

// @0x3da254: the first templates are made, the first of them is the current one
void LevelTemplateMgr::InitLevelParts() {
    for (int i = 0; i < 4; i++) {
        CreateGameplayTemplateInstance();
        OnCreateGameplayTemplateInstance();
        if (m_bifurcationBlocked || m_pausedCreation) break;
    }
    m_current = m_instances.front();
    if (!m_current->IsEnabled()) m_current->SetEnabled(true);
    m_instances.pop_front();
}

// @0x3da2dc
void LevelTemplateMgr::CleanLevelParts() {
    if (m_current) m_current->DeleteEntities();
    delete m_current;
    m_current = 0;
    for (GameplayTemplateInstance *g : m_instances) {
        g->DeleteEntities();
        delete g;
    }
    m_instances.clear();
    m_history.clear();
    for (LevelTemplateData *d : m_cache) delete d;
    m_cache.clear();
}

// @0x3db08c
const jet::String &LevelTemplateMgr::GetLastBackgroundLocation() const { return m_instances.back()->GetLocation(); }

// @0x3db028: the last template made (the current one while the others are not)
void LevelTemplateMgr::GetNextBackgroundInfo(BackgroundInfo &info) const {
    GameplayTemplateInstance *g = m_instances.empty() ? m_current : m_instances.back();
    g->GetStartAnchor(info.position, info.angle);
    info.bifurcation = g->GetBifurcation();
    info.size = info.size2 = g->GetDefinition()->GetSizeY();
}

// @0x4c0638
bool UpdateTemplateVisibility(LevelTemplateInstance *inst, unsigned distance, float end, float prevEnd) {
    float fogNear = 0, fogFar = 120.0f;
    LevelTemplateDef *def = inst->GetDefinition();
    const math::vec3<float> &front = CameraMgr::GetInstance()->GetSceneCamera()->GetFrontVector();
    const math::vec3<float> &dir = inst->GetDirection();
    const float dot = front.x * dir.x + front.y * dir.y + front.z * dir.z;
    const float margin = dot >= 0.0f ? 5.0f : -5.0f;
    const bool behind = dot < 0.0f;
    const float d = (float)distance;
    jet::video::Color c;
    if (!inst->IsEnabled()) {
        if (def && def->GetBackgroundData()) def->UseFog(&c, &fogNear, &fogFar);
        if (prevEnd <= d + fogFar && (behind || d < margin + end)) {
            inst->SetEnabled(true);
            return true;
        }
        return false;
    }
    if (behind || d < margin + end) {
        if (def && def->GetBackgroundData()) def->UseFog(&c, &fogNear, &fogFar);
        if (prevEnd <= d + fogFar) return false;
    }
    inst->SetEnabled(false);
    return true;
}

// @0x3ea410
bool LevelTemplateMgr::UpdateVisibilities() {
    Minion *minion = GameLevel::GetInstance()->GetMinion();
    if (!minion) return false;
    const unsigned distance = minion->GetTraveledDistance();
    bool changed = false;
    GameplayTemplateInstance *prev = m_current;
    for (GameplayTemplateInstance *g : m_instances) {
        if (!g->SpawningFinished() || g->HasPendingPlatformInstances()) continue;
        changed |= UpdateTemplateVisibility(g, distance, (float)g->GetDistanceToRemove(), prev ? (float)prev->GetDistanceToRemove() : 0.0f);
        prev = g;
    }
    return changed;
}

// @0x45efa4: the current template is spawned, then the platforms' objects one at a time; with all done the next template is made (up to four
// ahead). In the action phase the cameras are checked, a conditional bifurcation is followed, and the run goes on to the next template when
// the minion (or its pacesetter) has left the current one. The minion's sledge change (CheckMinionStateChange) is checked every frame. (The speed
// of the template, ApplySpeedValue, joins with MinionSpeedMgr.)
bool LevelTemplateMgr::UpdateTemplateInstanceBuilding() {
    Game *game = Game::GetInstance();
    GameLevel *gl = GameLevel::GetInstance();
    if (game->IsInActionPhase()) gl->CheckCameraChange();
    gl->CheckMinionStateChange();
    const unsigned frame = jet::System::GetDriver()->GetFrameIdx();
    bool changed = false;
    if (frame != m_frameIndex) {
        if (m_current) {
            if (!m_current->SpawningFinished()) {
                m_current->SpawnEntities(gl->GetEntityToSpawnCount(), 0);
                gl->RemoveEntities();
            } else {
                m_current->UpdateDelayedPlatformInstances();
                gl->RemoveEntities();
                bool allFinished = true;
                GameplayTemplateInstance *prev = m_current;
                for (GameplayTemplateInstance *g : m_instances) {
                    if (!g->SpawningFinished()) {
                        g->SpawnEntities(gl->GetEntityToSpawnCount(), prev);
                        allFinished = false;
                        gl->RemoveEntities();
                        break;
                    }
                    if (g->HasPendingPlatformInstances()) allFinished = false;
                    g->UpdateDelayedPlatformInstances();
                    prev = g;
                    gl->RemoveEntities();
                }
                if (!m_bifurcationBlocked && !m_pausedCreation && allFinished && m_instances.size() < 4) {
                    const bool wasEmpty = m_instances.empty();
                    CreateGameplayTemplateInstance();
                    OnCreateGameplayTemplateInstance();
                    if (wasEmpty) {
                        gl->SetNextCameraChangeInfo();
                        gl->SetNextMinionStateChangeInfo();
                    }
                    changed = true;
                }
            }
        }
        m_frameIndex = frame;
    }
    Minion *minion = gl->GetMinion();
    if (!game->IsInActionPhase() || minion->GetStateFlag265()) return changed;
    changed |= UpdateConditionalBifurcationState();
    const math::vec3<float> minionPos = minion->GetPosition();
    const math::vec3<float> pacesetterPos = minion->GetPacesetter() ? minion->GetPacesetter()->GetPosition() : minionPos;
    if (!m_instances.empty() && !m_current->Contains(minionPos) && !m_current->Contains(pacesetterPos)) {
        changed |= AdvanceLevelTemplateList();
        m_current->ApplyGameplayValues();
        if (!m_instances.empty()) {
            gl->SetNextCameraChangeInfo();
            gl->SetNextMinionStateChangeInfo();
        }
    }
    if (!gl->IsFluffyMode()) minion->SetDistanceRun(m_current->ComputeRelativeDistance(minion->GetPosition()));
    return changed;
}

// @0x45ecf0: the template the minion left goes to the level's deletion (behind the distance the minion has run) and the first of the list
// is the current one. The minion's pacesetter, if it lost its own, is put back on the new template.
bool LevelTemplateMgr::AdvanceLevelTemplateList() {
    GameLevel *gl = GameLevel::GetInstance();
    Minion *minion = gl->GetMinion();
    if (!gl->IsFluffyMode()) {
        minion->SetDistanceBase(minion->GetDistanceBase() + (unsigned)(int)m_current->GetDefinition()->GetSizeY());
        MinionSpeedMgr::GetInstance()->OnAdvanceLevelTemplateList();  // @0x45ef30
        gl->OnAdvanceLevelTemplateList();
    }
    const int graphId = m_current->GetCreationNodeGraphId();
    const jet::String nodeId = m_current->GetCreationNodeId();
    const unsigned total = minion->GetDistanceRun() + minion->GetDistanceBase();
    const PlatformToggles &t = CGameConfig::Current();
    m_current->SetPendingToDelete(m_fastForward ? (unsigned)(int)(t.templateRemoveDistance + t.templateRemoveDistanceFastForward) + total
                                                : (unsigned)(int)t.templateRemoveDistance + total);
    Pacesetter *hadPacesetter = minion->GetPacesetter();
    gl->OnLevelTemplateInstanceDelete(m_current);
    delete m_current;
    m_current = 0;
    bool created = false;
    if (!m_bifurcationBlocked && !m_pausedCreation) {
        CreateGameplayTemplateInstance();
        created = true;
    }
    m_current = m_instances.front();
    if (hadPacesetter && !minion->GetPacesetter()) {
        minion->SetPacesetter(gl->GetPacesetter());
        gl->GetPacesetter()->SetCurrentGameplayTemplateInstance(m_current);
        gl->GetPacesetter()->SetLanePosition(minion->GetPosition(), 0, (LaneIndex)3, 0);
    }
    if (m_current->GetCreationNodeGraphId() != graphId || !m_current->GetCreationNodeId().Equals(nodeId)) m_provider->OnNodeCompleted(graphId, nodeId);
    if (!m_current->IsEnabled()) m_current->SetEnabled(true);
    m_instances.pop_front();
    if (created) OnCreateGameplayTemplateInstance();
    else gl->UpdateUsedDynamicLibraries();
    return created;
}

// @0x45f390: the tee waits for the minion's lane: once it is past the limit of the left or the right branch the way is taken, the
// sequence goes on after it and the template of a secret exit may follow
bool LevelTemplateMgr::UpdateConditionalBifurcationState() {
    if (!m_bifurcationBlocked || !m_instances.empty() || m_current->IsBifurcationDecided() || m_current->GetBifurcation() != Bifurcation_Tee) return false;
    GameLevel *gl = GameLevel::GetInstance();
    Minion *minion = gl->GetMinion();
    LaneIndex lane;
    unsigned node;
    m_current->GetClosestLanePosition(minion->GetPosition(), lane, node);
    unsigned leftLimit, rightLimit;
    m_current->GetDefinition()->GetTemplateData()->GetGroundPlatform(false)->GetDefinition()->GetTeeLimits(leftLimit, rightLimit);
    if (!minion->IsInvincible()) {
        if (lane == 2 && node >= rightLimit) m_current->ResolveBifurcation(2);
        else if (node >= leftLimit) {
            lane = (LaneIndex)0;
            m_current->ResolveBifurcation(1);
        }
    }
    if (!m_current->IsBifurcationDecided()) return false;
    m_current->SendGameplayTemplateInstanceEvent({ GameplayTemplateInstanceEvent::BifurcationResolved });
    m_teeInCache = false;
    CacheInitialLevelSequence();
    minion->OnBifurcationResolved(lane);
    if (m_current->HasSecretExit() && m_current->GetFinalBifurcation() == (int)m_current->GetSecretExitDirection()) {
        const std::vector<unsigned> &secret = m_current->GetSecretExitLevelTemplates();
        IncludeInLevelSequenceFront((int)secret[(size_t)GameLevel::s_randomizer[0]->Rand(0, (int)secret.size())]);
        m_secretExitTaken = true;
    }
    m_current->SpawnEntities(gl->GetEntityToSpawnCount(), 0);
    gl->RemoveEntities();
    m_bifurcationBlocked = false;
    m_instances.push_back(m_current);
    CreateGameplayTemplateInstance();
    OnCreateGameplayTemplateInstance();
    m_instances.pop_front();
    return true;
}

// @0x45e3c0: a template of the sequence made ahead of the cache
void LevelTemplateMgr::IncludeInLevelSequenceFront(int templateId) {
    LevelTemplateData *d = new LevelTemplateData(*m_cache.front());
    d->templateId = templateId;
    d->bifurcation = GetBifurcation(templateId);
    d->pause = GetPause(templateId);
    m_cache.push_front(d);
}

// @0x45e150
bool LevelTemplateMgr::CanCheckOriginDistance() const {
    GameLevel *gl = GameLevel::GetInstance();
    if (!GameLevel::s_gameplay || !Game::GetInstance()->IsInActionPhase()) return false;
    const int mode = gl->GetGameplayMode();
    if (mode == 1 || mode == 3) return false;
    Minion *minion = gl->GetMinion();
    if (!minion || minion->IsLinked() || !minion->IsOnGround() || !m_current || !m_current->SpawningFinished()) return false;
    for (GameplayTemplateInstance *g : m_instances)
        if (!g->SpawningFinished()) return false;
    return true;
}

const float LevelTemplateMgr::k_originDistanceLimit = 3500.0f;  // @0xc89784

// @0x45e264: far from the origin the floats lose their precision: the level moves back by whole meters, the camera follows at once
void LevelTemplateMgr::CheckOriginDistance() {
    GameLevel *gl = GameLevel::GetInstance();
    const math::vec3<float> pos = gl->GetMinion()->GetPosition();
    if (!(pos.x > k_originDistanceLimit) && !(pos.y > k_originDistanceLimit)) return;
    math::vec3<float> d(std::floor(0.5f - pos.x), std::floor(0.5f - pos.y), 0);
    if (d.getLength() == 0.0f) return;
    d.x += 10.0f;
    m_current->Translate(d);
    for (GameplayTemplateInstance *g : m_instances) g->Translate(d);
    gl->Translate(d);
    BackgroundMgr::GetInstance()->Translate(d);
    // EffectMgr::Translate @0x59ce84 joins with the effects
    CameraMgr::GetInstance()->Update(99999);
}

// @0x45f984
GameplayTemplateInstance *LevelTemplateMgr::GetPredictedTransform(unsigned distance, math::vec3<float> &pos, math::quat<float> &rot) const {
    unsigned start = GameLevel::GetInstance()->GetMinion()->GetDistanceBase();
    unsigned end = start + (unsigned)m_current->GetDefinition()->GetSizeY();
    GameplayTemplateInstance *found = 0;
    if (end >= distance) found = m_current;
    else {
        start = end;
        for (GameplayTemplateInstance *g : m_instances) {
            end = start + (unsigned)g->GetDefinition()->GetSizeY();
            if (end >= distance) {
                found = g;
                break;
            }
            start = end;
        }
    }
    if (found && !found->GetPredictedTransform(distance - start, pos, rot)) found = 0;
    return found;
}

// @0x45fa60
GameplayTemplateInstance *LevelTemplateMgr::GetClosestLaneTransform(const math::vec3<float> &p, math::vec3<float> &pos, math::quat<float> &rot) const {
    if (m_current->Contains(p)) {
        m_current->GetClosestLanePosition(p, pos, rot);
        return m_current;
    }
    for (GameplayTemplateInstance *g : m_instances)
        if (g->Contains(p)) {
            g->GetClosestLanePosition(p, pos, rot);
            return g;
        }
    return 0;
}

// @0x45fd54
void LevelTemplateMgr::GetLastTemplateEndAnchor(math::vec3<float> &pos, float &angle) {
    (m_instances.empty() ? m_current : m_instances.back())->GetEndAnchor(pos, angle);
}

// @0x45e020: the templates are made and the run goes on; only when neither that nor the visibility changed anything may the level move
// back to the origin
void LevelTemplateMgr::UpdateLevelParts() {
    const bool built = UpdateTemplateInstanceBuilding();
    const bool shown = UpdateVisibilities();
    if (!built && !shown && CanCheckOriginDistance()) CheckOriginDistance();
}

// @0x3ebd9c
GameplayTemplateInstance *LevelTemplateMgr::GetNextLevelTemplateInstance(GameplayTemplateInstance *g) const {
    if (g == m_current) return m_instances.empty() ? 0 : m_instances.front();
    for (auto it = m_instances.begin(); it != m_instances.end(); ++it)
        if (*it == g) return g == m_instances.back() ? 0 : *std::next(it);
    return 0;
}

// @0x3ebe0c
GameplayTemplateInstance *LevelTemplateMgr::GetPreviousLevelTemplateInstance(GameplayTemplateInstance *g) const {
    if (g == m_current) return 0;
    if (!m_instances.empty() && g == m_instances.front()) return m_current;
    for (auto it = m_instances.begin(); it != m_instances.end(); ++it)
        if (*it == g) return *std::prev(it);
    return 0;
}

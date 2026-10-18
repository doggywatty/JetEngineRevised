// source/game/gamemanagers/BackgroundMgr.cpp
#include "BackgroundMgr.h"
#include <cmath>
#include "game/GameLevel.h"
#include "game/Game.h"
#include "game/Player.h"
#include "game/entities/BackgroundData.h"
#include "game/entities/LocationConfig.h"
#include "game/entities/Minion.h"
#include "game/entities/ParticleSystemEntity.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/leveltemplate/BackgroundTemplateInstance.h"
#include "game/leveltemplate/LevelTemplateMgr.h"
#include "camera/cameramgr.h"
#include "core/util.h"
#include "clarafile.h"
#include "project.h"
#include "scene/camera.h"
#include "system.h"
#include "video/driver.h"

const jet::String BackgroundMgr::k_backgroundPrefix("Background_"), BackgroundMgr::k_leftString("left"), BackgroundMgr::k_rightString("right"),
    BackgroundMgr::k_teeString("tee"), BackgroundMgr::k_emptyString("");
const float BackgroundMgr::k_backgroundTemplateSize = 180.0f, BackgroundMgr::k_safeBackgroundChangeOffset = 20.0f, BackgroundMgr::k_hidenPositionOffset = 500.0f;
const unsigned BackgroundMgr::k_backgroundCount = 10;

static const float k_degToHalfRad = 0.0087266462f;
static const float k_defaultFogFar = 120.0f;

static unsigned FrameIndex() { return jet::System::GetDriver()->GetFrameIdx(); }
static unsigned MinionDistance() { return GameLevel::GetInstance()->GetMinion()->GetTraveledDistance(); }
static bool Has(const jet::String &s, const jet::String &sub) { return s.find(sub, 0) != (size_t)-1; }
static bool IsTurnName(const jet::String &s) { return Has(s, BackgroundMgr::k_leftString) || Has(s, BackgroundMgr::k_rightString) || Has(s, BackgroundMgr::k_teeString); }
static unsigned PositiveMeters(float f) { return f > 0.0f ? (unsigned)f : 0; }

// @0x4c0880: the particles of the entity's links
static void SetParticleEnabled(GameEntity *e, bool on) {
    for (unsigned i = 0; i < e->GetLinkedEntitiesCount(); i++) {
        clara::Entity *l = e->GetLinkedEntity(i);
        if (dynamic_cast<ParticleSystemEntity*>(l)) {
            l->SetEnabled(on);
            l->SetVisible(on);
        } else
            SetParticleEnabled(static_cast<GameEntity*>(l), on);
    }
}

// @0x4c0900
static void SetParticlesEnabled(BackgroundTemplateInstance *bg, bool on) {
    for (LevelTemplateInstance::Instance &i : bg->GetCreatedEntities()) {
        if (dynamic_cast<ParticleSystemEntity*>(i.entity)) {
            i.entity->SetEnabled(on);
            i.entity->SetVisible(on);
        } else
            SetParticleEnabled(i.entity, on);
    }
}

// @0x4bfd70
BackgroundMgr::BackgroundMgr(bool enabled)
    : m_changeOffset(0), m_checkOffset(0), m_currentList(&m_lists[0]), m_lastApplied(0), m_location(Player::GetInstance()->GetCurrentLocation()), m_currentLocation(0),
      m_fluffyBackgrounds(0), m_createdSizeBackup(0), m_createdSize(0), m_updateFrame(FrameIndex()), m_dropFrame(FrameIndex()), m_enableBackground(enabled) {
    for (BackgroundList &l : m_lists) l.origin = math::vec3<float>(0, 0, 0);
}

// @0x4c0114
BackgroundMgr::~BackgroundMgr() {
    for (BackgroundList &l : m_lists)
        for (BackgroundTemplateInstance *b : l.items) DropBG(b);
    for (GameEntity *e : m_bgEntities) delete e;
}

// @0x4c02bc: the fluffy dimension lies where the fluffy gameplay is placed
void BackgroundMgr::PostInit() {
    m_currentLocation = -1;
    m_lists[0].origin = math::vec3<float>(0, 0, 0);
    m_location = Player::GetInstance()->GetCurrentLocation();
    GameLevel *l = GameLevel::GetInstance();
    if (GameEntity *f = dynamic_cast<GameEntity*>(l->GetFluffyGameplay())) m_lists[1].origin = f->GetPosition();
    BuildBackgroundPool();
}

// @0x4c0364
void BackgroundMgr::InitBackgrounds() {
    m_changeOffset = k_backgroundCount < 3 ? k_safeBackgroundChangeOffset : 0.0f;
    m_checkOffset = (int)std::floor((float)k_backgroundCount);
    ResetBackground();
}

// @0x4c04f0
void BackgroundMgr::ResetBackground() {
    for (BackgroundTemplateInstance *b : m_currentList->items) DropBG(b);
    m_currentList->items.clear();
    m_createdSize = 0;
}

// the fluffy gameplay's pick of a background of the location (@0x4d9b14, @0x4d9b4c)
jet::String BackgroundMgr::NextFluffyBackgroundName() const {
    const clara::Param *p = m_fluffyBackgrounds->FindParamByName(jet::String("FluffyBackgrounds"));
    clara::DataEntity *bg = clara::Project::GetInstance()->FindEntityByPath(p->GetAsPath(jet::core::Rand(0, (int)p->GetComponentCount())));
    return static_cast<BackgroundData*>(bg)->GetFileName();
}

// @0x4c03cc: three backgrounds of the fluffy dimension one above the other
void BackgroundMgr::ResetFluffyBackground() {
    for (BackgroundTemplateInstance *b : m_currentList->items) DropBG(b);
    m_currentList->items.clear();
    float offset = 0;
    for (int i = 0; i < 3; i++) {
        BackgroundTemplateInstance *bg = GetNextBG(NextFluffyBackgroundName());
        const math::vec3<float> p(m_currentList->origin.x, m_currentList->origin.y, offset + m_currentList->origin.z);
        bg->SetStartAnchor(p, 0);
        bg->Reposition(p, 0);
        m_currentList->items.push_back(bg);
        offset += k_backgroundTemplateSize;
        GrabBG(bg);
    }
}

// @0x4c0638 is above; the bg entities that were made join the manager's
// @0x4c05b8
void BackgroundMgr::ProcessCreatedEntities(BackgroundTemplateInstance *bg) {
    for (LevelTemplateInstance::Instance &i : bg->GetCreatedEntities()) {
        if (i.flag) continue;
        i.entity->SetPersistenceType(0);
        i.entity->RegisterForUpdate(false);
        i.entity->RegisterForRender(false);
        m_bgEntities.push_back(i.entity);
        i.flag = true;
    }
}

// @0x4c053c
void BackgroundMgr::Update() {
    if (!m_enableBackground) return;
    UpdateBackgroundPool();
    CheckSettingsChange();
    if (GameLevel::GetInstance()->IsFluffyMode()) return;
    DropBackgrounds(MinionDistance());
}

// @0x4c0970
void BackgroundMgr::UpdateBackgroundPool() {
    if (!m_enableBackground) return;
    GameLevel *gl = GameLevel::GetInstance();
    unsigned frame = FrameIndex();
    if (frame != m_updateFrame) {
        for (BackgroundTemplateInstance *bg : m_currentList->items) {
            if (bg->SpawningFinished()) continue;
            bg->SpawnEntities(gl->GetEntityToSpawnCount(), 0);
            ProcessCreatedEntities(bg);
            gl->RemoveEntities();
            if (bg->SpawningFinished()) SetParticlesEnabled(bg, true);
        }
        m_updateFrame = frame;
    }
    if (gl->IsFluffyMode()) {
        UpdateFluffyBackgroundPool();
        return;
    }
    const unsigned distance = MinionDistance();
    BackgroundTemplateInstance *prev = 0;
    for (BackgroundTemplateInstance *bg : m_currentList->items) {
        UpdateTemplateVisibility(bg, distance, (float)bg->GetDistanceToRemove(), prev ? (float)prev->GetDistanceToRemove() : 0.0f);
        prev = bg;
    }
    for (BackgroundTemplateInstance *bg : m_currentList->items)
        if (bg->IsEnabled())
            for (LevelTemplateInstance::Instance &i : bg->GetCreatedEntities()) i.entity->Update();
}

// @0x4c0b64: the settings of the next background apply once the minion gets to it
void BackgroundMgr::CheckSettingsChange() {
    std::list<BackgroundTemplateInstance*> &l = m_currentList->items;
    for (auto it = l.begin(); it != l.end(); ++it) {
        if (*it != m_lastApplied) continue;
        auto next = std::next(it);
        if (next != l.end() && ShouldApplySettings(*next)) ApplyBackgroundSettings(*next);
        return;
    }
    for (BackgroundTemplateInstance *bg : l)
        if (ShouldApplySettings(bg)) {
            ApplyBackgroundSettings(bg);
            return;
        }
}

// @0x4c0c04
bool BackgroundMgr::ShouldApplySettings(const BackgroundTemplateInstance *bg) const {
    return m_lastApplied != bg && (float)bg->GetDistanceToApplySettings() < (float)MinionDistance();
}

// @0x4c0c68
int BackgroundMgr::GetLocationAtDistance(unsigned distance) const {
    const std::list<BackgroundTemplateInstance*> &l = m_currentList->items;
    for (auto it = l.rbegin(); it != l.rend(); ++it)
        if (distance > (*it)->GetDistanceToApplySettings()) return (*it)->GetDefinition()->GetBackgroundData()->GetLocation();
    return -1;
}

// @0x4c0ca8
void BackgroundMgr::UpdateFluffyBackgroundPool() {
    GameLevel *gl = GameLevel::GetInstance();
    const float z = gl->GetMinion()->GetPosition().z;
    BackgroundTemplateInstance *bg = 0;
    for (BackgroundTemplateInstance *b : m_currentList->items) {
        bg = b;
        if (z < (float)b->GetDistanceToRemove()) break;
    }
    math::vec3<float> start(0, 0, 0);
    float angle;
    bg->GetStartAnchor(start, angle);
    if (start.z + m_changeOffset < z) {
        const float nextZ = start.z + k_backgroundTemplateSize;
        BackgroundTemplateInstance *fresh = GetNextBG(NextFluffyBackgroundName());
        DropBG(m_currentList->items.front());
        GrabBG(fresh);
        const math::vec3<float> p(m_currentList->origin.x, m_currentList->origin.y, nextZ + m_currentList->origin.z);
        fresh->SetStartAnchor(p, 0);
        fresh->Reposition(p, 0);
        m_currentList->items.pop_front();
        m_currentList->items.push_back(fresh);
    }
    for (BackgroundTemplateInstance *b : m_currentList->items)
        for (LevelTemplateInstance::Instance &i : b->GetCreatedEntities()) i.entity->Update();
}

// @0x4c0e6c: the first background goes once the minion is 120 meters past its end
void BackgroundMgr::DropBackgrounds(unsigned distance) {
    if (!m_enableBackground) return;
    unsigned frame = FrameIndex();
    if (frame == m_dropFrame) return;
    m_dropFrame = frame;
    std::list<BackgroundTemplateInstance*> &l = m_currentList->items;
    if (l.empty() || distance < l.front()->GetDistanceToRemove() + 0x78) return;
    DropBG(l.front());
    l.pop_front();
}

// @0x4c0f04
void BackgroundMgr::OnCreateGameplayTemplateInstance() {
    if (!m_enableBackground) return;
    GameLevel *gl = GameLevel::GetInstance();
    LevelTemplateMgr *ltm = gl->GetLevelTemplateMgr();
    std::list<BackgroundTemplateInstance*> &list = m_currentList->items;
    const std::vector<LevelTemplateMgr::HistoryData> &history = ltm->GetHistory();
    if (list.empty() && history.size() >= 2) {
        // the first background: the meters since the last turn in the run are what is left of its segment
        unsigned meters = 0;
        for (size_t i = history.size(); i > 0; i--) {
            const LevelTemplateMgr::HistoryData &h = history[i - 1];
            if (IsTurnName(h.location)) break;
            meters += PositiveMeters(h.def->GetSizeY());
        }
        const unsigned rem = meters % 180;
        BackgroundTemplateInstance *bg = GetNextBG(GetBGName(history.back().location));
        list.push_back(bg);
        GrabBG(bg);
        const math::vec3<float> p((float)(int)rem, 0, 0);
        bg->SetStartAnchor(p, 0);
        bg->Reposition(p, 0);
        m_createdSize = rem;
        const unsigned apply = Game::GetInstance()->GetStartDistance() - rem;
        bg->SetDistanceToApplySettings(apply);
        bg->SetDistanceToRemove(apply + 180);
        return;
    }
    const LevelTemplateMgr::HistoryData &last = history.back();
    const bool turn = IsTurnName(last.location);
    unsigned startNew = turn ? 1 : (list.empty() ? 1 : 0);
    const unsigned size = PositiveMeters(last.def->GetSizeY());
    if (m_createdSize + size > 180) startNew |= 1;
    bool lastIsTurn = false;
    if (!list.empty()) {
        const clara::DataEntity *bd = list.back()->GetDefinition()->GetBackgroundData();
        lastIsTurn = IsTurnName(bd->GetName());
    }
    if (!startNew && !lastIsTurn) {
        m_createdSize += size;
        return;
    }
    m_createdSize = size;
    LevelTemplateMgr::BackgroundInfo info;
    info.position = math::vec3<float>(0, 0, 0);
    info.angle = 0;
    info.bifurcation = 0;
    const unsigned base = list.empty() ? 0 : list.back()->GetDistanceToRemove();
    ltm->GetNextBackgroundInfo(info);
    const jet::String name = GetBGName(last.location);
    const unsigned segment = turn ? 60 : 180;
    unsigned count = size / segment;
    if (!count) count = 1;
    const float half = info.angle * k_degToHalfRad;
    const math::quat<float> q(0, 0, std::sin(half), std::cos(half));
    unsigned offset = 0;
    for (unsigned k = 0; k < count; k++) {
        BackgroundTemplateInstance *bg = GetNextBG(name);
        list.push_back(bg);
        GrabBG(bg);
        const math::vec3<float> p = info.position + math::transform(q.getAsMat3(), math::vec3<float>((float)offset, 0, 0));
        bg->SetStartAnchor(p, info.angle);
        bg->Reposition(p, info.angle);
        math::vec3<float> d = math::transform(q.getAsMat3(), math::vec3<float>(1, 0, 0));
        d.normalize();
        bg->SetDirection(d);
        bg->SetDistanceToApplySettings(offset + base);
        bg->SetDistanceToRemove(segment + base + offset);
        offset += segment;
    }
}

// @0x4c1498
void BackgroundMgr::BackUp(std::vector<BackupBackground> &out) {
    out.clear();
    for (BackgroundTemplateInstance *bg : m_currentList->items) {
        BackupBackground b;
        b.applyDistance = bg->GetDistanceToApplySettings();
        b.removeDistance = bg->GetDistanceToRemove();
        bg->GetStartAnchor(b.position, b.angle);
        b.direction = bg->GetDirection();
        b.name = GetBGName(bg);
        out.push_back(b);
    }
    m_createdSizeBackup = m_createdSize;
}

// @0x4c15b0
void BackgroundMgr::Restore(std::vector<BackupBackground> &in) {
    ResetBackground();
    m_createdSize = m_createdSizeBackup;
    for (const BackupBackground &b : in) {
        BackgroundTemplateInstance *bg = GetNextBG(b.name);
        bg->SetStartAnchor(b.position, b.angle);
        bg->Reposition(b.position, b.angle);
        bg->SetDirection(b.direction);
        bg->SetDistanceToApplySettings(b.applyDistance);
        bg->SetDistanceToRemove(b.removeDistance);
        m_currentList->items.push_back(bg);
        bg->SpawnEntities(-1, 0);
        ProcessCreatedEntities(bg);
        GameLevel::GetInstance()->RemoveEntities();
    }
}

// @0x4c16d4: the background library's file name
jet::String BackgroundMgr::GetBGName(const BackgroundTemplateInstance *bg) const {
    return bg->GetDefinition()->GetBackgroundData()->GetClaraFile()->GetName();
}

// @0x4c1730: a hidden background from the library of that name
BackgroundTemplateInstance *BackgroundMgr::GetNextBG(const jet::String &name) {
    LevelTemplateDef *def = LevelTemplateMgr::GetBackgroundTemplates().at(name);
    BackgroundTemplateInstance *bg = new BackgroundTemplateInstance(def);
    GameLevel *gl = GameLevel::GetInstance();
    bg->SpawnEntities(gl->GetEntityToSpawnCount(), 0);
    gl->RemoveEntities();
    bg->Translate(math::vec3<float>(-k_hidenPositionOffset, 0, 0));
    ProcessCreatedEntities(bg);
    return bg;
}

// @0x4c1894: the entities go with it
void BackgroundMgr::DropBG(BackgroundTemplateInstance *bg) {
    if (!m_enableBackground) return;
    bg->EnableTouchInteracts(false);
    SetParticlesEnabled(bg, false);
    bg->SetEnabled(false);
    for (LevelTemplateInstance::Instance &i : bg->GetCreatedEntities()) {
        m_bgEntities.remove(i.entity);
        delete i.entity;
    }
    delete bg;
}

// @0x4c1954
void BackgroundMgr::GrabBG(BackgroundTemplateInstance *bg) {
    if (!m_enableBackground) return;
    bg->EnableTouchInteracts(true);
    SetParticlesEnabled(bg, true);
    bg->SetEnabled(true);
}

// @0x4c1a44
void BackgroundMgr::Render() {
    if (!m_enableBackground) return;
    for (BackgroundTemplateInstance *bg : m_currentList->items)
        if (bg->IsEnabled())
            for (LevelTemplateInstance::Instance &i : bg->GetCreatedEntities()) i.entity->Render();
}

// @0x4c1c90
void BackgroundMgr::Translate(const math::vec3<float> &d) {
    if (!m_enableBackground) return;
    for (BackgroundTemplateInstance *bg : m_currentList->items) bg->Translate(d);
}

// @0x4c1cd4
void BackgroundMgr::EnableFluffyDimensionBackground(bool on) {
    if (!m_enableBackground) return;
    if (on) {
        LocationConfig *c = LocationsMgr::GetInstance()->GetLocationConfig(m_location);
        if (c) m_fluffyBackgrounds = c->GetFluffyBackground();
        m_currentList = &m_lists[1];
        ResetFluffyBackground();
    } else
        m_currentList = &m_lists[0];
    ApplyBackgroundSettings(m_currentList->items.front());
}

// @0x4c1d48
void BackgroundMgr::EnableLargeMinionBackground(bool on) {
    if (!m_enableBackground) return;
    if (on) {
        BackUp(m_backup);
        ResetBackground();
        m_currentList = &m_lists[2];
        return;
    }
    ResetBackground();
    m_currentList = &m_lists[0];
    Restore(m_backup);
    m_backup.clear();
    if (!m_currentList->items.empty()) ApplyBackgroundSettings(m_currentList->items.front());
}

// @0x4c1dec
int BackgroundMgr::GetLocationOfCurrentBackground() const {
    return m_currentList->items.front()->GetDefinition()->GetBackgroundData()->GetLocation();
}

// @0x4c1e0c: what the background brings applies, and the location it is in becomes the player's
void BackgroundMgr::ApplyBackgroundSettings(const BackgroundTemplateInstance *bg) {
    bg->ApplyPostFxValue();
    bg->ApplyFogValue();
    bg->ApplySoundSettings();
    bg->ApplyRocketPath();
    bg->ApplyLensFlare();
    bg->ApplySkyDome();
    const int previous = m_currentLocation;
    m_location = m_currentLocation = bg->GetDefinition()->GetBackgroundData()->GetLocation();
    if (m_location == -1) m_location = m_currentLocation = 0;
    if (previous != m_currentLocation) GameLevel::GetInstance()->OnLocationChanged(previous, m_currentLocation);
    m_lastApplied = bg;
}

void BackgroundMgr::ApplySkyDome() { m_currentList->items.front()->ApplySkyDome(); }
void BackgroundMgr::ApplyFogValue() { m_currentList->items.front()->ApplyFogValue(); }
// @0x542d90
void BackgroundMgr::ApplyLensFlare() { m_currentList->items.front()->ApplyLensFlare(); }

// @0x4c1eec
void BackgroundMgr::ApplyCurrentBackgroundSound() {
    if (m_currentList && !m_currentList->items.empty()) m_currentList->items.front()->ApplySoundSettings();
}

// @0x4c1f0c: the title of the background the minion is in
const jet::String &BackgroundMgr::GetCurrentLocationTitle() const {
    if (m_currentList) {
        const unsigned distance = MinionDistance();
        for (BackgroundTemplateInstance *bg : m_currentList->items)
            if (bg && distance < bg->GetDistanceToRemove()) return bg->GetDefinition()->GetBackgroundData()->GetTitle();
    }
    return k_emptyString;
}

// @0x4c1f98
std::vector<int> BackgroundMgr::GetLocationPoolArray() const {
    std::vector<int> v;
    for (BackgroundTemplateInstance *bg : m_currentList->items) v.push_back(bg->GetDefinition()->GetBackgroundData()->GetLocation());
    return v;
}

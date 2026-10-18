// source/game/leveltemplate/GameplayTemplateInstance.cpp
#include "game/leveltemplate/GameplayTemplateInstance.h"
#include <cfloat>
#include <cstdio>
#include "game/GameLevel.h"
#include "game/Game.h"
#include "game/CGameConfig.h"
#include "game/entities/Deco3d.h"
#include "game/entities/Minion.h"
#include "game/gamemanagers/MinionSpeedMgr.h"
#include "game/entities/Object.h"
#include "game/gamestates/GS_Gameplay.h"
#include "game/entities/PlatformInstance.h"
#include "game/entities/TemplateData.h"
#include "game/entities/camera/CameraEntity.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/leveltemplate/LevelTemplateMgr.h"
#include "game/entities/LocationConfig.h"
#include "game/utils/ControlledRandom.h"
#include "scene/model.h"
#include "scene/modelloader.h"
#include "scene/scenemgr.h"

const jet::String GameplayTemplateInstance::k_emptyStr("");
unsigned GameplayTemplateInstance::s_entityGenerationGroupSequentialId = 0;

// @0x3e0840
GameplayTemplateInstance::GameplayTemplateInstance(LevelTemplateDef *def)
    : LevelTemplateInstance(def), m_id(0), m_startDistance(0), m_camera(0), m_cameraTransitionTime(0), m_speed(0), m_groundObject(0), m_numTotalDummies(0),
      m_entityGenerationGroupId(0), m_creationNodeGraphId(0) {
    InitEntityGenerationGroup();
}

// @0x3e0a18
GameplayTemplateInstance::~GameplayTemplateInstance() {
    m_id = -1;
    m_groundObject = 0;
    SendGameplayTemplateInstanceEvent({ GameplayTemplateInstanceEvent::Destroyed });
    for (LaneInterval *i : m_laneForbiddenMovements) delete i;
    ClearEntityGenerationGroup();
    delete m_speed;
}

// @0x3e2860: the entity and what is linked to it
static void SetObjectEnabled(clara::Entity *e, bool on) {
    if (!e) return;
    e->SetEnabled(on);
    e->SetVisible(on);
    for (unsigned i = 0; i < e->GetLinkedEntitiesCount(); i++) SetObjectEnabled(e->GetLinkedEntity(i), on);
}

// @0x3e28c4
void GameplayTemplateInstance::SetEnabled(bool b) {
    m_isEnabled = b;
    for (Instance &i : m_createdEntities) SetObjectEnabled(i.entity, b);
}

// @0x3e2900
void GameplayTemplateInstance::SendGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent &e) {
    for (GameplayTemplateInstanceListener *l : std::set<GameplayTemplateInstanceListener*>(m_listeners)) l->OnGameplayTemplateInstanceEvent(e);
}

// @0x3e203c
void GameplayTemplateInstance::SetSpeed(const SpeedDef *s) {
    if (!s) return;
    if (!m_speed) m_speed = new SpeedDef;
    m_speed->speed = s->speed;
    m_speed->isSet = true;
}

// @0x3e0f54
CameraController *GameplayTemplateInstance::GetCameraController() const {
    return m_camera ? static_cast<CameraEntity*>(m_camera)->GetCameraController() : 0;
}

// @0x454da4
void GameplayTemplateInstance::ApplyCameraValue(int ms) {
    if (!m_camera) return;
    const unsigned time = (unsigned)(ms < 0 ? m_cameraTransitionTime : ms);
    GameLevel *gl = GameLevel::GetInstance();
    gl->SetCameraRelativeToMinionSpeed(GetCameraController(), time, Minion::k_referenceSpeed);
}

// @0x454e10
void GameplayTemplateInstance::ApplySpeedValue() {
    if (!m_speed || !m_speed->isSet) return;
    if (m_speed->speed > 0)
        MinionSpeedMgr::GetInstance()->SetSpeedForcedByScript(true, m_speed->speed);
    else
        MinionSpeedMgr::GetInstance()->SetSpeedForcedByScript(false, 0);
}

// @0x455e44 (GameLevel::InitLocationSpecificData, which it ends with, does nothing in the game)
void GameplayTemplateInstance::ApplyGameplayValues() {
    if (GS_Gameplay::HasInstance()) GS_Gameplay::GetInstance()->EnableShootingArea(m_def->IsShootingArea() ? 0xf : 0);
}

// @0x3e0b8c: one object a call for each platform, those done go to the level's deletion
void GameplayTemplateInstance::UpdateDelayedPlatformInstances() {
    for (size_t i = 0; i < m_platformInstances.size();) {
        PlatformInstance *p = m_platformInstances[i];
        if (!p->CreateObjectsFromSequence(this, 1)) {
            i++;
            continue;
        }
        m_platformInstances.erase(m_platformInstances.begin() + i);
        GameLevel::GetInstance()->DeleteEntityLater(p);
    }
}

// @0x3e0d7c: a platform keeps its object (the lanes follow the ground platform's model), an interactive object instance gives its object and goes;
// an entity that appears less than always may be dropped
GameEntity *GameplayTemplateInstance::ProcessCreatedEntity(const GameEntity *tmpl, GameEntity *copy, GameplayTemplateInstance *previous) {
    GameEntity *result = copy;
    if (copy->GetType() == 0xc) {
        PlatformInstance *p = static_cast<PlatformInstance*>(copy);
        Object *obj = p->GetCreatedEntity();
        PlatformInstance *ground = m_def->GetGroundPlatform();
        if (ground && tmpl == ground) InitLanes(obj, p, ground, previous);
        p->SetGameplayTemplateInstance(this);
        if (!p->UsesRandomizer()) p->AddSequencesFromParamConfig();
        else p->AddSequencesFromRandomizer();
        p->CopySequenceToCreate();
        const int location = GameLevel::GetInstance()->GetLevelTemplateMgr()->GetBGLocation(m_location);
        if (location != -1) {
            // GetLocationConfig can legitimately return null (BackgroundMgr::EnableFluffyDimensionBackground
            // and EdgeProbabilityMgr already null-check its result) -- this call site was missing the
            // check and crashed here the first time this session loaded gameplay with partial location data.
            LocationConfig *c = LocationsMgr::GetInstance()->GetLocationConfig(location);
            if (c && !c->GetRenderPlatforms()) obj->GetDeco()->RegisterForRender(false);
        }
        p->SetKeepCreatedEntity(true);
        m_platformInstances.push_back(p);
        result = obj;
    } else if (copy->GetType() == 0xe) {
        ObjectHolder *h = dynamic_cast<ObjectHolder*>(copy);
        h->SetKeepCreatedEntity(true);
        result = h->GetCreatedEntity();
        delete copy;
    }
    if (!GameLevel::s_allProbabilityMode && result && result->GetAppearProbability() != 100 &&
        result->GetAppearProbability() < GameLevel::s_randomizer[1]->Rand(0, 100)) {
        delete result;
        result = 0;
    }
    return result;
}

// @0x3e1058: "Dummy_L012": the lane and the digits of the node
jet::String GameplayTemplateInstance::GetLaneDummyName(LaneIndex lane, int hundreds, int tens, int ones) {
    char b[32];
    const char c = lane == Lane_Center ? 'C' : lane == Lane_Right ? 'R' : 'L';
    if (lane >= Lane_Count) return jet::String();
    snprintf(b, sizeof b, "Dummy_%c%d%d%d", c, hundreds, tens, ones);
    return jet::String(b);
}

// @0x3e1118: the lanes are the paths through the Dummy_L/C/R nodes of the ground platform's model (and of the copies that follow it when the
// template is longer), led in from where the previous template ended
void GameplayTemplateInstance::InitLanes(Object *obj, PlatformInstance *copy, PlatformInstance *ground, GameplayTemplateInstance *previous) {
    m_groundObject = obj;
    std::vector<math::vec3<float> > pos[3];
    std::vector<math::quat<float> > rot[3];
    int counts[3] = { 0, 0, 0 };
    const math::vec3<float> tdp = m_def->GetTemplateDataPosition();
    const math::vec3<float> offset(0, 0, tdp.z - copy->GetPosition().z);
    jet::scene::Model *model = obj->GetDeco()->GetModel();
    jet::scene::Model *extra = 0;
    PlatformInstance *next = ground->GetNextCloned();
    m_numTotalDummies = 0;
    int firstLane = -1;
    for (int n = 0; n < 1000;) {
        const int h = n / 100, t = (n - h * 100) / 10, o = n - h * 100 - t * 10;
        bool none = true;
        for (int lane = 0; lane < 3; lane++) {
            const jet::scene::Node *node = model->FindNodeByName(GetLaneDummyName((LaneIndex)lane, h, t, o));
            if (!node) continue;
            pos[lane].push_back(node->GetAbsolutePosition() + offset);
            rot[lane].push_back(node->GetAbsoluteRotation());
            counts[lane]++;
            none = false;
            if (firstLane == -1) firstLane = lane;
        }
        if (!none) m_numTotalDummies++;
        if (!(none && next)) {
            n++;
            continue;
        }
        // nothing more in this model: on to the model of the next copy, placed after this one
        if (GameEntity *deco = next->GetDefinition()->GetDecoReference()) {
            if (extra) jet::scene::ModelLoader::GetInstance()->Free(extra);
            jet::String name;
            deco->GetParam(Deco3d::k_model, name);
            extra = jet::scene::ModelLoader::GetInstance()->Load(name);
            if (extra) {
                extra->SetUpdateMode(2);
                jet::scene::Node &root = obj->GetDeco()->GetModel()->GetRootNode();
                const math::quat<float> rootRot = root.GetAbsoluteRotation();
                const math::vec3<float> delta = next->GetPosition() - ground->GetPosition();
                const math::vec3<float> moved = math::transform(obj->GetRotation().getAsMat3(), delta);
                extra->GetRootNode().SetTransform(root.GetAbsolutePosition() + moved, rootRot);
                model = extra;
                if (extra->GetAnimation()) extra->AnimateNodes(true);
            }
        }
        n = 0;
        next = next->GetNextCloned();
    }
    if (extra) jet::scene::ModelLoader::GetInstance()->Free(extra);
    if (!previous) {
        for (int lane = 0; lane < 3; lane++) {
            pos[lane].insert(pos[lane].begin(), pos[firstLane][0]);
            rot[lane].insert(rot[lane].begin(), rot[firstLane][0]);
        }
    } else {
        for (int lane = 0; lane < 3; lane++) {
            boost::shared_ptr<PathCommon> path = previous->GetLanePath((LaneIndex)lane);
            math::vec3<float> p = path->GetLastInterpolatedPosition();
            math::quat<float> r = path->GetLastInterpolatedRotation();
            if (previous->GetBifurcation() == Bifurcation_Tee && math::vec3<float>(pos[lane][0] - p).getLength() > 3.0f) {
                p = pos[lane][0];
                r = rot[lane][0];
            }
            pos[lane].insert(pos[lane].begin(), p);
            rot[lane].insert(rot[lane].begin(), r);
        }
    }
    int valid = 0;
    for (int lane = 0; lane < 3; lane++) {
        LaneData &d = m_lanes[lane];
        d.valid = counts[lane] != 0;
        if (d.valid) valid++;
        else {
            // a lane the model does not have follows the first one
            const math::vec3<float> delta = pos[lane][0] - pos[firstLane][0];
            for (size_t j = 1; j < pos[firstLane].size(); j++) {
                pos[lane].push_back(pos[firstLane][j] + delta);
                rot[lane].push_back(rot[firstLane][j]);
            }
        }
        d.single = (copy->GetSingleLane() == lane);
        d.collisions.assign(pos[lane].size(), false);
        d.bonus.assign(pos[lane].size(), false);
        d.positions = pos[lane];
        d.rotations = rot[lane];
        d.path.reset(new PathCommon(pos[lane], rot[lane], std::vector<float>(), 0.2f, jet::core::InterpolationTypeDef::SPLINE, 2, 0.5f, 0.5f,
                                    jet::video::Color(2421511679u)));
        d.samplesPerSegment = d.path->GetNumSamplesPerSegment();
    }
    if (valid == 1)
        for (int lane = 0; lane < 3; lane++) m_lanes[lane].single = counts[lane] != 0;
    InitLaneForbiddenMovementData(ground->GetDefinition());
    InitBonusIntervalsData(ground->GetDefinition());
}

// @0x3e1da4
void GameplayTemplateInstance::InitLaneForbiddenMovementData(const PlatformTemplate *t) {
    for (const LaneInterval *i : t->GetForbiddenMovements()) m_laneForbiddenMovements.push_back(new LaneInterval(*i));
}

// @0x3e1e48: the nodes of the lanes between the interval's lanes that the bonus runs along
void GameplayTemplateInstance::InitBonusIntervalsData(const PlatformTemplate *t) {
    for (const LaneInterval *i : t->GetBonusIntervals()) {
        const int first = std::min(i->originLane, i->destinationLane), last = std::max(i->originLane, i->destinationLane);
        const int from = (int)i->nodeInterval.x;
        for (int lane = first; lane <= last; lane++) {
            std::vector<bool> &bits = m_lanes[lane].bonus;
            const int to = (int)i->nodeInterval.y == -1 ? (int)bits.size() - 1 : std::min((int)i->nodeInterval.y, (int)bits.size() - 1);
            for (int n = from; n <= to; n++) bits[n] = true;
        }
    }
}

// @0x3e2530
bool GameplayTemplateInstance::CanReachLane(LaneIndex from, LaneIndex to, int node) const {
    if (m_lanes[from].single) return false;
    if (m_lanes[to].single) return false;
    for (const LaneInterval *i : m_laneForbiddenMovements)
        if (i->originLane == from && i->destinationLane == to && node >= (int)i->nodeInterval.x && node <= (int)i->nodeInterval.y) return false;
    return true;
}

// @0x4cead0
bool GameplayTemplateInstance::HasCollisionOnLane(LaneIndex l, unsigned node) const {
    return node < m_lanes[l].collisions.size() && m_lanes[l].collisions[node];
}

void GameplayTemplateInstance::SetCollisionOnLane(LaneIndex l, unsigned node, bool b) {
    if (node < m_lanes[l].collisions.size()) m_lanes[l].collisions[node] = b;
}

bool GameplayTemplateInstance::IsBonusGameplayNode(LaneIndex l, unsigned node) const {
    return node < m_lanes[l].bonus.size() && m_lanes[l].bonus[node];
}

// @0x3e1f44: debug drawing of the lanes is not part of the port
void GameplayTemplateInstance::RenderLanes() {}

// @0x3e20d8
void GameplayTemplateInstance::CacheLanesTransform(std::vector<math::vec3<float> > &pos, std::vector<math::quat<float> > &rot, std::vector<bool> &valid, unsigned segment, float t) const {
    for (int l = 0; l < 3; l++) {
        valid[l] = m_lanes[l].path != 0;
        if (!valid[l]) continue;
        pos[l] = m_lanes[l].path->GetPositionAt(segment, t);
        rot[l] = m_lanes[l].path->GetRotationAt(segment, t);
    }
}

// @0x3e21ac
bool GameplayTemplateInstance::GetClosestLanePosition(const math::vec3<float> &p, math::vec3<float> &pos, math::quat<float> &rot) const {
    boost::shared_ptr<PathCommon> path = GetLanePath(Lane_Center);
    if (!path) return false;
    unsigned segment;
    float t;
    path->GetClosestSegment(p, segment, t, 0);
    pos = path->GetPositionAt(segment, t);
    rot = path->GetRotationAt(segment, t);
    return true;
}

// @0x3e228c: the closest node of the three lanes, in the plane
bool GameplayTemplateInstance::GetClosestLanePosition(const math::vec3<float> &p, LaneIndex &lane, unsigned &node) const {
    float best = FLT_MAX;
    for (int l = 0; l < 3; l++)
        for (unsigned n = 0; n < m_lanes[l].positions.size(); n++) {
            const math::vec3<float> &q = m_lanes[l].positions[n];
            const float d = (q.x - p.x) * (q.x - p.x) + (q.y - p.y) * (q.y - p.y);
            if (d < best) {
                lane = (LaneIndex)l;
                node = n;
                best = d;
            }
        }
    return true;
}

static const PlatformTemplate *GroundTemplate(const LevelTemplateDef *d) { return d->GetTemplateData()->GetGroundPlatform(false)->GetDefinition(); }

float GameplayTemplateInstance::GetBeginMinionSledgeOffset() const { return m_def->GetGroundPlatform()->GetDefinition()->GetBeginMinionSledgeOffset(); }
float GameplayTemplateInstance::GetEndMinionSledgeOffset() const { return m_def->GetGroundPlatform()->GetDefinition()->GetEndMinionSledgeOffset(); }
int GameplayTemplateInstance::GetSledgeType() const { return m_def->GetGroundPlatform()->GetDefinition()->GetSledgeType(); }
Bifurcation GameplayTemplateInstance::GetBifurcation() const { return GroundTemplate(m_def)->GetBifurcation(); }
bool GameplayTemplateInstance::HasSecretExit() const { return GroundTemplate(m_def)->HasSecretExit(); }
Bifurcation GameplayTemplateInstance::GetSecretExitDirection() const { return GroundTemplate(m_def)->GetSecretExitDirection(); }
const std::vector<unsigned> &GameplayTemplateInstance::GetSecretExitLevelTemplates() const { return GroundTemplate(m_def)->GetSecretExitLevelTemplates(); }

// @0x3e2394: the lanes and the entities move too
void GameplayTemplateInstance::Translate(const math::vec3<float> &d) {
    math::vec3<float> start(0, 0, 0);
    float angle = 0;
    GetStartAnchor(start, angle);
    SetStartAnchor(start + d, angle);
    for (int l = 0; l < 3; l++) {
        if (m_lanes[l].path) m_lanes[l].path->Translate(d);
        for (math::vec3<float> &p : m_lanes[l].positions) p += d;
    }
    for (Instance &i : m_createdEntities) i.position += d;
}

// @0x3e2550
unsigned GameplayTemplateInstance::ComputeRelativeDistance(const math::vec3<float> &p) const {
    if (GetBifurcation() == Bifurcation_Center) {
        const float d = PlaneDistance(p, 0);
        return d > 0 ? (unsigned)d : 0;
    }
    LaneIndex lane = Lane_Left;
    unsigned node = 0;
    GetClosestLanePosition(p, lane, node);
    const float d = ((float)node / (float)m_lanes[lane].positions.size()) * m_def->GetSizeY();
    return d > 0 ? (unsigned)d : 0;
}

// @0x3e25e8: where the center lane is after a distance along the template
bool GameplayTemplateInstance::GetPredictedTransform(unsigned distance, math::vec3<float> &pos, math::quat<float> &rot) const {
    const std::vector<math::vec3<float> > &p = m_lanes[Lane_Center].positions;
    if (p.empty()) return false;
    const unsigned n = (unsigned)p.size();
    const double v = (double)distance / ((double)m_def->GetSizeY() / (double)n);
    unsigned i = v > 0 ? (unsigned)v : 0;
    if (i > n - 1) i = n - 1;
    pos = p[i];
    rot = m_lanes[Lane_Center].rotations[i];
    return true;
}

// @0x3e2730: where along the template a deco of the group stands, and the group it joins
void GameplayTemplateInstance::AddToEntityGenerationGroup(GameEntity::GenerationGroupEntity *e, unsigned index) {
    if (!e) return;
    e->position = m_startDistance + (m_def->GetSizeY() / (float)(m_numTotalDummies - 1)) * (float)index;
    e->group = m_entityGenerationGroupId;
    m_def->GetEntityGenerationGroup(m_entityGenerationGroupId).AddEntity(e);
}

// @0x3e27c4: its own group of decos, 30 meters apart, a few more than the usual per template
void GameplayTemplateInstance::InitEntityGenerationGroup() {
    m_entityGenerationGroupId = s_entityGenerationGroupSequentialId++;
    const float f = CGameConfig::Current().decoGenerationGroupAllowedDecosLerp;
    LevelTemplateDef::EntityGenerationGroup &g = m_def->GetEntityGenerationGroup(m_entityGenerationGroupId);
    g.minDistance = 30.0f;
    const float n = (f * 3.0f - f) + 1.0f;
    g.maxDecos = n > 0 ? (unsigned)n : 0;
}

// @0x3e2848
void GameplayTemplateInstance::ClearEntityGenerationGroup() { m_def->RemoveEntityGenerationGroup(m_entityGenerationGroupId); }

// @0x3e2940
bool GameplayTemplateInstance::IsGeneratedBySpecialNode() const {
    return !m_creationNodeId.Equals(k_emptyStr) && GameLevel::GetInstance()->IsNodeOnSpecialTree(m_creationNodeId);
}

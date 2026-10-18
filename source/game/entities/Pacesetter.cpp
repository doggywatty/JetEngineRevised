// source/game/entities/Pacesetter.cpp
#include "Pacesetter.h"
#include "pacesetterfollower.h"
#include "JumpDefinition.h"
#include "paths/pathcommon.h"
#include "../GameLevel.h"
#include "../Game.h"
#include "../leveltemplate/GameplayTemplateInstance.h"
#include "../leveltemplate/LevelTemplateMgr.h"
#include <algorithm>
#include <cmath>

jet::String Pacesetter::k_tmplName("Pacesetter");

static float Sign(float v) { return v < 0 ? -1.0f : (v == 0 ? 0.0f : 1.0f); }
static LevelTemplateMgr *Templates() { return GameLevel::GetInstance()->GetLevelTemplateMgr(); }
static unsigned LastSegment(const boost::shared_ptr<PathCommon> &p) { return (unsigned)p->GetSegmentLenghts().size() - 1; }

// @0x4f577c
Pacesetter::Pacesetter(const clara::Template *t)
    : GameEntity(t), m_currentGti(0), m_currentLane(1), m_segment(0), m_t(0), m_jump(0), m_jumpCtrl(0), m_jumpSpeed(0),
      m_laneMu(0.0f, 0.0f, 0), m_speed(0.0f, 0.0f, 1000), m_speedAcceleration(1.0f), m_speedHold(0), m_paused(false), m_isBonusNode(false),
      m_prevLaneValid(false), m_forcedStep(-1.0f), m_step(0), m_laneCheckDisabled(false), m_laneCachePos(3), m_laneCacheRot(3),
      m_laneCacheValid(3, false), m_controller(0) {}

Pacesetter::~Pacesetter() {
    RegisterForUpdate(false);
    RegisterForRender(false);
}

// @0x4f5a08
void Pacesetter::Init() {
    if (!(m_flags & F_INIT)) clara::Entity::Init();
}

// @0x4f5a0c
void Pacesetter::PostInit() {
    m_currentLane = 1;
    RegisterForUpdate(true);
    RegisterForRender(true);
}

// @0x4ceb44
void Pacesetter::Update() {
    if (m_paused || !m_currentGti) return;
    _Update();
}

// @0x4f6b44: the followers go where it goes
void Pacesetter::NotifyTransformChanged(bool b) {
    GameEntity::NotifyTransformChanged(b);
    for (PacesetterFollower *f : m_followers) f->GoTo(GetPosition(), GetRotation());
}

// @0x4f6964
void Pacesetter::RegisterFollower(PacesetterFollower *f) {
    m_followers.push_back(f);
    f->SetPacesetter(this);
}

// @0x4f69dc
void Pacesetter::UnregisterFollower(PacesetterFollower *f) {
    m_followers.erase(std::remove(m_followers.begin(), m_followers.end(), f), m_followers.end());
    f->SetPacesetter(0);
}

// @0x4f6abc
void Pacesetter::SetController(PacesetterController *c) {
    if (c == m_controller) return;
    if (m_controller) {
        m_controller->EndControl(this);
        SendPacesetterEvent({ PacesetterEventType::ControlEnded });
    }
    m_controller = c;
    if (c) {
        c->StartControl(this);
        SendPacesetterEvent({ PacesetterEventType::ControlStarted });
    }
}

// @0x4f7580
void Pacesetter::SendPacesetterEvent(const PacesetterEvent &e) {
    for (PacesetterFollower *f : m_followers) f->OnPacesetterEvent(e);
}

// @0x4b0618
void Pacesetter::SetCurrentGameplayTemplateInstance(GameplayTemplateInstance *g) {
    if (m_currentGti) m_currentGti->UnregisterListener(this);
    m_currentGti = g;
    if (g) g->RegisterListener(this);
}

// @0x4f5a34: the instance is gone, the followers know
void Pacesetter::OnGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent &e) {
    if (e.type != GameplayTemplateInstanceEvent::Destroyed) return;
    m_currentGti = 0;
    for (PacesetterFollower *f : m_followers) f->OnGameplayTemplateInstanceDestroyed();
}

// @0x4b0678
void Pacesetter::SetSpeed(float speed, int time) {
    if (speed == m_speed.GetTargetValue()) {
        if (time < 0) return;
    } else if (time < 0) {
        time = (int)((float)(int)std::fabs(m_speed.GetCurrentValue() - speed) / m_speedAcceleration) * 1000;
    }
    m_speed.SetDelay(time);
    m_speed.SetTargetValue(speed);
}

// @0x4bed48
void Pacesetter::SetChangeLaneMu(float mu, int time) {
    m_laneMu.SetDelay(time);
    m_laneMu.SetTargetValue(mu);
}

// @0x4cecbc: the lane is reached from the current one by the lanes between
void Pacesetter::GoToLane(LaneIndex lane, int time) {
    SetChangeLaneMu(0, 0);
    const float target = m_laneMu.GetTargetValue();
    const int moved = (int)std::floor(std::fabs(target) + 0.5f) * (int)Sign(target);
    SetChangeLaneMu((float)((int)lane - (moved + m_currentLane)) + target, time);
}

// @0x4ceb60: when the move passes a lane the pacesetter is in the next one (unless that is one that does not exist)
void Pacesetter::UpdateCurrentLane() {
    float target = m_laneMu.GetTargetValue();
    int lanes = (int)std::floor(std::fabs(target) + 0.5f);
    if (lanes > 0) {
        const int n = lanes * (int)Sign(target);
        const unsigned next = (unsigned)(n + m_currentLane);
        if (next > 2 || !m_currentGti->IsValidLane((LaneIndex)next)) {
            target -= (float)n;
            m_laneMu.SetTargetValue(target);
        }
    }
    const float cur = m_laneMu.GetCurrentValue();
    lanes = (int)std::floor(std::fabs(cur) + 0.5f);
    if (lanes > 0) {
        const int n = lanes * (int)Sign(cur);
        m_currentLane = math::clamp(n + m_currentLane, 0, 2);
        m_laneMu.SetCurrentValue(cur - (float)n);
        m_laneMu.SetTargetValue(target - (float)n);
    }
}

// @0x4f5a7c
void Pacesetter::GetTransformInPath(math::vec3<float> &pos, math::quat<float> &rot, LaneIndex lane) {
    if (lane == 3) lane = (LaneIndex)m_currentLane;
    boost::shared_ptr<PathCommon> path = m_currentGti->GetLanePath(lane);
    pos = path->GetPositionAt(m_segment, m_t);
    rot = path->GetRotationAt(m_segment, m_t);
}

// @0x4f5b18: the transform a distance ahead (behind when negative), on the following (or the previous) template when it goes past
void Pacesetter::GetAdvancedTransformInPath(float distance, math::vec3<float> &pos, math::quat<float> &rot, LaneIndex lane) {
    if (lane == 3) lane = (LaneIndex)m_currentLane;
    boost::shared_ptr<PathCommon> path = m_currentGti->GetLanePath(lane);
    unsigned seg = 0;
    float t = 0;
    path->TravelAlongPath(m_segment, m_t, distance, seg, t, 0);
    if (distance < 0.0f) {
        if (m_segment == seg && m_t == t) {
            if (GameplayTemplateInstance *prev = Templates()->GetPreviousLevelTemplateInstance(m_currentGti)) {
                boost::shared_ptr<PathCommon> other = prev->GetLanePath(lane);
                const float travelled = path->GetTravelDistanceTo(seg, t);
                path = other;
                path->TravelAlongPath(LastSegment(path), 1.0f, travelled + distance, seg, t, 0);
            }
        }
    } else if (seg < m_segment) {
        if (GameplayTemplateInstance *next = Templates()->GetNextLevelTemplateInstance(m_currentGti)) {
            const float travelled = path->GetTravelDistanceTo(seg, t);
            path = next->GetLanePath(lane);
            path->TravelAlongPath(0, 0, travelled, seg, t, 0);
        }
    }
    pos = path->GetPositionAt(seg, t);
    rot = path->GetRotationAt(seg, t);
}

// @0x4f5d4c
bool Pacesetter::GetAdvancedInfo(float distance, LaneIndex lane, GameplayTemplateInstance *&gti, boost::shared_ptr<PathCommon> &path, unsigned &segment, float &t) {
    gti = m_currentGti;
    path = gti->GetLanePath(lane);
    segment = 0;
    t = 0;
    path->TravelAlongPath(m_segment, m_t, distance, segment, t, 0);
    if (distance >= 0.0f && segment < m_segment) {
        if (GameplayTemplateInstance *next = Templates()->GetNextLevelTemplateInstance(gti)) {
            const float travelled = path->GetTravelDistanceTo(segment, t);
            gti = next;
            path = next->GetLanePath(lane);
            path->TravelAlongPath(0, 0, travelled, segment, t, 0);
        }
    }
    return true;
}

// @0x4f5e8c: the lane's node some segments from the current one, through the templates around; false when they end first
bool Pacesetter::GetAdvancedSegmentNodeTransformInPath(int segments, math::vec3<float> &pos, math::quat<float> &rot, LaneIndex lane) {
    GameplayTemplateInstance *gti = m_currentGti;
    if (lane == 3) lane = (LaneIndex)m_currentLane;
    boost::shared_ptr<PathCommon> path = gti->GetLanePath(lane);
    int node = segments + (int)m_segment;
    bool found = true;
    if (segments < 1) {
        if (segments != 0) {
            while (node < 0) {
                gti = Templates()->GetPreviousLevelTemplateInstance(gti);
                if (!gti) {
                    node = 0;
                    found = false;
                    break;
                }
                path = gti->GetLanePath(lane);
                node += (int)path->GetInterpolatedPositions().size();
            }
        }
    } else {
        int count;
        while ((count = (int)path->GetInterpolatedPositions().size()) <= node) {
            gti = Templates()->GetNextLevelTemplateInstance(gti);
            if (!gti) {
                node = count - 1;
                found = false;
                break;
            }
            path = gti->GetLanePath(lane);
            node %= count;
        }
    }
    pos = path->GetInterpolatedPositions()[(unsigned)node];
    rot = path->GetInterpolatedRotations()[(unsigned)node];
    return found;
}

// @0x4f602c
void Pacesetter::_Update() {
    Game *game = Game::GetInstance();
    m_step = 0;
    const int dt = (int)(game->GetUpdateTimeUs() / 1000);
    m_speedHold = std::max(0, m_speedHold - dt);
    if (m_speedHold == 0) m_speed.Update(dt);
    if (m_controller) {
        math::vec3<float> pos(0, 0, 0);
        math::quat<float> rot(0, 0, 0, 1);
        if (m_controller->GetTransform(this, pos, rot)) {
            SetLanePosition(pos, 0, (LaneIndex)3, 0);
            SetPosition(pos);
            SetRotation(rot);
        }
        m_step = m_speed.GetCurrentValue() * game->GetUpdateTimeSeconds();
    } else {
        if (!m_laneCheckDisabled) {
            const bool valid = m_currentGti->IsValidLane((LaneIndex)m_currentLane);
            if (m_prevLaneValid && !valid) GoToLane((LaneIndex)1, 233);
            m_prevLaneValid = valid;
        }
        float step = m_forcedStep;
        m_forcedStep = -1.0f;
        if (step == -1.0f) step = m_speed.GetCurrentValue() * game->GetUpdateTimeSeconds();
        if (m_jump && m_jumpCtrl && step != 0.0f) {
            math::vec3<float> delta = m_jump->GetMovementDelta(*m_jumpCtrl);
            delta.z = 0;
            step = delta.getLength();
            m_jumpSpeed = m_jump->GetAverageFrontSpeed(*m_jumpCtrl);
        }
        m_step = step;
        boost::shared_ptr<PathCommon> path = m_currentGti->GetLanePath((LaneIndex)m_currentLane);
        unsigned seg = 0;
        float t = 0;
        path->TravelAlongPath(m_segment, m_t, step, seg, t, 0);
        if (step >= 0.0f) {
            if (seg < m_segment) {
                if (GameplayTemplateInstance *next = Templates()->GetNextLevelTemplateInstance(m_currentGti)) {
                    boost::shared_ptr<PathCommon> other = next->GetLanePath((LaneIndex)m_currentLane);
                    const float travelled = path->GetTravelDistanceTo(seg, t);
                    SetCurrentGameplayTemplateInstance(next);
                    path = other;
                    path->TravelAlongPath(0, 0, travelled, seg, t, 0);
                }
            }
        } else if (m_segment == seg && m_t == t) {
            if (GameplayTemplateInstance *prev = Templates()->GetPreviousLevelTemplateInstance(m_currentGti)) {
                boost::shared_ptr<PathCommon> other = prev->GetLanePath((LaneIndex)m_currentLane);
                const float travelled = path->GetTravelDistanceTo(seg, t);
                m_t = 1.0f;
                m_segment = LastSegment(other);
                SetCurrentGameplayTemplateInstance(prev);
                path = other;
                path->TravelAlongPath(m_segment, m_t, travelled + step, seg, t, 0);
            }
        }
        m_segment = seg;
        m_t = t;
        m_currentGti->CacheLanesTransform(m_laneCachePos, m_laneCacheRot, m_laneCacheValid, seg, t);
        if (m_laneMu.Update(dt)) UpdateCurrentLane();
        const float mu = m_laneMu.GetCurrentValue();
        const float dir = Sign(mu), lo = std::floor(std::fabs(mu)), hi = std::ceil(std::fabs(mu));
        const int a = math::clamp(m_currentLane + (int)(lo * dir), 0, 2), b = math::clamp(m_currentLane + (int)(hi * dir), 0, 2);
        math::vec3<float> pos;
        math::quat<float> rot;
        if (a == b) {
            pos = m_laneCachePos[(unsigned)b];
            rot = m_laneCacheRot[(unsigned)b];
        } else {
            const float f = std::fabs(mu - (float)(int)(lo * dir));
            pos = math::lerp(m_laneCachePos[(unsigned)a], m_laneCachePos[(unsigned)b], f);
            rot = math::lerp(m_laneCacheRot[(unsigned)a], m_laneCacheRot[(unsigned)b], f);
        }
        SetPosition(pos);
        SetRotation(rot);
    }
    const bool before = m_isBonusNode;
    m_isBonusNode = m_currentGti->IsBonusGameplayNode((LaneIndex)m_currentLane, m_segment / (m_currentGti->GetNumSamplesPerSegment((LaneIndex)m_currentLane) + 1));
    if (m_isBonusNode != before)
        for (PacesetterFollower *f : m_followers) f->NotifyInBonusGameplay(m_isBonusNode);
}

// @0x4f66d8
float Pacesetter::GetTraveledDistanceOnCurrentPath() {
    return m_currentGti->GetLanePath((LaneIndex)m_currentLane)->GetTravelDistanceTo(m_segment, m_t);
}

// @0x4f6728
void Pacesetter::ApplyCurrentLaneTransform() {
    m_currentGti->CacheLanesTransform(m_laneCachePos, m_laneCacheRot, m_laneCacheValid, m_segment, m_t);
    SetPosition(m_laneCachePos[(unsigned)m_currentLane]);
    SetRotation(m_laneCacheRot[(unsigned)m_currentLane]);
}

// @0x4f6ba8
int Pacesetter::GetCurrentDummyIndex(LaneIndex lane) {
    return (int)(m_segment / (m_currentGti->GetLanePath(lane)->GetNumSamplesPerSegment() + 1));
}

// @0x4f6bec
bool Pacesetter::CanReachLane(LaneIndex lane, bool checkCollisions) const {
    if ((unsigned)lane >= 3 || !m_currentGti->IsValidLane(lane)) return false;
    const unsigned node = m_segment / (m_currentGti->GetLanePath(lane)->GetNumSamplesPerSegment() + 1);
    bool ok = m_currentGti->CanReachLane((LaneIndex)m_currentLane, lane, (int)node);
    if (ok && checkCollisions) ok = !m_currentGti->HasCollisionOnLane(lane, node);
    return ok;
}

// @0x4f6cac
bool Pacesetter::IsInSingleLane() const { return m_currentGti->IsSingleLane((LaneIndex)m_currentLane); }

// @0x4f6cc4: a conditional bifurcation is crossed in its lane until the player has chosen
bool Pacesetter::CanDodgeInConditionalBifurcation() const {
    if (!m_currentGti || m_currentGti->GetBifurcation() != 3) return true;
    return !m_currentGti->IsBifurcationDecided();
}

// @0x4f6d18
bool Pacesetter::CanDodgeLeft(bool checkCollisions) const {
    return CanDodgeInConditionalBifurcation() && !m_controller && CanReachLane((LaneIndex)(m_currentLane - 1), checkCollisions);
}

// @0x4f6d5c
bool Pacesetter::CanDodgeRight(bool checkCollisions) const {
    return CanDodgeInConditionalBifurcation() && !m_controller && CanReachLane((LaneIndex)(m_currentLane + 1), checkCollisions);
}

// @0x4f6da4
bool Pacesetter::SetLanePosition(const math::vec3<float> &pos, float advance, LaneIndex lane, math::vec3<float> *offset) {
    if (lane != 3) m_currentLane = lane;
    bool ok = true;
    boost::shared_ptr<PathCommon> path = m_currentGti->GetLanePath((LaneIndex)m_currentLane);
    for (;;) {
        float closest = 100000000.0f;
        path->GetClosestSegment(pos, m_segment, m_t, &closest);
        const float away = math::distance(path->GetPositionAt(m_segment, m_t), pos);
        if (m_segment == LastSegment(path) && m_t == 1.0f && away > 0.0f) {
            GameplayTemplateInstance *next = Templates()->GetNextLevelTemplateInstance(m_currentGti);
            if (!next) {
                ok = false;
                break;
            }
            SetCurrentGameplayTemplateInstance(next);
            path = m_currentGti->GetLanePath((LaneIndex)m_currentLane);
            continue;
        }
        GameplayTemplateInstance *next;
        if (m_currentGti->GetBifurcation() != 3 || !(next = Templates()->GetNextLevelTemplateInstance(m_currentGti))) break;
        boost::shared_ptr<PathCommon> other = next->GetLanePath((LaneIndex)m_currentLane);
        float otherClosest = 100000000.0f;
        unsigned seg;
        float t;
        other->GetClosestSegment(pos, seg, t, &otherClosest);
        if (closest <= otherClosest) break;
        SetCurrentGameplayTemplateInstance(next);
        path = m_currentGti->GetLanePath((LaneIndex)m_currentLane);
    }
    if (offset && ok) *offset = pos - path->GetPositionAt(m_segment, m_t);
    if (advance == 0.0f || !ok) return ok;
    unsigned seg = 0;
    float t = 0;
    path->TravelAlongPath(m_segment, m_t, advance, seg, t, 0);
    if (seg < m_segment) {
        const float travelled = path->GetTravelDistanceTo(seg, t);
        if (GameplayTemplateInstance *next = Templates()->GetNextLevelTemplateInstance(m_currentGti)) {
            path = next->GetLanePath((LaneIndex)m_currentLane);
            SetCurrentGameplayTemplateInstance(next);
            path->GetClosestSegment(pos, m_segment, m_t, 0);
            path->TravelAlongPath(m_segment, m_t, travelled, seg, t, 0);
        } else
            ok = false;
    }
    m_segment = seg;
    m_t = t;
    return ok;
}

// @0x4f7190
bool Pacesetter::GetDistanceInLane(const math::vec3<float> &pos, float &distance, std::vector<PacestterCacheData> &cache, float &direction, LaneIndex lane) {
    int dir = 0;
    unsigned index = 0;
    if (lane == 3) lane = (LaneIndex)m_currentLane;
    GameplayTemplateInstance *gti = m_currentGti;
    boost::shared_ptr<PathCommon> path = gti->GetLanePath(lane);
    unsigned startSegment = m_segment, segment = 0;
    float startT = m_t, t = 0;
    distance = 0;
    for (;;) {
        bool cached = false;
        while (index < cache.size()) {
            if (cache[index].instance == gti) {
                segment = cache[index].segment;
                t = cache[index].t;
                cached = true;
                break;
            }
            cache.erase(cache.begin() + index);
        }
        if (!cached) {
            path->GetClosestSegment(pos, segment, t, 0);
            cache.push_back({ gti, segment, t });
        }
        distance += path->GetDistanceFromTo(startSegment, startT, segment, t, true);
        direction = (segment > startSegment || (segment == startSegment && startT < t)) ? 1.0f : -1.0f;
        if (segment == 0 && t == 0.0f && dir != 1) {
            gti = Templates()->GetPreviousLevelTemplateInstance(gti);
            dir = -1;
        } else {
            if (segment != LastSegment(path) || t != 1.0f || dir == -1) return true;
            if (segment == 0) {
                gti = Templates()->GetPreviousLevelTemplateInstance(gti);
                dir = -1;
            } else {
                gti = Templates()->GetNextLevelTemplateInstance(gti);
                dir = 1;
            }
        }
        if (!gti || !gti->IsValidLane(lane)) {
            distance = 0;
            return false;
        }
        path = gti->GetLanePath(lane);
        if (segment == 0) {
            startSegment = LastSegment(path);
            startT = 1.0f;
        } else {
            startSegment = 0;
            startT = 0;
        }
        index++;
    }
}

// @0x4f74e8: the time (ms) to the plane of a point; a long time when it is out of reach
float Pacesetter::GetTimeToReachPlane(const math::vec3<float> &pos, const math::quat<float>&, std::vector<PacestterCacheData> &cache, float *distanceOut) {
    float distance = 0, direction = 0;
    if (!GetDistanceInLane(pos, distance, cache, direction, (LaneIndex)3)) return 3600000.0f;
    if (distanceOut) *distanceOut = distance;
    const float speed = (!m_jump || !m_jumpCtrl) ? m_speed.GetCurrentValue() : m_jumpSpeed;
    return ((direction * distance) / speed) * 1000.0f;
}

// @0x575974 (in Minion::SM_OnChangeState): the position along the segment is 0 and the lane change is over, in the lane
void Pacesetter::ResetLaneForCrash() {
    m_t = 0;
    m_laneMu.SetDelay(0);
    m_laneMu.ResetValues(0.0f);
    ApplyCurrentLaneTransform();
}

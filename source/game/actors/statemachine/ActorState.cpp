// source/game/actors/statemachine/ActorState.cpp
#include "ActorState.h"
#include "ActorStateSet.h"
#include <cstdlib>
#include <cstring>
#include "core/util.h"
#include "camera/cameramgr.h"
#include "game/GameLevel.h"
#include "game/cameracontrollers/FollowMinionCamera.h"
#include "game/entities/Minion.h"
#include "game/entities/MinionCostume.h"
#include "game/gamemanagers/CostumeMgr.h"
#include "game/gamemanagers/PowerUpMgr.h"

static bool StartsWith(const jet::String &s, const char *prefix) {
    const char *a = s.c_str();
    for (; *prefix; a++, prefix++)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*prefix)) return false;
    return true;
}

// @0x511f44
jet::String ActorState::GetSubstringName(const char *src, int field) {
    for (int commas = 0; *src && commas <= field;)
        if (*src++ == ',') commas++;
    jet::String out;
    std::string s;
    for (; *src && *src != ',' && *src != '|'; src++) s += *src;
    return jet::String(s.c_str());
}

// @0x52131c
ActorState::ActorState(const StateData *d, int index)
    : m_data(d), m_index((short)index), m_label((short)d->GetParam("StateLabel", 1)), m_type(0), m_animStartTime(-1), m_animEndTime(-1),
      m_loopStartTime(-1), m_loopEndTime(-1), m_loopCount(0), m_blendTime(-1), m_loop(false), m_animSpeed(1.0f), m_allowDodgeByInput(false),
      m_isCinematicCameraEnabled(false), m_cinematicCamTransition(0), m_attackTrack(0), m_stepsTrack(0), m_landTrack(0), m_sledgeTrack(0),
      m_giantScaleTrack(0), m_bulletTimeTrack(0), m_musicVolumeTrack(0), m_blindBoxTrack(0), m_selectorRepeatBehavior(0), m_selectorLastSelected(0) {
    if (d->isSelector) {
        m_selectorRepeatBehavior = d->GetParam("repeatBehavior", 0);
        return;
    }
    m_type = (short)d->GetParam("StateType", 0);
    m_animSpeed = (float)d->GetParam("AnimSpeed", 100) * 0.01f;
    m_allowDodgeByInput = d->GetParam("AllowDodgeByInput", 0) == 1;
    m_cinematicCamTransition = (short)d->GetParam("CinematicCamTransition", 0);
    m_isCinematicCameraEnabled = d->GetParam("CinematicCamera", 0) == 1;
    if (const StateTrack *t = d->FindTrack(0, "Range"))
        if (!t->events.empty()) {
            m_animStartTime = (short)t->events[0].time;
            m_animEndTime = (short)(t->events[0].duration + m_animStartTime);
        }
    if (const StateTrack *t = d->FindTrack(0, "Loop"))
        if (!t->events.empty()) {
            const StateEvent &e = t->events[0];
            m_loopStartTime = (short)e.time;
            m_loopEndTime = (short)(e.duration + e.time);
            m_loop = true;
            m_loopCount = (short)e.args[0];
        }
    if (const StateTrack *t = d->FindTrack(0, "Blend"))
        if (!t->events.empty()) m_blendTime = (short)t->events[0].duration;
    if (d->anims.empty()) return;
    StateData *mut = const_cast<StateData*>(d);
    const std::vector<StateTrack> &tracks = d->anims[0].tracks;
    for (unsigned k = 0; k < tracks.size(); k++) {
        const StateTrack &tr = tracks[k];
        const jet::String &name = tr.name;
        if (StartsWith(name, "TRIGGER_")) {
            for (int f = 0;; f++) {
                const jet::String idx = GetSubstringName(name.c_str(), f);
                if (idx.empty()) break;
                const unsigned ti = (unsigned)atoi(idx.c_str());
                if (ti >= mut->triggers.size()) continue;
                StateTrigger &t = mut->triggers[ti];
                if (tr.events.empty()) {
                    t.start = -1;
                    t.end = -1;
                    continue;
                }
                for (const StateEvent &e : tr.events) {
                    if (e.args[0] == 1) t.delay = e.time;
                    if (e.args[0] == 0) {
                        t.start = e.time;
                        t.end = e.time + e.duration;
                    }
                }
            }
        } else if (StartsWith(name, "Attack")) m_attackTrack = &tr;
        else if (StartsWith(name, "STEPS_")) m_stepsTrack = &tr;
        else if (StartsWith(name, "LAND_")) m_landTrack = &tr;
        else if (StartsWith(name, "SLEDGE_")) m_sledgeTrack = &tr;
        else if (StartsWith(name, "GIANTSCALE_")) m_giantScaleTrack = &tr;
        else if (StartsWith(name, "BULLETTIME_")) m_bulletTimeTrack = &tr;
        else if (StartsWith(name, "MUSICVOLUME_")) m_musicVolumeTrack = &tr;
        else if (StartsWith(name, "DIRBLEND_")) {
            m_dirBlendR = GetSubstringName(name.c_str(), 0);
            m_dirBlendL = GetSubstringName(name.c_str(), 1);
        } else if (StartsWith(name, "EFFECT_")) {
            const jet::String effect = GetSubstringName(name.c_str(), 0), node = GetSubstringName(name.c_str(), 1);
            if (!effect.empty())
                for (const StateEvent &e : tr.events) m_effectConfig.push_back({ effect, node, &e });
        } else if (StartsWith(name, "SOUNDS_")) {
            const jet::String sound = GetSubstringName(name.c_str(), 0);
            for (const StateEvent &e : tr.events) m_soundConfig.push_back(new SoundConfig{ sound, (int)k, e.args.size() > 1 && e.args[1] == 1 });
        } else if (StartsWith(name, "BLINDBOX_")) m_blindBoxTrack = &tr;
    }
}

ActorState::~ActorState() {
    for (SoundConfig *s : m_soundConfig) delete s;
}

bool ActorState::HasTrigger(int kind, int code, int param) const {
    for (const StateTrigger &t : m_data->triggers)
        if (t.kind == kind && t.code == code && t.param == param) return true;
    return false;
}

bool ActorState::HasEngineTrigger(int code) const {
    for (const StateTrigger &t : m_data->triggers)
        if (t.kind == 1 && t.code == code) return true;
    return false;
}

// @0x522008
int ActorState::GetNextStateIndex(const ActorStateSet *set, MinionCostume *costume) const {
    int count = 0;
    for (const StateTrigger &t : m_data->triggers)
        if (t.code == -1 && t.param == -1 && set->GetState((unsigned)t.target) && CheckTriggerConditions(&t, costume)) count++;
    if (!count) return -1;
    int pick = jet::core::Rand(0, count);
    for (const StateTrigger &t : m_data->triggers)
        if (t.code == -1 && t.param == -1 && set->GetState((unsigned)t.target) && CheckTriggerConditions(&t, costume)) {
            if (pick < 1) return t.target;
            pick--;
        }
    return -1;
}

// @0x522160
void ActorState::GetStretchTime(float &start, float &end, unsigned animDuration) const {
    if (const StateTrack *t = m_data->FindTrack(0, "stretch")) {
        start = (float)t->events[0].time;
        end = (float)(t->events[0].time + t->events[0].duration);
        return;
    }
    start = 0;
    end = animDuration ? (float)animDuration : 0.0f;
    if (m_animStartTime < 0 || m_animEndTime < 0) return;
    start = (float)m_animStartTime;
    end = std::min(end, (float)m_animEndTime);
}

// @0x5223ac: the entries not picked yet (by the repeat behaviour) are weighted, the weight dropping with the time in the state
ActorState *ActorState::GetSelectedState(const ActorStateSet *set, int timeInState) {
    if (m_data->entries.empty()) return 0;
    const int n = (int)m_data->entries.size();
    unsigned all = 0;
    int total = 0;
    for (int i = 0; i < n; i++) {
        all |= 1u << i;
        if (!((1u << i) & m_selectorLastSelected)) total += m_data->entries[(unsigned)i].weight;
    }
    float roll = 0.0f;
    if (total > 0) roll = (float)(jet::core::Rand() % (unsigned)total);
    for (int i = 0; i < n; i++) {
        const unsigned bit = 1u << i;
        if (bit & m_selectorLastSelected) continue;
        const SelectorEntryData &e = m_data->entries[(unsigned)i];
        roll -= (float)e.weight;
        if (e.threshold < timeInState) roll -= (float)timeInState * e.factor;
        if (roll >= 0.0f && roll != 0.0f) continue;
        ActorState *s = set->GetState((unsigned)e.state);
        switch (m_selectorRepeatBehavior) {
        case 1: m_selectorLastSelected = bit; break;
        case 2: {
            const unsigned seen = bit | m_selectorLastSelected;
            m_selectorLastSelected = seen == all ? (n == 1 ? 0u : bit) : seen;
            break;
        }
        case 0: m_selectorLastSelected = 0; break;
        default: break;
        }
        return s;
    }
    return 0;
}

// the kinds of states by their StateType
// @0x560e18
bool ActorState::IsDodgeLeftState() const { return m_type == 0x1f || m_type == 0x21 || m_type == 0x28 || m_type == 0x1b || m_type == 0x17; }
// @0x560e4c
bool ActorState::IsDodgeRightState() const { return (m_type & ~2) == 0x20 || m_type == 0x29 || m_type == 0x1a || m_type == 0x16; }
// @0x560e84
bool ActorState::IsAnyDespicableEndState() const { return m_type == 0x2d || m_type == 0x2c; }
// @0x560ea8
bool ActorState::IsAnySledgeInitState() const { return m_type == 0x49 || m_type == 0x4a || m_type == 0x4b || m_type == 0x4c || m_type == 0x2d; }
// @0x560ee4
bool ActorState::IsAnyCrashState() const { return m_type == 0x24 || m_type == 0x25 || m_type == 0x2e; }
// @0x560f0c
bool ActorState::IsGravityState() const {
    return (m_type & ~2) == 4 || m_type == 5 || m_type == 0x12 || m_type == 0x11 || m_type == 0x27 || m_type == 0x28 || m_type == 0x29 || m_type == 0x26 ||
           m_type == 0x24 || m_type == 0x32;
}
// @0x560f78
bool ActorState::IsMountDisengageState() const { return (m_type & ~8) == 0x32; }
// @0x560f90
bool ActorState::IsAnyJumpingState() const { return m_type == 5 || m_type == 6 || m_type == 7 || m_type == 0x12 || m_type == 0x11; }
// @0x560fcc: the states the shadow is drawn in
bool ActorState::IsShadowState() const {
    const int t = m_type;
    if (t == 0x11 || (unsigned)(t - 4) <= 3) return false;
    if ((t >= 0x12 && t <= 0x1c) || (t >= 0x26 && t <= 0x29) || (t >= 0x2b && t <= 0x2e) || t == 0x31 || t == 0x32 || (t >= 0x35 && t <= 0x3b) || t == 0x48 ||
        t == 0x25 || t == 0x43)
        return false;
    return t != 0x1e;
}
// @0x5897a8
bool ActorState::IsDieState() const { return m_type == 0x1d || m_type == 0x2e || m_type == 0x24 || m_type == 0x25 || m_type == 0x26; }

// @0x58ab64: every condition of the trigger must hold. A condition names what it asks about: the speed (0 or 1 always hold), whether the
// minion can dodge left (0) or right (1) or cannot, the running camera, whether a boss is on (0) or not (1), which costume (by its index among
// the costumes of the data) the state machine's costume is or is not, whether the freeze gun is on (0) or off (1), and the sledge type.
// The jump state condition never holds.
bool ActorState::CheckTriggerConditions(const StateTrigger *t, MinionCostume *costume) {
    if (!t) return true;
    for (const StateTriggerCondition &c : t->conditions) {
        const jet::String &n = c.name;
        const int v = c.value;
        bool ok;
        if (n.Equals("MinionSpeed")) {
            ok = (unsigned)v <= 1;
        } else if (n.Equals("MinionCanDodge")) {
            Minion *m = GameLevel::GetInstance()->GetMinion();
            ok = v == 0 ? m->CanDodgeLeft() : v == 1 ? m->CanDodgeRight() : false;
        } else if (n.Equals("MinionCannotDodge")) {
            Minion *m = GameLevel::GetInstance()->GetMinion();
            ok = v == 0 ? !m->CanDodgeLeft() : v == 1 ? !m->CanDodgeRight() : false;
        } else if (n.Equals("MinionJumpState")) {
            ok = false;
        } else if (n.Equals("camera")) {
            CameraController *ctrl = CameraMgr::GetInstance()->GetCurrentController();
            if (!ctrl || !ctrl->GetType().Equals(FollowMinionCamera::k_cameraType)) return false;
            ok = (v == 0 || v == 1 || v == 2) && static_cast<FollowMinionCamera*>(ctrl)->GetCameraStyle() == v;
        } else if (n.Equals("isBossEnabled")) {
            const bool boss = GameLevel::GetInstance()->GetBoss() != 0;
            ok = v == 0 ? boss : v == 1 ? !boss : false;
        } else if (n.Equals("isMinion")) {
            CostumeMgr *mgr = CostumeMgr::GetInstance();
            if ((unsigned)v >= mgr->GetCostumeRefCount() || !costume) return false;
            ok = costume->GetClaraEntityName().Equals(mgr->GetCostumeRef((unsigned)v)->GetClaraEntityName());
        } else if (n.Equals("isNotMinion")) {
            CostumeMgr *mgr = CostumeMgr::GetInstance();
            ok = (unsigned)v >= mgr->GetCostumeRefCount() || !costume || !costume->GetClaraEntityName().Equals(mgr->GetCostumeRef((unsigned)v)->GetClaraEntityName());
        } else if (n.Equals("FreezeGun")) {
            const bool on = PowerUpMgr::GetInstance()->IsFreezeGunActive();
            ok = v == 0 ? on : v == 1 ? !on : false;
        } else if (n.Equals("SledgeType")) {
            ok = v == GameLevel::GetInstance()->GetSledgeType();
        } else
            return false;
        if (getenv("JET_DBGCOND")) fprintf(stderr, "cond %s=%d -> %d\n", n.c_str(), v, (int)ok);
        if (!ok) return false;
    }
    return true;
}

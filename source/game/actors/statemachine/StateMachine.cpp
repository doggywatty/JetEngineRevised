// source/game/actors/statemachine/StateMachine.cpp
#include "StateMachine.h"
#include <algorithm>
#include <cmath>
#include "project.h"
#include "../../Game.h"
#include "../../dynamicobjects/EffectMgr.h"
#include "../../dynamicobjects/EffectDef.h"
#include "../../interfaces/GameSettings.h"
#include "anim/animationloader.h"
#include "scene/model.h"
#include "scene/node.h"
#include "system.h"

bool StateMachine::s_SM_isBlendingEnabled = true;
float StateMachine::s_SM_bulletTimeTrackTransitionTimePgt = 0.01f;

StateMachine::StateMachine() {}
// @0x58b808: the effects of the tracks stop, and no effect follows the model any more
StateMachine::~StateMachine() {
    SM_ResetStateTracks();
    if (EffectMgr *mgr = EffectMgr::GetInstance()) mgr->OnModelChanged(m_model, 0);
}

int StateMachine::SM_GetEndAnimTime() const { return m_model->CC().GetEnd(); }
int StateMachine::SM_GetStartAnimTime() const { return m_model->CC().GetLoopStart(); }
int StateMachine::SM_GetAnimTime() const { return m_model->CC().GetTime(); }
int StateMachine::SM_GetAnimOldTime() const { return m_model->CC().GetPrevTime(); }
int StateMachine::SM_GetAnimTotalTime() const { return m_model->CC().GetTotalTime(); }

// @0x523324: the model it animates, its first state set and the state to start in
void StateMachine::SM_Init(jet::scene::Model *model, const clara::Param::StateValue &v) {
    m_model = model;
    SM_AddSateSet(v.a);
    m_setIdx = 0;
    m_changeDepth = 0;
    m_stateIdx = -1;
    const int idx = m_stateSets[0] ? m_stateSets[0]->FindStateIdxByName(v.b) : -1;
    if (idx >= 0 && model) SM_SetState(m_stateSets[0]->GetState((unsigned)idx)->m_index);
}

// @0x5233c4: the set (once) by its name from the database
void StateMachine::SM_AddSateSet(const jet::String &name) {
    for (const ActorStateSet *s : m_stateSets)
        if (s && s->GetName().Equals(name)) return;
    const StateSetData *d = StateDatabase::GetInstance()->FindStateSetDataByName(name);
    m_stateSets.push_back(d ? ActorStateSet::GetStateSetFromCollection(d) : 0);
}

// @0x52348c: after a state, its trigger without event, else the first state of label 1; empty states are left at once where the set says so
void StateMachine::SM_NextState() {
    for (;;) {
        ActorState *prev = m_state;
        ActorState *next = SM_GetNextState(prev);
        if (next) SM_SetState(next->m_index);
        else SM_SetStateLabel(1);
        const ActorStateSet *set = SM_CurrentSet();
        if (!set || !set->m_skipEmptyStates || prev == m_state || m_state->m_animStartTime != m_state->m_animEndTime) return;
    }
}

// @0x52408c: the state a trigger without event leads to, and when it has none the one before in the stack, and so on
ActorState *StateMachine::SM_GetNextState(ActorState *from) {
    for (;;) {
        const int idx = from->GetNextStateIndex(SM_CurrentSet(), m_animatedCostume);
        if (idx >= 0)
            if (ActorState *s = SM_CurrentSet()->GetState((unsigned)idx)) return s;
        if (m_stateStack.empty()) return 0;
        from = m_stateStack.back();
        m_stateStack.pop_back();
    }
}

// @0x5240f8: a selector goes to the top of the stack (once)
void StateMachine::SM_AddToStack(ActorState *s) {
    for (size_t i = 0; i < m_stateStack.size(); i++)
        if (m_stateStack[i] == s) {
            m_stateStack.erase(m_stateStack.begin() + (long)i + 1, m_stateStack.end());
            return;
        }
    m_stateStack.push_back(s);
}

// @0x524258
bool StateMachine::SM_SetState(int idx) {
    if (!SM_CanChangeState() || idx < 0) return false;
    ActorState *cur = m_state, *next = cur;
    if (idx != m_stateIdx) next = SM_GetState(idx);
    if (!next) return false;
    if (next->IsSelector()) {
        ActorState *pick = next->GetSelectedState(SM_CurrentSet(), 0);
        if (!pick) return false;
        SM_AddToStack(next);
        return SM_SetState(pick->m_index);
    }
    const int filtered = SM_GetFilteredState(idx);
    m_changeDepth++;
    SM_OnChangeState(cur, next);
    m_changeDepth--;
    if (filtered == m_stateIdx) m_model->CC().SetFinished(true);
    const jet::String &animName = next->SetRandomAnim(0);
    if (animName.empty()) return false;
    jet::anim::Animation *anim = jet::anim::AnimationLoader::GetInstance()->Load(animName);
    short blend = next->m_blendTime;
    bool doBlend = blend > 0;
    if (blend < 0 || filtered == m_stateIdx || !s_SM_isBlendingEnabled) {
        blend = 0;
        doBlend = false;
    }
    m_state = next;
    m_stateIdx = filtered;
    m_model->AnimFlags() = doBlend ? (m_model->AnimFlags() | 0x80) : (m_model->AnimFlags() & 0x7f);
    m_model->SetAnimation(anim, blend);
    m_model->CC().SetLooping(next->m_loop);
    m_model->CC().SetSpeedFixed((int)std::floor(next->m_animSpeed * 65536.0f + 0.5f));
    SM_OnChangeAnim(cur);
    m_queuedTriggers.clear();
    if (next->m_animStartTime >= 0 && next->m_animEndTime >= 0) {
        m_model->CC().SetLoop(next->m_animStartTime, next->m_animEndTime);
        m_model->CC().SetTime(next->m_animStartTime);
    }
    if (m_model->GetAnimation()) m_model->AnimateNodes(true);
    SM_UpdateStateTracks();
    return true;
}

// @0x524d6c: a state of the current set by name; its label is taken over by the linked machine
bool StateMachine::SM_SetState(const jet::String &name) {
    const int idx = FindStateIdxByName(name);
    if (idx >= 0 && m_linked)
        if (ActorState *s = SM_GetState(idx))
            if (s->m_label >= 0) m_linked->SM_SetStateLabel(s->m_label);
    return SM_SetState(idx);
}

// @0x523504
void StateMachine::SM_Update() {
    SM_UpdateQueuedEvents();
    if (m_state) {
        const int now = SM_GetAnimTime(), old = SM_GetAnimOldTime();
        for (size_t i = 0; i < m_queuedTriggers.size(); i++) {
            const StateTrigger *t = m_queuedTriggers[i];
            if (now < t->delay && old <= now) continue;
            if (SM_ExecuteTrigger(t, true)) break;
        }
        jet::scene::Model::Controller &cc = m_model->CC();
        if (!cc.IsFinished()) {
            if (m_state->m_loopStartTime >= 0 && cc.IsLooping() && cc.GetLoopStart() != m_state->m_loopStartTime && m_state->m_loopStartTime < SM_GetAnimTime()) {
                if (m_state->m_loopCount > 0) cc.SetLoopCount(m_state->m_loopCount);
                cc.SetLoop(m_state->m_loopStartTime, m_state->m_loopEndTime);
            }
        } else if (SM_CanChangeState()) {
            cc.SetFinished(false);
            if (!m_state->m_loop || (m_state->m_loopCount <= cc.GetLoopCount() && m_state->m_loopCount > 0)) SM_NextState();
            m_queuedTriggers.clear();
        }
    }
    SM_UpdateStateTracks();
}

// @0x524514: an input waits for a state that takes it, except a release (code 2), which is only taken when it is there
bool StateMachine::SM_OnInputEvent(int code, int param, bool pressed) {
    if (pressed && code != 2) {
        m_queuedEvents.push_back({ 0, code, param, jet::System::GetTime() });
        return true;
    }
    return SM_OnStateEvent(0, code, param);
}

// @0x52457c
bool StateMachine::SM_OnScreenEvent(int code, int param, bool pressed) {
    if (!pressed) return SM_OnStateEvent(2, code, param);
    if (!m_queuedEvents.empty() && m_queuedEvents[0].code != code) m_queuedEvents.clear();
    m_queuedEvents.push_back({ 2, code, param, jet::System::GetTime() });
    return true;
}

// @0x524634
bool StateMachine::SM_OnEngineEvent(int code) { return SM_OnStateEvent(1, code, 0); }

// @0x524644: the trigger of the state for the event that is valid in the time of the animation and whose conditions hold
int StateMachine::SM_FindTriggerIdxByEvent(int kind, int code, int param) {
    const int now = SM_GetAnimTime();
    const std::vector<StateTrigger> &triggers = m_state->GetStateData()->triggers;
    if (getenv("JET_DBGCOND")) fprintf(stderr, "findtrig kind=%d code=%d param=%d now=%d state=%s n=%d\n", kind, code, param, now, m_state->GetName().c_str(), (int)triggers.size());
    for (unsigned i = 0; i < triggers.size(); i++) {
        const StateTrigger &t = triggers[i];
        if (getenv("JET_DBGCOND")) fprintf(stderr, "  t kind=%d code=%d param=%d start=%d end=%d target=%d\n", t.kind, t.code, t.param, t.start, t.end, t.target);
        if (SM_GetState(t.target) && t.kind == kind && t.code == code && t.param == param && t.start >= 0 && t.start <= now && (now <= t.end || t.end < 0) &&
            ActorState::CheckTriggerConditions(&t, m_animatedCostume))
            return (int)i;
    }
    return -1;
}

// @0x524748
bool StateMachine::SM_OnStateEvent(int kind, int code, int param) {
    SM_SwitchStateEvent(&kind, &code, &param);
    if (m_linked) m_linked->SM_OnStateEvent(kind, code, param);
    const int idx = SM_FindTriggerIdxByEvent(kind, code, param);
    if (idx < 0) return false;
    const StateTrigger *t = &m_state->GetStateData()->triggers[(unsigned)idx];
    if (SM_GetAnimTime() < t->delay) m_queuedTriggers.push_back(t);
    else SM_ExecuteTrigger(t, false);
    return true;
}

// @0x524850
bool StateMachine::SM_ExecuteTrigger(const StateTrigger *t, bool checkConditions) {
    if (!t->hasTarget || (checkConditions && !ActorState::CheckTriggerConditions(t, m_animatedCostume))) return false;
    SM_SetState(t->target);
    return true;
}

// @0x5248a8: every machine linked goes to the first state of the label in its set
void StateMachine::SM_SetStateLabel(int label) {
    for (StateMachine *m = this; m; m = m->m_linked) {
        const ActorStateSet *set = m->SM_CurrentSet();
        if (set)
            if (ActorState *s = set->GetStateFromLabel(label)) m->SM_SetState(s->m_index);
    }
}

// @0x524908
bool StateMachine::SM_HasStateLabel(int label, int) const {
    const ActorStateSet *set = SM_CurrentSet();
    return set && set->GetStateFromLabel(label);
}

// @0x52494c: the animation goes on in another model
void StateMachine::SM_ChangeModel(jet::scene::Model *model) {
    jet::scene::Model::Controller &old = m_model->CC();
    jet::anim::Animation *anim = m_model->GetAnimation();
    m_model = model;
    jet::scene::Model::Controller &cc = model->CC();
    model->SetAnimation(anim, 0);
    if (cc.IsActive() != old.IsActive()) cc.SetActive(old.IsActive());
    cc.SetLoop(old.GetLoopStart(), old.GetEnd());
    cc.SetTime(old.GetTime());
    cc.SetTotalTime(old.GetTotalTime());
    cc.SetPrevTime(old.GetPrevTime());
    cc.SetLooping(old.IsLooping());
    cc.SetFinished(old.IsFinished());
    cc.SetLoopCount(old.GetLoopCount());
    cc.SetSpeedFixed(old.GetSpeedFixed());
}

// @0x524a0c: events that waited too long are dropped; the ones a trigger of the state takes are taken
void StateMachine::SM_UpdateQueuedEvents() {
    if (m_queuedEvents.empty()) return;
    const long long now = jet::System::GetTime();
    ActorState *state = m_state;
    const int animTime = SM_GetAnimTime();
    for (size_t i = 0; i < m_queuedEvents.size(); i++)
        if (now - m_queuedEvents[i].time > m_queuedEventsTime) m_queuedEvents.erase(m_queuedEvents.begin() + (long)i--);
    for (size_t e = 0; e < m_queuedEvents.size(); e++) {
        const StateMachineEvent ev = m_queuedEvents[e];
        for (const StateTrigger &t : state->GetStateData()->triggers) {
            if (t.kind == ev.kind && t.code == ev.code && t.param == ev.param && t.start >= 0 && t.start <= animTime && (animTime <= t.end || t.end < 0) &&
                ActorState::CheckTriggerConditions(&t, m_animatedCostume)) {
                SM_OnStateEvent(ev.kind, ev.code, ev.param);
                // the event taken (the first of the queue, as it came) is removed; one input to the state goes with the engine event of it
                while (!m_queuedEvents.empty()) {
                    const StateMachineEvent first = m_queuedEvents.front();
                    m_queuedEvents.erase(m_queuedEvents.begin());
                    if (first.kind == ev.kind && first.code == ev.code && first.param == ev.param && first.time == ev.time) break;
                }
                if (ev.kind == 0 && ev.code == 1)
                    for (size_t k = 0; k < m_queuedEvents.size(); k++)
                        if (m_queuedEvents[k].kind == 0 && m_queuedEvents[k].param == ev.param) m_queuedEvents.erase(m_queuedEvents.begin() + (long)k--);
                return;
            }
        }
    }
}

// @0x524cf4
void StateMachine::SM_SetStateSet(const jet::String &name, bool toLabel1) {
    for (size_t i = 0; i < m_stateSets.size(); i++)
        if (m_stateSets[i]->GetName().Equals(name)) {
            m_stateIdx = -1;
            m_setIdx = (int)i;
            m_stateStack.clear();
            if (toLabel1) SM_SetStateLabel(1);
            return;
        }
}

// @0x5139e8 / @0x523f08: what the tracks of the state do while the animation plays. Their effects and sounds come with those managers; the time
// speed of a bullet time track is followed here.
void StateMachine::SM_UpdateStateTracks() {
    ActorState *state = m_state;
    if (!state) return;
    const int prev = SM_GetAnimOldTime();
    if (m_trackSetIdx == m_setIdx && m_trackStateIdx == m_stateIdx && m_trackTime == prev) return;
    const int cur = SM_GetAnimTime(), loopStart = SM_GetStartAnimTime(), loopEnd = SM_GetEndAnimTime();
    m_trackTime = prev;
    m_trackSetIdx = m_setIdx;
    m_trackStateIdx = m_stateIdx;
    // @0x58c0d4: the effects of the state start when their event is on, at its node of the model (from the start of the event only)
    for (unsigned i = 0; i < state->m_effectConfig.size(); i++) {
        const EffectConfig &cfg = state->m_effectConfig[i];
        if (!cfg.event->IsOnTime(prev, cur, loopStart, loopEnd)) continue;
        if (cfg.event->duration != 0 && prev > cfg.event->time) continue;
        jet::scene::Dummy *dummy = m_model->FindDummyByName(cfg.node, 0);
        SM_AddEffectEvent(state, &cfg, 0, (int)i, dummy ? dummy->GetNode() : 0, cur);
    }
    if (state->m_bulletTimeTrack) {
        for (const StateEvent &e : state->m_bulletTimeTrack->events) {
            if (!e.IsOnTime(prev, cur, loopStart, loopEnd)) continue;
            const int time = (int)((float)e.duration * s_SM_bulletTimeTrackTransitionTimePgt);
            const int abs = time < 0 ? -time : time;
            if (e.time < prev || cur < e.time) {
                const int d = e.duration < 0 ? -e.duration : e.duration;
                const int end = e.time + d - abs;
                if (end < prev || cur < end) continue;
                Game::GetInstance()->SetTimeSpeed(1.0f, time, -1);
            } else
                Game::GetInstance()->SetTimeSpeed((float)e.args[0] * 0.01f, time, -1);
        }
    }
    if (state->m_musicVolumeTrack && m_tracksEnabled) {
        const StateEvent *e = state->m_musicVolumeTrack->FindEventByTime(prev, cur, loopStart, loopEnd);
        if (e && prev <= e->time && e->time <= cur) {
            // the volume of the music group (SoundMgr) is set to args[0] percent of the saved one, when that manager comes
        }
    }
}

// @0x523f08 and what the base does when a state is left (@0x4b04a4): a bullet time ends with the state
void StateMachine::SM_ResetStateTracks() {
    // @0x58c580: the effects stop (each tells the state machine, which lets go of it)
    while (!m_effects.empty()) {
        EffectControl *ec = m_effects.front();
        if (EffectMgr *mgr = EffectMgr::GetInstance()) mgr->Stop(ec);
        if (!m_effects.empty() && m_effects.front() == ec) m_effects.erase(m_effects.begin());
    }
    if (m_state && m_state->m_bulletTimeTrack && Game::GetInstance() && Game::GetInstance()->GetTimeSpeed() != 1.0f) Game::GetInstance()->SetTimeSpeed(1.0f, 0, -1);
}
void StateMachine::SM_OnChangeState(ActorState *, ActorState *) { SM_ResetStateTracks(); }

// @0x589824
bool StateMachine::SM_RemoveEffect(EffectControl *ec) {
    std::vector<EffectControl*>::iterator it = std::find(m_effects.begin(), m_effects.end(), ec);
    if (it == m_effects.end()) return false;
    m_effects.erase(it);
    return true;
}

// @0x58bcf0: the effect is found by its name in the project
void StateMachine::SM_AddEffectEvent(ActorState *state, const EffectConfig *cfg, int a, int b, const jet::scene::Node *node, int c) {
    if (!cfg) return;
    EffectDef *def = dynamic_cast<EffectDef*>(clara::Project::GetInstance()->FindEntityByName(cfg->effect));
    if (def) SM_AddEffectEvent(state, cfg->event, def, a, b, node, c);
}

// @0x58bd6c
void StateMachine::SM_AddEffectEvent(ActorState *state, const StateEvent *event, const EffectDef *def, int a, int b, const jet::scene::Node *node, int c) {
    // who started it: the state machine, the state, and a and b (the number of the effect of the state)
    const int low = ((a << 8) & 0xffff) | (b & 0xff) | ((int)state->m_index << 16);
    const unsigned high = SM_GetId() | (unsigned)(low >> 31);
    const long long key = (long long)(((unsigned long long)high << 32) | (unsigned)low);
    for (size_t i = 0; i < m_effects.size(); i++)
        if (m_effects[i]->tag == key) return;
    const jet::scene::Node *at = node ? node : &m_model->GetRootNode();
    const math::vec3<float> position = at->GetAbsolutePosition();
    const math::quat<float> rotation = at->GetAbsoluteRotation();
    const math::vec3<float> scale = SM_GetScale();
    // the time it plays: what is left of the event; an event without duration plays for ever, where it started
    int lifeTime = -1;
    bool isOnTime = false;
    if (event->duration != 0) {
        lifeTime = event->duration + (event->time - c);
        if (lifeTime >= 0 && state->m_loop) isOnTime = event->IsOnTime(state->m_loopStartTime, state->m_loopEndTime, state->m_loopStartTime, state->m_loopEndTime);
    }
    if (node && lifeTime == -1) node = 0;
    const bool endsAfterRange = lifeTime >= 0 && event->duration + event->time >= state->m_animStartTime + state->m_animEndTime;
    const bool autoRemove = !endsAfterRange && !isOnTime;
    if (!autoRemove) lifeTime = -1;
    EffectMgr *mgr = EffectMgr::GetInstance();
    if (!mgr || !mgr->Add(m_effectLayerId, def, position, rotation, scale, autoRemove, node, lifeTime, m_effectsEnabled, this)) return;
    EffectControl *ec = mgr->GetLastAdded();
    ec->tag = key;
    m_effects.push_back(ec);
}

// source/game/actors/statemachine/StateMachine.h
#ifndef SOURCE_GAME_ACTORS_STATEMACHINE_STATEMACHINE_H
#define SOURCE_GAME_ACTORS_STATEMACHINE_STATEMACHINE_H

#include <vector>
#include "param.h"
#include "ActorStateSet.h"
#include "game/dynamicobjects/EffectControl.h"

namespace jet::scene { class Model; class Node; }
class EffectDef;

// An event waiting for a state that takes it: kind (0 input, 1 engine, 2 screen), code, param and when it came (ms)
struct StateMachineEvent { int kind, code, param; long long time; };

// Drives the animation of an actor from its state sets (StateMachine @0x52312c): a state plays its animation on the model; when it ends,
// or an input, engine or screen event matches one of its triggers, the next state follows. A state without trigger returns to the first state
// of label 1. The actor decides (the virtuals) what a change means to it.
class StateMachine : public EffectControlListener {
public:
    StateMachine();
    virtual ~StateMachine();
    void SM_Init(jet::scene::Model*, const clara::Param::StateValue&);
    void SM_AddSateSet(const jet::String&);
    void SM_Reset() { m_queuedTriggers.clear(); m_queuedEvents.clear(); m_stateStack.clear(); }  // @0x523470
    void SM_Update();
    void SM_NextState();
    ActorState *SM_GetNextState(ActorState*);
    virtual bool SM_SetState(int);
    bool SM_SetState(const jet::String&);
    ActorState *SM_GetState(int idx) const { const ActorStateSet *s = SM_CurrentSet(); return s ? s->GetState((unsigned)idx) : 0; }
    ActorState *SM_GetState() const { return m_state; }
    int SM_GetStateIdx() const { return m_stateIdx; }
    void SM_SetStateSet(const jet::String&, bool toLabel1);
    void SM_SetStateLabel(int label);
    void SM_SetStateLabel(const jet::String &name) { (void)name; SM_SetStateLabel(-1); }
    bool SM_HasStateLabel(int label, int) const;
    int FindStateIdxByName(const jet::String &n) const { const ActorStateSet *s = SM_CurrentSet(); return s ? s->FindStateIdxByName(n) : -1; }
    void SM_ChangeModel(jet::scene::Model*);
    void SM_LinkTo(StateMachine *o) { m_linked = o; }
    void SM_SetAnimatedCostume(MinionCostume *c) { m_animatedCostume = c; }
    MinionCostume *SM_GetAnimatedCostume() const { return m_animatedCostume; }
    jet::scene::Model *SM_GetModel() const { return m_model; }
    void SM_SetQueuedEventsTime(int ms) { m_queuedEventsTime = ms; }
    // events: true when a trigger of the state took it (or it is waiting for the right time)
    virtual bool SM_OnInputEvent(int code, int param, bool pressed);
    virtual bool SM_OnEngineEvent(int code);
    virtual bool SM_OnScreenEvent(int code, int param, bool pressed);
    bool SM_ExecuteTrigger(const StateTrigger*, bool checkConditions);
    static void SM_SetBlendingEnabled(bool b) { s_SM_isBlendingEnabled = b; }
    // GameplayDef: how much of a bullet time track the transitions take
    static void SetBulletTimeTrackTransitionTimePgt(float f) { s_SM_bulletTimeTrackTransitionTimePgt = f; }
    // @0x58c4fc, @0x58c500: the effect the states started ended or was released
    virtual void ECL_OnStop(EffectControl *ec) { SM_RemoveEffect(ec); }
    virtual void ECL_OnRelease(EffectControl *ec) { SM_RemoveEffect(ec); }
    const ActorStateSet *SM_CurrentSet() const { return m_setIdx >= 0 && (unsigned)m_setIdx < m_stateSets.size() ? m_stateSets[(unsigned)m_setIdx] : 0; }
protected:
    // what a change of state means to the actor
    virtual void SM_OnChangeState(ActorState *from, ActorState *to);
    virtual void SM_OnChangeAnim(ActorState*) {}
    virtual void SM_SwitchStateEvent(int*, int*, int*) {}
    virtual int SM_GetFilteredState(int idx) { return idx; }
    virtual bool SM_CanChangeState() { return true; }
    virtual unsigned SM_GetId() { return 0; }
    virtual math::vec3<float> SM_GetScale() { return math::vec3<float>(1, 1, 1); }
    virtual void SM_UpdateStateTracks();
    int SM_GetEndAnimTime() const;
    int SM_GetStartAnimTime() const;
    int SM_GetAnimTime() const;
    int SM_GetAnimOldTime() const;
    int SM_GetAnimTotalTime() const;
    void SM_ResetStateTracks();
    // the sounds, music and effects of the tracks only play where the owner is shown
    void SetTracksEnabled(bool sounds, bool effects) { m_tracksEnabled = sounds; m_effectsEnabled = effects; }
    // @0x58d420 stops the sounds the state tracks started with the SoundMgr; none are started here (no SoundMgr)
    void SM_StopAllSounds() {}
    // @0x589824: the effect is not one of the state machine's any more
    bool SM_RemoveEffect(EffectControl*);
    // @0x58bcf0, @0x58bd6c: the effect of an EFFECT_ track event starts at the node (once per state, track and event: the control is tagged with their key)
    void SM_AddEffectEvent(ActorState*, const EffectConfig*, int a, int b, const jet::scene::Node*, int c);
    void SM_AddEffectEvent(ActorState*, const StateEvent*, const EffectDef*, int a, int b, const jet::scene::Node*, int c);
    int SM_FindTriggerIdxByEvent(int kind, int code, int param);
    bool SM_OnStateEvent(int kind, int code, int param);
    void SM_UpdateQueuedEvents();
    void SM_AddToStack(ActorState*);
    jet::scene::Model *m_model = 0;
    std::vector<const ActorStateSet*> m_stateSets;
    int m_setIdx = 0, m_stateIdx = -1;
    ActorState *m_state = 0;
    std::vector<ActorState*> m_stateStack;
    std::vector<const StateTrigger*> m_queuedTriggers;
    std::vector<StateMachineEvent> m_queuedEvents;
    std::vector<EffectControl*> m_effects;  // 0x64: the effects the tracks of the states started
    int m_queuedEventsTime = 100;
    StateMachine *m_linked = 0;
    MinionCostume *m_animatedCostume = 0;
    int m_changeDepth = 0;           // 0x54: while a change of state is being handled
    int m_trackSetIdx = -1, m_trackStateIdx = -1, m_trackTime = -2;  // where the state tracks were last looked at
    bool m_tracksEnabled = true;     // 0x88: the music and the sounds of the tracks
    bool m_effectsEnabled = true;    // 0x89: the effects of the tracks
    int m_effectLayerId = 0;         // 0x8C: the layer the state's effects play in
    static bool s_SM_isBlendingEnabled;
    static float s_SM_bulletTimeTrackTransitionTimePgt;
};

#endif

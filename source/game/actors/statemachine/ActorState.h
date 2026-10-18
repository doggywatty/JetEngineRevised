// source/game/actors/statemachine/ActorState.h
#ifndef SOURCE_GAME_ACTORS_STATEMACHINE_ACTORSTATE_H
#define SOURCE_GAME_ACTORS_STATEMACHINE_ACTORSTATE_H

#include <vector>
#include "StateDatabase.h"

class ActorStateSet;
class MinionCostume;

// A sound a state plays on one of its SOUNDS_ tracks, an effect on an EFFECT_ track (node: where it is attached)
struct SoundConfig { jet::String name; int track; bool loop; };
struct EffectConfig { jet::String effect, node; const StateEvent *event; };

// A state of an actor (ActorState @0x52131c): the data of its state plus what the animation needs of it -- the range, loop and blend of
// the animation (the "Range", "Loop" and "Blend" tracks), and the special tracks (EFFECT_, SOUNDS_, STEPS_, ...) by their prefixes.
class ActorState {
public:
    ActorState(const StateData*, int index);
    ~ActorState();
    // the animation of the state (a selector has none)
    const jet::String &SetRandomAnim(int) { return m_data->anims.empty() ? jet::String::null : m_data->anims[0].name; }
    bool IsSelector() const { return m_data->isSelector; }
    // a selector picks one of its states by weight; the picks are remembered to follow the repeat behaviour (0 free, 1 never twice in a
    // row, 2 each once before one repeats)
    ActorState *GetSelectedState(const ActorStateSet*, int timeInState);
    // the triggers without event (code and param -1) that lead on when the state is entered, one of them by chance
    int GetNextStateIndex(const ActorStateSet*, MinionCostume*) const;
    bool HasInputTrigger(int code, int param) const { return HasTrigger(0, code, param); }
    bool HasScreenTrigger(int code, int param) const { return HasTrigger(2, code, param); }
    bool HasEngineTrigger(int code) const;
    // the range of the stretched animation (the "stretch" track, else the state's range inside the animation's duration)
    void GetStretchTime(float &start, float &end, unsigned animDuration) const;
    // true when every condition of the trigger holds
    static bool CheckTriggerConditions(const StateTrigger*, MinionCostume*);
    // "<prefix>,<a>,<b>": the field after index pipes and comma fields
    static jet::String GetSubstringName(const char *src, int field);
    const StateData *GetStateData() const { return m_data; }
    const jet::String &GetName() const { return m_data->name; }
    short GetIndex() const { return m_index; }
    // the kinds of states by their StateType (@0x560e18..0x560fcc, @0x5897a8)
    bool IsDodgeLeftState() const;
    bool IsDodgeRightState() const;
    bool IsAnyDespicableEndState() const;
    bool IsAnySledgeInitState() const;
    bool IsAnyCrashState() const;
    bool IsGravityState() const;
    bool IsMountDisengageState() const;
    bool IsAnyJumpingState() const;
    bool IsShadowState() const;
    bool IsDieState() const;

    const StateData *m_data;
    short m_index;
    short m_label;                    // StateLabel (1)
    short m_type;                     // StateType
    short m_animStartTime, m_animEndTime;  // Range (-1)
    short m_loopStartTime, m_loopEndTime, m_loopCount;  // Loop (-1)
    short m_blendTime;                // Blend (-1)
    bool m_loop;
    float m_animSpeed;
    bool m_allowDodgeByInput, m_isCinematicCameraEnabled;
    short m_cinematicCamTransition;
    std::vector<SoundConfig*> m_soundConfig;
    std::vector<EffectConfig> m_effectConfig;
    const StateTrack *m_attackTrack, *m_stepsTrack, *m_landTrack, *m_sledgeTrack, *m_giantScaleTrack, *m_bulletTimeTrack, *m_musicVolumeTrack, *m_blindBoxTrack;
    jet::String m_dirBlendR, m_dirBlendL;
    int m_selectorRepeatBehavior;
    unsigned m_selectorLastSelected;  // bits of the entries picked
private:
    bool HasTrigger(int kind, int code, int param) const;
};

#endif

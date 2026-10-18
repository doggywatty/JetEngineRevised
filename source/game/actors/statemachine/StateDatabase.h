// source/game/actors/statemachine/StateDatabase.h
#ifndef SOURCE_GAME_ACTORS_STATEMACHINE_STATEDATABASE_H
#define SOURCE_GAME_ACTORS_STATEMACHINE_STATEDATABASE_H

#include <vector>
#include "core/pstring.h"

namespace jet::stream { class IStream; }

// The actor state machines ("states" datalib; StateDatabase::Load @0x644160,
// StateSetData::Load @0x643e10, StateData::Load @0x6432e4, StateTrack::Load @0x642e90).
// Strings are u16 length + bytes.
//   u16 sets; set: str name, u16 n (s32 + str) pairs, u16 states, states
//   state: str name, u8 selector
//     selector: u16 entries (str, s32, f32, s32), u16 params (str, s32), u16 triggers
//     else: u16 params (str, s32), u16 triggers, u16 anims, u16 animsets (str, s16, s16)
//   trigger: 6 x s32, u8 named [str], u16 conditions (str, s32)
//   anim: str, s16, u16 tracks; track: str, u16 events, s32
//   event: s32, s32, s32 n, n x s32, u8 named [str]
struct StateParam { jet::String name; int value; };
struct StateTriggerCondition { jet::String name; int value; };
// a way out of a state (StateData::Load @0x58ab64): the event that takes it (kind 0 input, 1 engine, 2 screen; code and param say which),
// the time window of the state it is valid in (start..end, -1 open) and the delay it waits when taken early; code and param -1 make it
// an entry trigger
struct StateTrigger {
    int code, kind, param;    // 0x0, 0x4, 0x8
    int extra;                // 0x18
    int start, end, delay;    // 0xc, 0x10 (-1 until its TRIGGER_ track sets it), 0x14
    bool hasTarget;           // 0x1c
    int target;               // 0x20: the state it leads to, -1 for none
    jet::String targetName;   // 0x24, until StateData::Init finds the index
    std::vector<StateTriggerCondition> conditions;
};
struct StateEvent {
    int time, duration;       // a negative duration counts as its absolute value
    std::vector<int> args;
    jet::String name;
    // @0x500a70: does the event lie in the time the animation moved from prev to cur (wrapping through the loop)
    bool IsOnTime(int prev, int cur, int loopStart, int loopEnd) const;
};
struct StateTrack {
    jet::String name;
    std::vector<StateEvent> events;
    int end;
    int FindEventIdxByTime(int prev, int cur, int loopStart, int loopEnd, int nth = 0) const;
    const StateEvent *FindEventByTime(int prev, int cur, int loopStart, int loopEnd) const;
    const StateEvent *FindLastEvent() const;
};
struct StateAnim { jet::String name; short set; std::vector<StateTrack> tracks; };
struct StateAnimSet { jet::String name; short first; short count; };
// a state a selector picks: chance weight, and after threshold ms in the state the weight loses factor per ms
struct SelectorEntryData { jet::String name; int weight; float factor; int threshold; int state; };

struct StateData {
    jet::String name;
    bool isSelector;
    std::vector<SelectorEntryData> entries;
    std::vector<StateParam> params;
    std::vector<StateTrigger> triggers;
    std::vector<StateAnim> anims;
    std::vector<StateAnimSet> animSets;
    int GetParam(const char *name, int def = 0) const;
    float GetParamFloat(const char *name, float def) const;  // fixed point: 5 bits of shift over a 27 bit value
    const StateTrack *FindTrack(unsigned anim, const char *name) const;
    int FindTrackIdxByName(unsigned anim, const char *name) const;
    void Init(const struct StateSetData&);
};

struct StateSetData {
    jet::String name;
    std::vector<StateData> states;
    int FindStateIdxByName(const jet::String&) const;
};

class StateDatabase {
public:
    static StateDatabase *GetInstance();
    bool Load(const jet::String &name);
    const StateSetData *FindStateSetDataByName(const jet::String&) const;
    unsigned GetStateSetCount() const { return (unsigned)m_sets.size(); }
    const StateSetData *GetStateSet(unsigned i) const { return &m_sets[i]; }
private:
    std::vector<StateSetData> m_sets;
};

#endif

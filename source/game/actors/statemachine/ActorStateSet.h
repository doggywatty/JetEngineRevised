// source/game/actors/statemachine/ActorStateSet.h
#ifndef SOURCE_GAME_ACTORS_STATEMACHINE_ACTORSTATESET_H
#define SOURCE_GAME_ACTORS_STATEMACHINE_ACTORSTATESET_H

#include <vector>
#include "ActorState.h"

// The states of a state set, made once for every actor that uses it (@0x522e98). The first state of each label is kept for SM_SetStateLabel.
class ActorStateSet {
public:
    static const int k_stateLabel_max = 39;
    explicit ActorStateSet(const StateSetData*);
    ~ActorStateSet();
    const jet::String &GetName() const { return m_data->name; }
    const StateSetData *GetStateSetData() const { return m_data; }
    unsigned GetStateCount() const { return (unsigned)m_states.size(); }
    ActorState *GetState(unsigned i) const { return i < m_states.size() ? m_states[i] : 0; }
    ActorState *FindStateByName(const jet::String &n) const { return GetState((unsigned)m_data->FindStateIdxByName(n)); }
    int FindStateIdxByName(const jet::String &n) const { return m_data->FindStateIdxByName(n); }
    const jet::String &GetStateName(unsigned i) const { return m_data->states[i].name; }
    ActorState *GetStateFromLabel(int label) const;
    // true: a state whose range is empty is left at once for the next
    bool m_skipEmptyStates = false;  // 0x14
    // the set made from the data (made once)
    static ActorStateSet *GetStateSetFromCollection(const StateSetData*);
    static void ClearStateSetCollection();
private:
    void InitLabelList();
    const StateSetData *m_data;
    std::vector<ActorState*> m_states;
    int m_labelStates[k_stateLabel_max];
    static std::vector<ActorStateSet*> s_stateSetCollection;
};

#endif

// source/game/actors/statemachine/ActorStateSet.cpp
#include "ActorStateSet.h"
#include <cstring>

std::vector<ActorStateSet*> ActorStateSet::s_stateSetCollection;

// @0x522e98, Init @0x522f1c
ActorStateSet::ActorStateSet(const StateSetData *d) : m_data(d) {
    for (unsigned i = 0; i < d->states.size(); i++) m_states.push_back(new ActorState(&d->states[i], (int)i));
    InitLabelList();
}

ActorStateSet::~ActorStateSet() {
    for (ActorState *s : m_states) delete s;
}

// @0x522f84
void ActorStateSet::InitLabelList() {
    memset(m_labelStates, -1, sizeof(m_labelStates));
    for (unsigned i = 0; i < m_states.size(); i++) {
        const unsigned label = (unsigned)m_states[i]->m_label;
        if (label < (unsigned)k_stateLabel_max && m_labelStates[label] < 0) m_labelStates[label] = (int)i;
    }
}

ActorState *ActorStateSet::GetStateFromLabel(int label) const {
    if ((unsigned)label >= (unsigned)k_stateLabel_max || m_labelStates[label] < 0) return 0;
    return m_states[(unsigned)m_labelStates[label]];
}

// @0x522fe8
ActorStateSet *ActorStateSet::GetStateSetFromCollection(const StateSetData *d) {
    for (ActorStateSet *s : s_stateSetCollection)
        if (s->m_data == d) return s;
    s_stateSetCollection.push_back(new ActorStateSet(d));
    return s_stateSetCollection.back();
}

// @0x5230b4
void ActorStateSet::ClearStateSetCollection() {
    for (ActorStateSet *s : s_stateSetCollection) delete s;
    s_stateSetCollection.clear();
}

// framework/include/states/stateset.h
#ifndef FRAMEWORK_INCLUDE_STATES_STATESET_H
#define FRAMEWORK_INCLUDE_STATES_STATESET_H

class StateSet {  // sizeof 0x14
public:
    StateSet(const StateSetData*);
    virtual ~StateSet();
    virtual void Init();
    const jet::String &GetName();
    int FindStateIdxByName(const jet::String&);
    const jet::String &GetStateName(unsigned);
    unsigned GetStateCount();
    const StateSetData *GetStateSetData();
protected:
    const jet::String *m_name;  // 0x4
    const StateSetData *m_data;  // 0x8
    unsigned m_statesCount;  // 0xC
    State **m_states;  // 0x10
};

#endif

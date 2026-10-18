// framework/include/states/state.h
#ifndef FRAMEWORK_INCLUDE_STATES_STATE_H
#define FRAMEWORK_INCLUDE_STATES_STATE_H

class State {  // sizeof 0xC
public:
    State(const StateData*);
    virtual ~State();
    virtual void Init(const StateData*);
    virtual const jet::String &SetRandomAnim(int);
    virtual void RefreshData();
    const jet::String &GetName();
    const StateData *GetStateData();
    int GetTriggerCount();
    const StateTrigger *GetTrigger(int);
    int GetTrackCount();
    const StateTrack *GetTrack(int);
    const StateTrack *FindTrackByName(const jet::String&);
    int GetAnimIdx();
protected:
    int m_animIdx;  // 0x4
    const StateData *m_data;  // 0x8
};

#endif

// framework/include/states/statedatabase.h
#ifndef FRAMEWORK_INCLUDE_STATES_STATEDATABASE_H
#define FRAMEWORK_INCLUDE_STATES_STATEDATABASE_H

class StateData {  // sizeof 0x58
public:
    void Load(jet::stream::IStream&, StateSetData*);
    void Unload();
    StateData();
    ~StateData();
    void Init(StateSetData*);
    const jet::String &GetName();
    unsigned GetTriggerCount();
    const StateTrigger *GetTrigger(unsigned);
    bool HasStateResult(unsigned);
    int GetParam(const jet::String&, int);
    float GetParamFloat(const jet::String&, float);
    unsigned GetAnimCount();
    const jet::String &GetAnim(unsigned);
    unsigned GetAnimSetIdx(unsigned);
    const StateAnimSet &GetAnimSet(unsigned);
    unsigned GetTrackCount(unsigned);
    const StateTrack *GetTrack(unsigned, unsigned);
    int FindTrackIdxByName(unsigned, const jet::String&);
    const StateTrack *FindTrackByName(unsigned, const jet::String&);
    unsigned GetSelectorCount();
    const SelectorEntryData *GetSelectorEntry(unsigned);
    const StateSetData *GetStateSetData();
private:
    static const int k_storePrecision_bits;
    const jet::String *m_name;  // 0x0
    ustl::vector<StateTrigger> m_triggers;  // 0x4
    ustl::vector<StateParam> m_params;  // 0x14
    ustl::vector<StateAnim> m_animations;  // 0x24
    ustl::vector<StateAnimSet> m_animSets;  // 0x34
    ustl::vector<SelectorEntryData> m_selectorEntries;  // 0x44
    StateSetData *m_stateSetData;  // 0x54
};

class StateDatabase : public Singleton<StateDatabase> {  // sizeof 0x30
public:
    StateDatabase();
    virtual ~StateDatabase();
    void Load(const jet::String&);
    void Unload();
    unsigned GetStateSetCount();
    const StateSetData *FindStateSetDataByName(const jet::String&);
    const StateSetData *GetStateSetData(unsigned);
    int FindStateSetIdxByName(const jet::String&);
    void GarbageCollect();
    const jet::String *AddString(const jet::String&);
    void Debug_StatesStatistics();
    void Debug_CheckDataIntegrity();
private:
    ustl::vector<StateSetData> m_stateSets;  // 0x4
    boost::unordered::unordered_set<jet::String,boost::hash<jet::String>,std::equal_to<jet::String>,std::allocator<jet::String> > m_ttStringTable;  // 0x14
};

class StateSetData {  // sizeof 0x14
public:
    StateSetData();
    ~StateSetData();
    void Load(jet::stream::IStream&);
    void Unload();
    const jet::String &GetName();
    unsigned GetStateCount();
    const jet::String &GetStateName(unsigned);
    int FindStateIdxByName(const jet::String&);
    const StateData *GetStateData(unsigned);
private:
    const jet::String *m_name;  // 0x0
    ustl::vector<StateData> m_states;  // 0x4
};

class StateTrack {  // sizeof 0x18
public:
    StateTrack();
    ~StateTrack();
    void Load(jet::stream::IStream&);
    void Unload();
    const jet::String &GetName();
    unsigned GetEventsCount();
    const StateEvent *GetEvent(unsigned);
    int* GetEventValue(unsigned);
    int GetEventTime(unsigned);
    int GetEventDuration(unsigned);
    const StateEvent *FindLastEvent();
    const StateEvent *FindEventByTime(int, int);
    int* FindEventValueByTime(int, int, int*);
    int FindEventTimeByTime(int, int);
    int FindEventDurationByTime(int, int);
    int FindEventIdxByTime(int, int, int);
    int GetWeaponId();
private:
    const jet::String *m_name;  // 0x0
    ustl::vector<StateEvent> m_events;  // 0x4
    int m_weaponId;  // 0x14
};

#endif

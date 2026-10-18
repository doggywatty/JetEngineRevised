// source/game/online/events/EventsMgr.h
#ifndef SOURCE_GAME_ONLINE_EVENTS_EVENTSMGR_H
#define SOURCE_GAME_ONLINE_EVENTS_EVENTSMGR_H

class EventsMgr : public Singleton<EventsMgr> {  // sizeof 0x1C
public:
    EventsMgr();
    virtual ~EventsMgr();
    void Init();
    void Update(int);
    void StartLoadingEvents();
    int GetTotalEventTime();
    int GetCurrentEventTime();
    social::Event *GetCurrentEvent();
private:
    void SetCurrentEvent(social::Event*);
    static bool sOnEventsLoaded(void*, bool, social::OnlineEventData);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_eventsCategory_name;
    static const int k_initialRetryTimer;
    static const int k_maxRetryTime;
    static const int k_refreshTimeIfNoEvent;
    int m_numLoadingRetries;  // 0x4
    int m_loadingRetryTimer;  // 0x8
    int m_noEventRefreshTimer;  // 0xC
    bool m_isLoadingEvents;  // 0x10
    bool m_hasError;  // 0x11
    social::EventSearch *m_eventSearch;  // 0x14
    social::Event *m_currentEvent;  // 0x18
};

#endif

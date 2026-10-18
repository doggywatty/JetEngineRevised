// source/game/online/tracking/SessionTrackingMgr.h
#ifndef SOURCE_GAME_ONLINE_TRACKING_SESSIONTRACKINGMGR_H
#define SOURCE_GAME_ONLINE_TRACKING_SESSIONTRACKINGMGR_H

// Same gap as every other header in this tree before this session: zero includes in the PDB extraction.
#include <cstdint>
#include <map>
#include <string>
#include "jsingleton.h"
namespace jet::stream { class MemoryStream; }
namespace game::common { class TrackingSession; }

namespace game::common {
class SessionTrackingMgr : public Singleton<game::common::SessionTrackingMgr> {  // sizeof 0x24
public:
    SessionTrackingMgr();
    virtual ~SessionTrackingMgr();
    game::common::TrackingSession *GetSession(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    game::common::TrackingSession *AddSession(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool RemoveSession(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Serialize(jet::stream::MemoryStream&);
    void Deserialize(jet::stream::MemoryStream&);
    unsigned GetNumberSessions();
private:
    void SerializeV1(jet::stream::MemoryStream&);
    void DeserializeV1(jet::stream::MemoryStream&);
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,game::common::TrackingSession *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,game::common::TrackingSession *> > > m_trackingSessionsMap;  // 0x4
    unsigned m_numberSessions;  // 0x20
    static const int k_saveGameVersion;
    static const int k_signatureV1;
};
}

namespace game::common {
class TrackingSession {  // sizeof 0x40
public:
    TrackingSession(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    TrackingSession();
    ~TrackingSession();
    void Restart();
    void Update(float);
    void Suspend();
    void Pause();
    void Resume();
    float GetRunningTime();
    float GetStartTimeDiffInSecs();
    float GetTimeBetweenSessionsInSecs();
    float GetTimeBetweenSessionsInMins();
    unsigned GetSessionCount();
    void Serialize(jet::stream::MemoryStream&);
    void Deserialize(jet::stream::MemoryStream&);
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_key;  // 0x0
    int64_t m_startTimestamp;  // 0x20
    int64_t m_suspendTimestamp;  // 0x28
    float m_runningTime;  // 0x30
    unsigned m_sessionCount;  // 0x34
    bool m_isPaused;  // 0x38
};
}

#endif

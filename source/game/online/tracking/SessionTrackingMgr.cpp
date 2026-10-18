// source/game/online/tracking/SessionTrackingMgr.cpp
//
// Minimal, deliberately conservative port: just enough of the map-backed session store for
// GS_Gameplay/GS_Loading to call GetSession/AddSession without crashing. Serialize/Deserialize
// (save-game persistence) are not ported -- nothing in this tree calls them yet.
#include "SessionTrackingMgr.h"

namespace game::common {

SessionTrackingMgr::SessionTrackingMgr() : m_numberSessions(0) {}
SessionTrackingMgr::~SessionTrackingMgr() {}

TrackingSession *SessionTrackingMgr::GetSession(const std::string &key) {
    auto it = m_trackingSessionsMap.find(key);
    return it == m_trackingSessionsMap.end() ? 0 : it->second;
}

TrackingSession *SessionTrackingMgr::AddSession(const std::string &key) {
    TrackingSession *s = new TrackingSession(key);
    m_trackingSessionsMap[key] = s;
    m_numberSessions++;
    return s;
}

bool SessionTrackingMgr::RemoveSession(const std::string &key) {
    auto it = m_trackingSessionsMap.find(key);
    if (it == m_trackingSessionsMap.end()) return false;
    delete it->second;
    m_trackingSessionsMap.erase(it);
    m_numberSessions--;
    return true;
}

unsigned SessionTrackingMgr::GetNumberSessions() { return m_numberSessions; }

TrackingSession::TrackingSession(const std::string &key)
    : m_key(key), m_startTimestamp(0), m_suspendTimestamp(0), m_runningTime(0.0f), m_sessionCount(1), m_isPaused(false) {}
TrackingSession::TrackingSession() : m_startTimestamp(0), m_suspendTimestamp(0), m_runningTime(0.0f), m_sessionCount(0), m_isPaused(false) {}
TrackingSession::~TrackingSession() {}

void TrackingSession::Restart() { m_runningTime = 0.0f; m_sessionCount++; m_isPaused = false; }
void TrackingSession::Update(float dt) { if (!m_isPaused) m_runningTime += dt; }
void TrackingSession::Suspend() { m_isPaused = true; }
void TrackingSession::Pause() { m_isPaused = true; }
void TrackingSession::Resume() { m_isPaused = false; }
float TrackingSession::GetRunningTime() { return m_runningTime; }
float TrackingSession::GetStartTimeDiffInSecs() { return 0.0f; }
float TrackingSession::GetTimeBetweenSessionsInSecs() { return 0.0f; }
float TrackingSession::GetTimeBetweenSessionsInMins() { return 0.0f; }
unsigned TrackingSession::GetSessionCount() { return m_sessionCount; }

}

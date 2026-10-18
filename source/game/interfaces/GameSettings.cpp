// source/game/interfaces/GameSettings.cpp
#include "GameSettings.h"
#include "game/gamemanagers/COPPAMgr.h"

// @0x499b7c
bool GameSettings::LoadRecordDB(clara::RecordDB &db) {
    return db.Load() && !db.Get(jet::String("___was_deleted")).GetAsBool(false);
}

// @0x499bec
void GameSettings::SaveRecordDB(clara::RecordDB &db) {
    db.Remove(jet::String("___was_deleted"));
    db.Save();
}

// @0x499c3c
void GameSettings::DeleteRecordDB(clara::RecordDB &db) {
    db.Clear();
    db.Set(jet::String("___was_deleted"), clara::Record(true));
    db.Save();
}

// @0x499ddc
void GameSettings::LoadGameSettings() {
    m_gameSettings.SetName(jet::String("settings"));
    LoadRecordDB(m_gameSettings);
    // @0x50aed0: the age gate's record (the tracking managers' records, GameTrackingMgr and KhronoPlayPatternMgr, are not ported)
    if (COPPAMgr::HasInstance() && m_gameSettings.Get(COPPAMgr::k_saveRecordName).GetType() != clara::Record::TypeDef::NONE)
        COPPAMgr::GetInstance()->LoadFromRecordDB(m_gameSettings, COPPAMgr::k_saveRecordName, false);
    m_loaded = true;
}

// @0x499d00
void GameSettings::SaveGameSettings() {
    if (!m_loaded) return;
    m_gameSettings.SetName(jet::String("settings"));
    // @0x50adc4: the age gate's record (GameTrackingMgr's and KhronoPlayPatternMgr's are not ported)
    if (COPPAMgr::HasInstance()) COPPAMgr::GetInstance()->SaveToRecordDB(m_gameSettings, COPPAMgr::k_saveRecordName, false);
    SaveRecordDB(m_gameSettings);
}

// @0x499fc8
void GameSettings::LoadGameProfile() {
    m_gameProfile.SetName(jet::String("profile"));
    LoadRecordDB(m_gameProfile);
}

// @0x49a020
void GameSettings::SaveGameProfile() {
    m_gameProfile.SetName(jet::String("profile"));
    SaveRecordDB(m_gameProfile);
}

// @0x49a664
void GameSettings::LoadDebugSettings() {
    m_debugSettings.SetName(jet::String("debugSettings"));
    LoadRecordDB(m_debugSettings);
}

// @0x49a6bc
void GameSettings::SaveDebugSettings() {
    m_debugSettings.SetName(jet::String("debugSettings"));
    SaveRecordDB(m_debugSettings);
}

float GameSettings::GetSavedMusicVolume() const { return m_gameSettings.Get(jet::String("MusicVolume")).GetAsFloat(0.75f); }
void GameSettings::SetMusicVolume(float v) { m_gameSettings.Set(jet::String("MusicVolume"), clara::Record(v)); }
float GameSettings::GetSavedSoundVolume() const { return m_gameSettings.Get(jet::String("SoundVolume")).GetAsFloat(0.75f); }
void GameSettings::SetSoundVolume(float v) { m_gameSettings.Set(jet::String("SoundVolume"), clara::Record(v)); }
bool GameSettings::WasIntroMoviePlayed() const { return m_gameSettings.Get(jet::String("WasIntroMoviePlayed")).GetAsBool(false); }
void GameSettings::SetIntroMoviePlayed() { m_gameSettings.Set(jet::String("WasIntroMoviePlayed"), clara::Record(true)); }
void GameSettings::ResetIntroMoviePlayed() { m_gameSettings.Set(jet::String("WasIntroMoviePlayed"), clara::Record(false)); }
jet::String GameSettings::GetLastIntroMoviePlayedGameVersion() const {
    return m_gameSettings.Get(jet::String("LastIntroMoviePlayedGameVersion")).GetAsString(jet::String());
}
void GameSettings::SetLastIntroMoviePlayedGameVersion(const jet::String &v) {
    m_gameSettings.Set(jet::String("LastIntroMoviePlayedGameVersion"), clara::Record(v));
}
jet::String GameSettings::GetIAPInstallMessageShownVersion() const {
    return m_gameSettings.Get(jet::String("IAPInstallMessageShownVersion")).GetAsString(jet::String());
}
void GameSettings::SetIAPInstallMessageShownFor(const jet::String &v) { m_gameSettings.Set(jet::String("IAPInstallMessageShownVersion"), clara::Record(v)); }
bool GameSettings::AreLocalNotificationsEnabled() const { return m_gameSettings.Get(jet::String("LocalNotificationsEnabled")).GetAsBool(true); }
// @0x50bac0: the push notifications follow (online::PushNotificationsMgr::SetPushNotificationsEnabled, not in this tree)
void GameSettings::SetLocalNotificationsEnabled(bool on) { m_gameSettings.Set(jet::String("LocalNotificationsEnabled"), clara::Record(on)); }

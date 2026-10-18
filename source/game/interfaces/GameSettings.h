// source/game/interfaces/GameSettings.h
#ifndef SOURCE_GAME_INTERFACES_GAMESETTINGS_H
#define SOURCE_GAME_INTERFACES_GAMESETTINGS_H

#include "jsingleton.h"
#include "core/pstring.h"
#include "recorddb.h"

// What the player set and the game remembers of itself, in three record files: "settings", "profile" and "debugSettings".
// The tracking managers' records (GameTrackingMgr, KhronoPlayPatternMgr) and the local notifications come with their managers.
class GameSettings : public Singleton<GameSettings> {  // sizeof 0x230
public:
    GameSettings() : m_loaded(false) {}
    virtual ~GameSettings() {}
    // false when the file is missing or was deleted by the game
    bool LoadRecordDB(clara::RecordDB&);
    void SaveRecordDB(clara::RecordDB&);
    // empties the record and saves it marked as deleted
    void DeleteRecordDB(clara::RecordDB&);
    clara::RecordDB &GetGameSettings() { return m_gameSettings; }
    void LoadGameSettings();
    void SaveGameSettings();
    clara::RecordDB &GetGameProfile() { return m_gameProfile; }
    void LoadGameProfile();
    void SaveGameProfile();
    clara::RecordDB &GetDebugSettings() { return m_debugSettings; }
    void LoadDebugSettings();
    void SaveDebugSettings();
    float GetSavedMusicVolume() const;
    void SetMusicVolume(float);
    float GetSavedSoundVolume() const;
    void SetSoundVolume(float);
    bool WasIntroMoviePlayed() const;
    void SetIntroMoviePlayed();
    void ResetIntroMoviePlayed();
    jet::String GetLastIntroMoviePlayedGameVersion() const;
    void SetLastIntroMoviePlayedGameVersion(const jet::String&);
    jet::String GetIAPInstallMessageShownVersion() const;
    void SetIAPInstallMessageShownFor(const jet::String&);
    bool AreLocalNotificationsEnabled() const;
    // @0x50bac0: the push notifications follow (online::PushNotificationsMgr::SetPushNotificationsEnabled, not in this tree)
    void SetLocalNotificationsEnabled(bool);
private:
    bool m_loaded;                        // 0x4
    clara::RecordDB m_gameSettings;       // 0x8
    clara::RecordDB m_gameProfile;        // 0x98
    clara::RecordDB m_debugSettings;      // 0x128
};

#endif

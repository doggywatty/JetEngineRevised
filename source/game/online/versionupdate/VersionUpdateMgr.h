// source/game/online/versionupdate/VersionUpdateMgr.h
#ifndef SOURCE_GAME_ONLINE_VERSIONUPDATE_VERSIONUPDATEMGR_H
#define SOURCE_GAME_ONLINE_VERSIONUPDATE_VERSIONUPDATEMGR_H

class VersionUpdateMgr : public Singleton<VersionUpdateMgr> {  // sizeof 0x1C
public:
    VersionUpdateMgr();
    virtual ~VersionUpdateMgr();
    bool CheckForVersionStatusAndDisplayPopup();
    bool HasUpdate();
    void DownloadUpdateTextForCurrentLanguage();
    bool IsLoadingUpdateText();
    const jet::String &GetUpdateText();
    void Update();
private:
    void ParseData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool m_hasUpdate;  // 0x4
    jet::String m_updateText;  // 0x8
    bool m_loadingUpdateText;  // 0xC
    social::IntrusivePointer<social::downloadable::Downloadable,social::IntrusivePointerNoLock> m_currentUpdateTextDownloadable;  // 0x10
    bool m_isUpdateTextRefreshing;  // 0x18
};

#endif

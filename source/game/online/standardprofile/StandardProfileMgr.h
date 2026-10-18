// source/game/online/standardprofile/StandardProfileMgr.h
#ifndef SOURCE_GAME_ONLINE_STANDARDPROFILE_STANDARDPROFILEMGR_H
#define SOURCE_GAME_ONLINE_STANDARDPROFILE_STANDARDPROFILEMGR_H

class StandardProfileMgr : public Singleton<StandardProfileMgr> {  // sizeof 0xD0
public:
    StandardProfileMgr();
    virtual ~StandardProfileMgr();
    void Init();
    bool IsUserProfileGenerated();
    void Update(unsigned);
    void AddStandardFieldsToProfile();
    void AddFieldToProfile(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, bool);
    void AddFieldToInventory(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, bool);
    void AddStringFieldToInventory(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    bool HasField(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool HasInventoryField(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void UpdateProfile(bool);
    void PrintProfile();
private:
    static void OnProfileUpdated(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > printAndClearNewProfileValues();
    void PrintJsonValue(const Json::Value&);
    void GetDeviceInfo();
    bool AddPendingProfileValues();
    bool m_isUserProfileReady;  // 0x4
    gaia::UserProfile *m_userProfile;  // 0x8
    gaia::BaseServiceManager::Credentials m_credentials;  // 0xC
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_deviceInfo;  // 0x10
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_newProfileStringValues;  // 0x24
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_newProfileNumericValues;  // 0x38
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_newProfileInventoryNumericValues;  // 0x4C
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_newProfileInventoryStringValues;  // 0x60
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_pendingProfileStringValues;  // 0x74
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_pendingProfileNumericValues;  // 0x88
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_pendingInventoryNumericValues;  // 0x9C
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_pendingInventoryStringValues;  // 0xB0
    unsigned m_lastProfileUpdateTime;  // 0xC4
    unsigned m_currentTime;  // 0xC8
    bool m_hasPendingUpdate;  // 0xCC
    bool m_isUpdatingProfile;  // 0xCD
};

#endif

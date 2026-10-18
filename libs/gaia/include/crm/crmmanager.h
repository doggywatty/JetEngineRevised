// libs/gaia/include/crm/crmmanager.h
#ifndef LIBS_GAIA_INCLUDE_CRM_CRMMANAGER_H
#define LIBS_GAIA_INCLUDE_CRM_CRMMANAGER_H

namespace gaia {
class CrmManager : public gaia::GaiaSimpleEventDispatcher<Json::Value> {  // sizeof 0xF0
public:
    CrmManager();
    ~CrmManager();
    int LogEventViaGLOT(const Json::Value&);
    int ResetCrmManager();
    int TriggerPointCut(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, Json::Value);
    bool IsPopupAvailable(const Json::Value&);
    int LaunchPopup(const Json::Value&);
    int DownloadPopupList(const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    int DeletePopupList(const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    static gaia::CrmManager *GetInstance();
    static bool IsInitialized();
    int Initialize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Update(unsigned);
    int OnPause();
    int OnResume();
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSaveFolderPath(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int LaunchOfflineWS(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool IsOfflineWSAvailable(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int DownloadOfflineWS(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int DeleteOfflineWS(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int DeserializeActions();
    int SerializeActions();
    int SerializePopupList();
    int DeserializePopupList();
    int ParseActionList(const Json::Value&);
    int InitPointcutList();
    void UpdateActionList(unsigned);
    int VerifyPointcut(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const Json::Value&);
    static bool APopupEventCallback(void*, bool, Json::Value);
    static bool AGiftEventCallback(void*, bool, Json::Value);
    static bool AStoreItemEventCallback(void*, bool, Json::Value);
    static bool AUrlEventCallback(void*, bool, Json::Value);
    static bool ALogEventCallback(void*, bool, Json::Value);
public:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_campaign_id;  // 0x20
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_pointcut_id;  // 0x40
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_popup_id;  // 0x60
private:
    Json::Value m_pointcutDefinitions;  // 0x80
    static gaia::CrmManager *s_instance;
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_clientId;  // 0x90
    std::vector<gaia::CrmAction *,std::allocator<gaia::CrmAction *> > m_actionList;  // 0xB0
    static bool s_IsInitialized;
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_popupsNeededList;  // 0xC4
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_storedPopupsList;  // 0xD8
};
}

#endif

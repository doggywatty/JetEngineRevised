// libs/gaia/include/gaiaimplementation/gaia_hestia.h
#ifndef LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_HESTIA_H
#define LIBS_GAIA_INCLUDE_GAIAIMPLEMENTATION_GAIA_HESTIA_H

namespace gaia {
class Gaia_Hestia : public gaia::GaiaSimpleEventDispatcher<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > {  // sizeof 0xAC
public:
    Gaia_Hestia();
    int GetHestiaStatus();
    int StartAndAuthorizeHestia(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetClientConfig(gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetFeedEtag(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int CancelRequest(gaia::BaseServiceManager::CancelRequestType);
    int InitializeCRM(const gaia::CRMSettings&, void (const gaia::CRMOperation&)*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int RefreshConfigs(const int, void ()*, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int DispatchCurrentConfig();
    int GetNumberOfStoredConfigs();
    int GetCurrentExpiryDate(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetCurrentConfig(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int SetAccountToUse(gaia::BaseServiceManager::Credentials);
    int InvalidateAllCRMConfigs();
private:
    int InvalidateConfigsStartingFrom(int);
    int InitializeLocalData();
    int EncryptConfig(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int DecryptConfig(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void deleteExpiredConfigs();
    int CheckForUpdateConfigs();
    int storeNewConfig(std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int SaveConfig(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetSaveConfigFlag(bool);
    bool GetSaveConfigFlag();
    int SaveConfigTable();
    int LoadConfigTable();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSaveFolderPath_(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void GetTimeString(unsigned long, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    unsigned long ConvertTimeStringToSec(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void (*m_cbGaiaHestia)();  // 0x20
    void (*m_cbInitCRM)(const gaia::CRMOperation&);  // 0x24
    glwebtools::Mutex m_serviceMutex;  // 0x28
    gaia::BaseServiceManager::Credentials m_accountType;  // 0x30
    bool saveConfigFlag;  // 0x34
    bool finishDownloadingConfigs;  // 0x35
    std::map<unsigned long,std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,std::less<unsigned long>,std::allocator<std::pair<unsigned long const ,std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > > m_configTable;  // 0x38
    bool m_localDataInitialized;  // 0x54
    gaia::UserProfile *m_profile;  // 0x58
    gaia::CRMOperation *crmOp;  // 0x5C
    gaia::CRMSettings m_CRMSettings;  // 0x60
};
}

#endif

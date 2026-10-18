// libs/gaia/include/utils/userprofile.h
#ifndef LIBS_GAIA_INCLUDE_UTILS_USERPROFILE_H
#define LIBS_GAIA_INCLUDE_UTILS_USERPROFILE_H

namespace gaia {
class UserProfile : public gaia::GaiaSimpleEventDispatcher<Json::Value> {  // sizeof 0x68
public:
    static gaia::UserProfile *GetInstance();
    static void Release();
    int Initialize(gaia::BaseServiceManager::Credentials, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    bool IsInitialized();
    int GetProfile(Json::Value&);
    int RefreshProfile();
    int DeleteCustomFields(bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int DeleteProfile(bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int UpdateProfile(const Json::Value&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int MergeProfile(const Json::Value&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int SetProfileField(const Json::Value&, bool, void (gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*)*, void*);
    int GetProfileField(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, Json::Value&);
protected:
    UserProfile(const gaia::UserProfile&);
    UserProfile();
    ~UserProfile();
    gaia::UserProfile &operator=(const gaia::UserProfile&);
private:
    int ValidateProfile(Json::Value&);
    int AddNewProfile(Json::Value&);
    Json::Value RemoveCustomFields();
    int EncodeData(const Json::Value&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int DecodeData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, Json::Value&);
    static gaia::UserProfile *s_instance;
    gaia::BaseServiceManager::Credentials m_accountType;  // 0x20
    bool m_IsInitialized;  // 0x24
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_stdProfileString;  // 0x28
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_standardProfileTemplate;  // 0x48
};
}

#endif

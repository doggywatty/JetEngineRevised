// libs/social_framework/include/social/userdata/friendsstandardprofile.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_FRIENDSSTANDARDPROFILE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_FRIENDSSTANDARDPROFILE_H

namespace social {
class FriendsStandardProfile : public social::Storable {  // sizeof 0x108
public:
    FriendsStandardProfile(social::UserOsiris*, social::UserOsiris*);
    virtual ~FriendsStandardProfile();
    void SetField(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, Json::Value);
    void GetField(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    Json::Value GetCallbackJson();
    void Update(float);
protected:
    virtual void SaveImpl();
    void FinishSave();
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void FriendsStandardProfileCallback(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > ParseJsonToString(Json::Value);
protected:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    static social::FriendsStandardProfile *m_instance;
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_serviceResponseJson;  // 0xBC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_eventStatus;  // 0xD0
    social::UserOsiris *m_ouser;  // 0xF0
    social::UserOsiris *m_fuser;  // 0xF4
    Json::Value m_callbackJson;  // 0xF8
};
}

#endif

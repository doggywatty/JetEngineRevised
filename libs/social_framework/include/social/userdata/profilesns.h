// libs/social_framework/include/social/userdata/profilesns.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_PROFILESNS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_PROFILESNS_H

namespace social {
class ProfileSNS : public social::Profile {  // sizeof 0xD0
public:
    ProfileSNS(sociallib::ClientSNSEnum, const social::User*);
    virtual ~ProfileSNS();
    virtual char* GetAvatarData(int&);
    sociallib::ClientSNSEnum GetSNS();
    virtual social::Batcher *GetBatch();
    void OnDataLoaded(bool, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
private:
    static bool sOnDataLoaded(void*, bool, sociallib::ClientSNSEnum);
    sociallib::ClientSNSEnum m_sns;  // 0xCC
};
}

#endif

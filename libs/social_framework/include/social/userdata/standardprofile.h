// libs/social_framework/include/social/userdata/standardprofile.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_STANDARDPROFILE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_STANDARDPROFILE_H

namespace social {
class StandardProfile : public social::Storable {  // sizeof 0x110
public:
    StandardProfile(social::UserOsiris*);
    virtual ~StandardProfile();
    void GetProfile(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, Json::Value&);
    void SetField(const Json::Value&);
    void AddToField(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
    void RefreshProfile();
    void DeleteProfile(bool);
    void MergeProfile(const Json::Value&);
    void UpdateProfile(const Json::Value&);
    void GetVisibility();
    void SetVisibility(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, bool);
    void GetVisibleProfile(Json::Value&);
protected:
    virtual void SaveImpl();
    void FinishSave();
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void StandardProfileCallback(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
protected:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    static social::StandardProfile *m_instance;
    gaia::UserProfile *m_stdProfile;  // 0xBC
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_visibleProfileJson;  // 0xC0
    Json::Value m_jsonVisibility;  // 0xD8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_eventStatus;  // 0xE8
    social::UserOsiris *m_ouser;  // 0x108
};
}

#endif

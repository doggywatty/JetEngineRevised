// libs/social_framework/include/social/userdata/standardprofileextended.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_STANDARDPROFILEEXTENDED_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_STANDARDPROFILEEXTENDED_H

namespace social {
class StandardProfileExtended : public social::Storable {  // sizeof 0x120
public:
    StandardProfileExtended(social::UserOsiris*);
    virtual ~StandardProfileExtended();
    Json::Value GetFullProfile();
    void ResetProfile();
    Json::Value GetField(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    void SetField(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, Json::Value, bool);
    void GetVisibility();
    void SetVisibility(std::basic_string<char,std::char_traits<char>,std::allocator<char> >, bool);
    void Update(float);
protected:
    virtual void SaveImpl();
    void FinishSave();
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void StandardProfileExtendedCallback(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    bool SearchPathA(Json::Value, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, Json::Value&);
    void Create(Json::Value&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, Json::Value, Json::Value, bool, bool);
    Json::Value Combine(Json::Value, Json::Value, bool);
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > SplitPath(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > ParseJsonToString(Json::Value);
protected:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    static social::StandardProfileExtended *m_instance;
    gaia::UserProfile *m_stdProfile;  // 0xBC
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_visibleProfileJson;  // 0xC0
    Json::Value m_jsonProfile;  // 0xD8
    Json::Value m_jsonVisibility;  // 0xE8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_eventStatus;  // 0xF8
    social::UserOsiris *m_ouser;  // 0x118
    bool m_overwriteStatus;  // 0x11C
};
}

#endif

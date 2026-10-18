// libs/social_framework/include/social/userdata/profileosiris.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_PROFILEOSIRIS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_PROFILEOSIRIS_H

namespace social {
class ProfileOsiris : public social::Profile {  // sizeof 0xF4
public:
    ProfileOsiris(const social::User*);
    virtual ~ProfileOsiris();
    virtual char* GetAvatarData(int&);
    const std::vector<social::Group *,std::allocator<social::Group *> > &GetGroups();
    void LoadFromData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetCustomName(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
private:
    static void sOnProfile(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_profileResponse;  // 0xCC
    std::vector<social::Group *,std::allocator<social::Group *> > m_groups;  // 0xE0
};
}

#endif

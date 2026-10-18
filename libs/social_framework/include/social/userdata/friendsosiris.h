// libs/social_framework/include/social/userdata/friendsosiris.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_FRIENDSOSIRIS_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_FRIENDSOSIRIS_H

namespace social {
class FriendsOsiris : public social::Friends {  // sizeof 0x130
public:
    FriendsOsiris(const social::User*);
    virtual ~FriendsOsiris();
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
private:
    static void sOnfriends(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_friendResponse;  // 0x11C
};
}

#endif

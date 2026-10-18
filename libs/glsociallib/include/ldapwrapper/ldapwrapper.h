// libs/glsociallib/include/ldapwrapper/ldapwrapper.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_LDAPWRAPPER_LDAPWRAPPER_H
#define LIBS_GLSOCIALLIB_INCLUDE_LDAPWRAPPER_LDAPWRAPPER_H

namespace sociallib {
class LdapWrapper : public sociallib::SNSWrapperBase {  // sizeof 0x8
public:
    LdapWrapper();
    virtual ~LdapWrapper();
    virtual void init(sociallib::SNSRequestState&);
    virtual void getUid(sociallib::SNSRequestState&);
    virtual void getFriends(sociallib::SNSRequestState&);
    virtual void getUserData(sociallib::SNSRequestState&);
    virtual void getUserNames(sociallib::SNSRequestState&);
    virtual void getName(sociallib::SNSRequestState&);
    virtual void getAvatar(sociallib::SNSRequestState&);
    virtual void getAppId(sociallib::SNSRequestState&);
    virtual void likeApplication(sociallib::SNSRequestState&);
    virtual void postMessageToWall(sociallib::SNSRequestState&);
    virtual void login(sociallib::SNSRequestState&);
    virtual void logout(sociallib::SNSRequestState&);
    virtual bool isLoggedIn();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSecret();
    bool m_isLoggedIn;  // 0x4
};
}

#endif

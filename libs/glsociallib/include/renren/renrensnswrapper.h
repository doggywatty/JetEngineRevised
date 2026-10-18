// libs/glsociallib/include/renren/renrensnswrapper.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_RENREN_RENRENSNSWRAPPER_H
#define LIBS_GLSOCIALLIB_INCLUDE_RENREN_RENRENSNSWRAPPER_H

namespace sociallib {
class RenrenSNSWrapper : public sociallib::SNSWrapperBase {  // sizeof 0x4
public:
    RenrenSNSWrapper();
    virtual ~RenrenSNSWrapper();
    virtual void init(sociallib::SNSRequestState&);
    virtual void getUid(sociallib::SNSRequestState&);
    virtual void getFriends(sociallib::SNSRequestState&);
    virtual void getFriendsData(sociallib::SNSRequestState&);
    virtual void getUserData(sociallib::SNSRequestState&);
    virtual void getUserNames(sociallib::SNSRequestState&);
    virtual void getName(sociallib::SNSRequestState&);
    virtual void getAvatar(sociallib::SNSRequestState&);
    virtual void getAppId(sociallib::SNSRequestState&);
    virtual void likeApplication(sociallib::SNSRequestState&);
    virtual void postMessageToWall(sociallib::SNSRequestState&);
    virtual void postMessageToWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void postMessageToFriendsWall(sociallib::SNSRequestState&);
    virtual void sendGameRequestToFriends(sociallib::SNSRequestState&);
    virtual void login(sociallib::SNSRequestState&);
    virtual void logout(sociallib::SNSRequestState&);
    virtual bool isLoggedIn();
    virtual void gotAchievement(sociallib::SNSRequestState&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAccessToken();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSecret();
};
}

#endif

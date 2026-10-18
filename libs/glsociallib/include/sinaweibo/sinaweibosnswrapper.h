// libs/glsociallib/include/sinaweibo/sinaweibosnswrapper.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_SINAWEIBO_SINAWEIBOSNSWRAPPER_H
#define LIBS_GLSOCIALLIB_INCLUDE_SINAWEIBO_SINAWEIBOSNSWRAPPER_H

namespace sociallib {
class SinaWeiboSNSWrapper : public sociallib::SNSWrapperBase {  // sizeof 0x4
public:
    SinaWeiboSNSWrapper();
    virtual ~SinaWeiboSNSWrapper();
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
    virtual bool handleOpenUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAccessToken();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSecret();
};
}

#endif

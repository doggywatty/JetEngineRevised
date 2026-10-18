// libs/glsociallib/include/twitter/twittersnswrapper.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_TWITTER_TWITTERSNSWRAPPER_H
#define LIBS_GLSOCIALLIB_INCLUDE_TWITTER_TWITTERSNSWRAPPER_H

namespace sociallib {
class TwitterSNSWrapper : public sociallib::SNSWrapperBase {  // sizeof 0x4
public:
    TwitterSNSWrapper();
    virtual ~TwitterSNSWrapper();
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
    virtual void postMessageToWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void login(sociallib::SNSRequestState&);
    virtual void logout(sociallib::SNSRequestState&);
    virtual bool isLoggedIn();
};
}

#endif

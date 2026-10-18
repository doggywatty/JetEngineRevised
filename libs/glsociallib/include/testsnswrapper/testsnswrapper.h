// libs/glsociallib/include/testsnswrapper/testsnswrapper.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_TESTSNSWRAPPER_TESTSNSWRAPPER_H
#define LIBS_GLSOCIALLIB_INCLUDE_TESTSNSWRAPPER_TESTSNSWRAPPER_H

namespace sociallib {
class TestSNSWrapper : public sociallib::SNSWrapperBase {  // sizeof 0x4
public:
    TestSNSWrapper();
    virtual ~TestSNSWrapper();
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
};
}

#endif

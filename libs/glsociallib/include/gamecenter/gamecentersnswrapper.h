// libs/glsociallib/include/gamecenter/gamecentersnswrapper.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_GAMECENTER_GAMECENTERSNSWRAPPER_H
#define LIBS_GLSOCIALLIB_INCLUDE_GAMECENTER_GAMECENTERSNSWRAPPER_H

namespace sociallib {
class GameCenterSNSWrapper : public sociallib::SNSWrapperBase {  // sizeof 0x4
public:
    GameCenterSNSWrapper();
    virtual ~GameCenterSNSWrapper();
    virtual void init(sociallib::SNSRequestState&);
    virtual void getUid(sociallib::SNSRequestState&);
    virtual void getFriends(sociallib::SNSRequestState&);
    virtual void getFriendsData(sociallib::SNSRequestState&);
    virtual void getUserData(sociallib::SNSRequestState&);
    virtual void getUserNames(sociallib::SNSRequestState&);
    virtual void getName(sociallib::SNSRequestState&);
    virtual void getAvatar(sociallib::SNSRequestState&);
    virtual void getUserAvatar(sociallib::SNSRequestState&);
    virtual void getAppId(sociallib::SNSRequestState&);
    virtual void likeApplication(sociallib::SNSRequestState&);
    virtual void postMessageToWall(sociallib::SNSRequestState&);
    virtual void gotAchievement(sociallib::SNSRequestState&);
    virtual void incrementAchievement(sociallib::SNSRequestState&);
    virtual void resetAchievements(sociallib::SNSRequestState&);
    virtual void showAchievements(sociallib::SNSRequestState&);
    virtual void login(sociallib::SNSRequestState&);
    virtual void logout(sociallib::SNSRequestState&);
    virtual bool isLoggedIn();
    virtual void showLeadearboardWithId(sociallib::SNSRequestState&);
    virtual void postLeaderboardScore(sociallib::SNSRequestState&);
    virtual void getLeaderboardsTypes(sociallib::SNSRequestState&);
    virtual void getNumberOfMatches(sociallib::SNSRequestState&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSecret();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAccessToken();
    void playerNotAuthenticatedError(sociallib::SNSRequestState&);
};
}

#endif

// libs/glsociallib/include/facebook/facebooksnswrapper.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_FACEBOOK_FACEBOOKSNSWRAPPER_H
#define LIBS_GLSOCIALLIB_INCLUDE_FACEBOOK_FACEBOOKSNSWRAPPER_H

namespace sociallib {
class FacebookSNSWrapper : public sociallib::SNSWrapperBase {  // sizeof 0x4
public:
    FacebookSNSWrapper();
    virtual ~FacebookSNSWrapper();
    virtual void init(sociallib::SNSRequestState&);
    virtual void getUid(sociallib::SNSRequestState&);
    virtual void getFriends(sociallib::SNSRequestState&);
    virtual void getFriendsData(sociallib::SNSRequestState&);
    virtual bool hasPermission(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    virtual void requestPermissions(sociallib::SNSRequestState&);
    virtual void getUserData(sociallib::SNSRequestState&);
    virtual void getUserNames(sociallib::SNSRequestState&);
    virtual void getName(sociallib::SNSRequestState&);
    virtual void getAvatar(sociallib::SNSRequestState&);
    virtual void getAppId(sociallib::SNSRequestState&);
    virtual void likeApplication(sociallib::SNSRequestState&);
    virtual void likeApplicationObject(sociallib::SNSRequestState&);
    virtual void postReviewApplication(sociallib::SNSRequestState&);
    virtual void postMessageToWall(sociallib::SNSRequestState&);
    virtual void postMessageToWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void postPhotoToWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void postMessageToFriendsWall(sociallib::SNSRequestState&);
    virtual void postMessageToFriendsWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void postOpenGraphAction(sociallib::SNSRequestState&);
    virtual void sendGameRequestToFriends(sociallib::SNSRequestState&);
    virtual void login(sociallib::SNSRequestState&);
    virtual void logout(sociallib::SNSRequestState&);
    virtual bool isLoggedIn();
    virtual void onResume();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAccessToken();
    virtual bool handleOpenUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSecret();
    virtual void getApplicationDetails(sociallib::SNSRequestState&);
    virtual void getUserScore(sociallib::SNSRequestState&);
    virtual void getAppScores(sociallib::SNSRequestState&);
    virtual void updateScore(sociallib::SNSRequestState&);
    virtual void deleteScore(sociallib::SNSRequestState&);
    virtual void getUserAchievements(sociallib::SNSRequestState&);
    virtual void gotAchievement(sociallib::SNSRequestState&);
    virtual void resetAchievement(sociallib::SNSRequestState&);
};
}

#endif

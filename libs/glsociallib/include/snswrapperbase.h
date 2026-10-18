// libs/glsociallib/include/snswrapperbase.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_SNSWRAPPERBASE_H
#define LIBS_GLSOCIALLIB_INCLUDE_SNSWRAPPERBASE_H

namespace sociallib {
class SNSWrapperBase {  // sizeof 0x4
public:
    SNSWrapperBase();
    virtual ~SNSWrapperBase();
    virtual void init(sociallib::SNSRequestState&);
    virtual void getUid(sociallib::SNSRequestState&);
    virtual void getFriends(sociallib::SNSRequestState&);
    virtual void getFriendsData(sociallib::SNSRequestState&);
    virtual void addFriend(sociallib::SNSRequestState&);
    virtual bool hasPermission(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    virtual void requestPermissions(sociallib::SNSRequestState&);
    virtual void getUserData(sociallib::SNSRequestState&);
    virtual void getUserNames(sociallib::SNSRequestState&);
    virtual void getName(sociallib::SNSRequestState&);
    virtual void getCountry(sociallib::SNSRequestState&);
    virtual void getAvatar(sociallib::SNSRequestState&);
    virtual void getUserAvatar(sociallib::SNSRequestState&);
    virtual void getAppId(sociallib::SNSRequestState&);
    virtual void likeApplication(sociallib::SNSRequestState&);
    virtual void likeApplicationObject(sociallib::SNSRequestState&);
    virtual void postReviewApplication(sociallib::SNSRequestState&);
    virtual void postMessageToWall(sociallib::SNSRequestState&);
    virtual void postMessageToWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void postPhotoToWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void postMessageToFriendsWall(sociallib::SNSRequestState&);
    virtual void postMessageToFriendsWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void uploadPhoto(sociallib::SNSRequestState&);
    virtual void postOpenGraphAction(sociallib::SNSRequestState&);
    virtual void sendGameRequestToFriends(sociallib::SNSRequestState&);
    virtual void login(sociallib::SNSRequestState&);
    virtual void logout(sociallib::SNSRequestState&);
    virtual void sendLostPassword(sociallib::SNSRequestState&);
    virtual void sendMessageTo(sociallib::SNSRequestState&);
    virtual void gotAchievement(sociallib::SNSRequestState&);
    virtual void getLeaderboardRows(sociallib::SNSRequestState&);
    virtual void postLeaderboardScore(sociallib::SNSRequestState&);
    virtual void getUserLeaderboardRank(sociallib::SNSRequestState&);
    virtual void getUserLeaderboardScore(sociallib::SNSRequestState&);
    virtual void getLeaderboardsTypes(sociallib::SNSRequestState&);
    virtual void getApplicationDetails(sociallib::SNSRequestState&);
    virtual void getUserScore(sociallib::SNSRequestState&);
    virtual void getAppScores(sociallib::SNSRequestState&);
    virtual void updateScore(sociallib::SNSRequestState&);
    virtual void deleteScore(sociallib::SNSRequestState&);
    virtual void getUserAchievements(sociallib::SNSRequestState&);
    virtual void getNumberOfMatches(sociallib::SNSRequestState&);
    virtual bool isLoggedIn();
    virtual void update();
    virtual void onResume();
    void doNothing(sociallib::SNSRequestState&);
    void requestNotSupported(sociallib::SNSRequestState&);
    void snsNotSupported(sociallib::SNSRequestState&);
    void emptyUidListError(sociallib::SNSRequestState&);
    void isHtmlUnsupportedError(sociallib::SNSRequestState&);
    void stringParamConversionToIntError(sociallib::SNSRequestState&);
    void notLoggedInError(sociallib::SNSRequestState&);
    virtual bool handleOpenUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAccessToken();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSecret();
    virtual void showAchievements(sociallib::SNSRequestState&);
    virtual void unlockAchievements(sociallib::SNSRequestState&);
    virtual void incrementAchievement(sociallib::SNSRequestState&);
    virtual void resetAchievements(sociallib::SNSRequestState&);
    virtual void resetAchievement(sociallib::SNSRequestState&);
    virtual void showLeadearboardWithId(sociallib::SNSRequestState&);
    virtual void showAllLeadearboards(sociallib::SNSRequestState&);
    virtual void getAllAchievements(sociallib::SNSRequestState&);
};
}

#endif

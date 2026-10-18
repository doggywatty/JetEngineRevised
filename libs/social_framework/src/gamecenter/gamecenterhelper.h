// libs/social_framework/src/gamecenter/gamecenterhelper.h
#ifndef LIBS_SOCIAL_FRAMEWORK_SRC_GAMECENTER_GAMECENTERHELPER_H
#define LIBS_SOCIAL_FRAMEWORK_SRC_GAMECENTER_GAMECENTERHELPER_H

class GameCenterHelper {  // sizeof 0x1
public:
    static void LogIn(void (int, bool, bool, bool, void*)*, void*, bool);
    static bool IsLoggedIn();
    static bool IsLoggingIn();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetLocalPlayerID();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetLocalPlayerAlias();
    static void GetLocalPlayerAvatar(void (bool, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const unsigned char*, int, void*)*, void*);
    static void GetPlayerAlias(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void (bool, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, void*)*, void*);
    static void GetPlayerAvatar(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void (bool, std::basic_string<char,std::char_traits<char>,std::allocator<char> >, const unsigned char*, int, void*)*, void*);
    static std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > GetFriendIDs();
    static void SubmitScore(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, __int64, void (bool, void*)*, void*);
    static void SubmitAchievementProgress(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, double, void (bool, void*)*, bool, void*);
    static void ResetAchievements(void (bool, void*)*, void*);
    static void ShowAchievementsPage(void (void*)*, void*);
    static void ShowLeaderboardsPage(void (void*)*, void*);
    static float GetIOSVersion();
private:
    static std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > s_cachedFriendsList;
};

#endif

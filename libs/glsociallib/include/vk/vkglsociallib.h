// libs/glsociallib/include/vk/vkglsociallib.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_VK_VKGLSOCIALLIB_H
#define LIBS_GLSOCIALLIB_INCLUDE_VK_VKGLSOCIALLIB_H

namespace sociallib {
class VKGLSocialLib {  // sizeof 0x78
public:
    void InitMembers();
private:
    void InitVKLogin();
    void DeleteVKLogin();
    void InitVKUserFriend();
    void DeleteVKUserFriend();
    void InitVKUser();
    void DeleteVKUser();
public:
    VKGLSocialLib();
    virtual ~VKGLSocialLib();
    virtual void Update();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetErrorMessage(int);
    virtual bool HandleEventLogin();
    virtual bool HandleEventLogout();
    virtual bool HandleEventGetFriends(const int, bool);
    virtual bool HandleEventGetFriendsData(const int, bool);
    virtual bool HandleEventGetName(const int, const char*);
    virtual bool HandleEventGetCountry(const char*);
    virtual bool HandleEventGetUserNames(const int, const char*);
    virtual bool HandleEventGetUserData(const int, const char*, bool);
    virtual bool HandleEventGetUserAvatar();
    virtual bool HandleEventDownloadUserAvatar();
    virtual bool HandleEventPostToWall(const char*, const char*, const char*, const char*);
    virtual bool HandleEventGetAvatar(const char*, const int);
    virtual void HandleEventUploadPhoto(const char*, const char*);
    virtual void OnNetworkError();
    virtual void OnRequestTimeout(const int);
    virtual void OnRequestSuccess(const int, const char*, const int);
    virtual void OnRequestFailure(const int, const int);
    virtual void ProcessUploadPhotoRespone(const char*);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAppID();
    void SetAppID(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAccessToken();
    void SetAccessToken(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetUserID();
    void SetUserID(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    long GetExpiresIn();
    void SetExpiresIn(long);
    bool GetIsLoggeIn();
    void SetIsLoggeIn(bool);
    sociallib::VKLogin *GetLoginPtr();
    sociallib::VKUser *getUserPtr();
    sociallib::VKUserFriend *GetUserFriendPtr();
private:
    sociallib::VKLogin *m_pVKLogin;  // 0x4
    sociallib::VKUser *m_pVKUser;  // 0x8
    sociallib::VKUserFriend *m_pVKUserFriend;  // 0xC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_appId;  // 0x10
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_accessToken;  // 0x30
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_uid;  // 0x50
    long m_expiresIn;  // 0x70
    bool m_bIsLoggedIn;  // 0x74
};
}

#endif

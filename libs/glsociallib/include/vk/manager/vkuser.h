// libs/glsociallib/include/vk/manager/vkuser.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_VK_MANAGER_VKUSER_H
#define LIBS_GLSOCIALLIB_INCLUDE_VK_MANAGER_VKUSER_H

namespace sociallib {
class VKUser : public sociallib::VKWebComponent {  // sizeof 0x104
public:
    VKUser();
    virtual ~VKUser();
    virtual void OnUpdateResponse(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    virtual bool OnUpdateSuccess(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual bool OnUpdateFailure(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool SendIsAppUser(const char*);
    bool SendGetProfiles(const int, const char*, const char*, const char*, const char*);
    bool SendGetName(const int, const char*);
    bool SendGetCountry(const char*);
    bool SendGetData(const char*, const char*, const bool);
    bool SendGetUserAvatar(const char*, const int);
    bool SendDownloadUserAvatar(const char*);
    bool SendUploadUserAvatar(const char*);
    void SendUploadPhoto(const char*, const char*);
    bool HandlePostToWall(const char*, const char*, const char*, const char*);
    bool SendGetPhotoUploadServer();
    bool SendPostToWall();
    bool SendPostToWallWithoutUpload(const char*, const char*, const char*, const char*);
    bool DownloadImageAndUploadToServer();
    bool SendSaveWallPhoto(const char*, const char*, const char*);
    void ProcessNameFromJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessCountryFromJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessAvatarUrlFromJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessUploadServerFromJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessPostPhotoToServerJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessSaveWallPhotoJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessPostToWallJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessUserNamesFromJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ProcessUserDataFromJSON(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    char* GetFirstName();
    char* GetLastName();
    bool GetSex();
    char* GetBirthday();
    char* GetUserAvatarKey();
    char* GetUserAvatarURL();
    char* GetUserAvatarData();
private:
    char* m_szFirstName;  // 0x4
    char* m_szLastName;  // 0x8
    bool m_bSex;  // 0xC
    char* m_szBirthday;  // 0x10
    char* m_szAvatarKey;  // 0x14
    char* m_szAvatarUrl;  // 0x18
    char* m_szAvatarData;  // 0x1C
    bool m_bGetAvatar;  // 0x20
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_wallID;  // 0x24
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_wallMsg;  // 0x44
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_wallLink;  // 0x64
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_wallUploadPhotoServer;  // 0x84
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_wallPostHash;  // 0xA4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_wallPhotoUrl;  // 0xC4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_wallPhotoId;  // 0xE4
};
}

#endif

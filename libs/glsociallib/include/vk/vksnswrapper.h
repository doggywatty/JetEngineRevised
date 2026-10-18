// libs/glsociallib/include/vk/vksnswrapper.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_VK_VKSNSWRAPPER_H
#define LIBS_GLSOCIALLIB_INCLUDE_VK_VKSNSWRAPPER_H

namespace sociallib {
class VkSNSWrapper : public sociallib::SNSWrapperBase {  // sizeof 0x4
public:
    VkSNSWrapper();
    virtual ~VkSNSWrapper();
    virtual void init(sociallib::SNSRequestState&);
    virtual void getUid(sociallib::SNSRequestState&);
    virtual void getFriends(sociallib::SNSRequestState&);
    virtual void getFriendsData(sociallib::SNSRequestState&);
    virtual void getUserData(sociallib::SNSRequestState&);
    virtual void getUserNames(sociallib::SNSRequestState&);
    virtual void getName(sociallib::SNSRequestState&);
    virtual void getCountry(sociallib::SNSRequestState&);
    virtual void getAvatar(sociallib::SNSRequestState&);
    virtual void getUserAvatar(sociallib::SNSRequestState&, sociallib::SNSAvatarSizeEnum);
    virtual void getAppId(sociallib::SNSRequestState&);
    virtual void postMessageToWall(sociallib::SNSRequestState&);
    virtual void postMessageToWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void postMessageToFriendsWall(sociallib::SNSRequestState&);
    virtual void postMessageToFriendsWallWithoutDialog(sociallib::SNSRequestState&);
    virtual void login(sociallib::SNSRequestState&);
    virtual void logout(sociallib::SNSRequestState&);
    virtual bool isLoggedIn();
    virtual void uploadPhoto(sociallib::SNSRequestState&);
    virtual void onResume();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetAccessToken();
    virtual void update();
    void userNotLoggedInError(sociallib::SNSRequestState&);
    void singleUserGetDataRequestError(sociallib::SNSRequestState&);
    void singleUserSendMessageToRequestError(sociallib::SNSRequestState&);
};
}

#endif

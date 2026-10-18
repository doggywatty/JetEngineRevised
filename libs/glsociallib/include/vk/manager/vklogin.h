// libs/glsociallib/include/vk/manager/vklogin.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_VK_MANAGER_VKLOGIN_H
#define LIBS_GLSOCIALLIB_INCLUDE_VK_MANAGER_VKLOGIN_H

namespace sociallib {
class VKLogin : public sociallib::VKWebComponent {  // sizeof 0x8
public:
    VKLogin();
    virtual ~VKLogin();
    virtual void Update();
    virtual void OnUpdateResponse(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    virtual bool OnUpdateSuccess(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual bool OnUpdateFailure(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool SendLogin();
    bool SendLogout();
    bool IsLoggedIn();
    void SetLoginState(bool);
protected:
    bool m_isLoggedIn;  // 0x4
};
}

#endif

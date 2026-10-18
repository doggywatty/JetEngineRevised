// libs/glsociallib/include/vk/manager/vkwebcomponent.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_VK_MANAGER_VKWEBCOMPONENT_H
#define LIBS_GLSOCIALLIB_INCLUDE_VK_MANAGER_VKWEBCOMPONENT_H

namespace sociallib {
class VKWebComponent {  // sizeof 0x4
public:
    VKWebComponent();
    virtual ~VKWebComponent();
    virtual void OnUpdateResponse(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    virtual bool OnUpdateSuccess(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual bool OnUpdateFailure(int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Cancel();
    bool SendByGet(int, sociallib::VKWebComponent*, const char*, bool, const char*, bool);
    bool SendByGet(int, void (int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool)*, const char*, bool, const char*, bool);
    bool SendByPost(int, sociallib::VKWebComponent*, const char*, bool, const char*);
    bool SendByPost(int, void (int, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool)*, const char*, bool, const char*);
    void SetErrorForRequest(sociallib::SNSRequestState*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
};
}

#endif

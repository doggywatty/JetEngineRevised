// libs/social_framework/include/social/userdata/avatar.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_AVATAR_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_AVATAR_H

namespace gui {
struct Avatar {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_PhotoArea();
    static jet::String &_Photo_user();
    static jet::String &_LoadingIcon();
    static jet::String &_PhotoArea_Small();
    static jet::String &_PhotoArea_HUD();
};
}

namespace social {
class Avatar : public social::Loadable {  // sizeof 0xA0
public:
    Avatar(const social::User*);
    virtual ~Avatar();
    void SetData(const char*, int, int, int, int);
    const char *GetData();
    int GetWidth();
    int GetHeight();
    int GetComponents();
    int GetTexSize();
protected:
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    int m_width;  // 0x8C
    int m_height;  // 0x90
    int m_texSize;  // 0x94
    int m_components;  // 0x98
    const char *m_data;  // 0x9C
};
}

#endif

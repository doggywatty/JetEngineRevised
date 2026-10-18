// core/jet/include/text/keyboard.h
#ifndef CORE_JET_INCLUDE_TEXT_KEYBOARD_H
#define CORE_JET_INCLUDE_TEXT_KEYBOARD_H

namespace glf {
class Keyboard : public glf::InputDevice {  // sizeof 0x2BC
public:
    virtual const glf::Rtti &GetRtti();
    const char *GetRttiName();
    static const char *GetTypename();
    Keyboard();
    int GetDownCount(glf::Keyboard::KeyCode);
    int GetUpCount(glf::Keyboard::KeyCode);
    bool IsDown(glf::Keyboard::KeyCode);
    bool IsUp(glf::Keyboard::KeyCode);
    wchar_t GetChar(glf::Keyboard::KeyCode);
    glf::Keyboard::KeyCode GetKeyCode(wchar_t);
    bool WasPressed(glf::Keyboard::KeyCode);
    bool WasReleased(glf::Keyboard::KeyCode);
    void SetVisible(bool);
    bool IsVisible();
    virtual bool PreSendEvent(glf::CoreEvent&);
    void RaiseButtonEvent(int, bool);
    static const glf::Rtti sRtti;
private:
    glf::SimpleButton mButtons[103];  // 0x50
    wchar_t mChars[103];  // 0x1EC
    bool mVisible;  // 0x2BA
};
}

namespace gui {
struct KeyBoard {  // sizeof 0x1
    jet::String operator class jet::String();
    static jet::String &_Bg0_FillRect();
    static jet::String &_InterfaceText();
    static jet::String &_Num0_Button();
    static jet::String &_Num1_Button();
    static jet::String &_Num2_Button();
    static jet::String &_Num3_Button();
    static jet::String &_Num4_Button();
    static jet::String &_Num5_Button();
    static jet::String &_Num6_Button();
    static jet::String &_Num7_Button();
    static jet::String &_Num8_Button();
    static jet::String &_Num9_Button();
    static jet::String &_Ok_button();
    static jet::String &_Back_button();
    static jet::String &_Remove_button();
};
}

namespace jet::text {
class Keyboard : public Singleton<jet::text::Keyboard> {  // sizeof 0x24
public:
    Keyboard();
    void SetVisible(bool);
    bool IsVisible();
    void InsertText(const char*);
    bool HasChar();
    unsigned GetChar();
    static jet::String ProcessChar(const jet::String&, const unsigned);
protected:
    bool m_visible;  // 0x4
    boost::circular_buffer<unsigned int,std::allocator<unsigned int> > m_buffer;  // 0x8
};
}

#endif

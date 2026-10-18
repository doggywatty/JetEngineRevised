// source/game/interfaces/popups/PopupQuestion.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPQUESTION_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPQUESTION_H

#include "BasePopup.h"

class InterfaceButton;
class InterfaceText;

// A question with a yes and a no; the callbacks are called with the user data when the button is pressed (the popup closes first)
class PopupQuestion : public BasePopup {  // sizeof 0x1E4
public:
    PopupQuestion(const jet::String &text, void *userData, void (*yes)(void*), void (*no)(void*));
    PopupQuestion(const jet::String &text, void *userData, void (*yes)(void*), void (*no)(void*), const jet::String &yesText, const jet::String &noText);
    virtual ~PopupQuestion();
    virtual void Update(int);
    virtual void RefreshPopup();
private:
    void Setup(const jet::String &text, void *userData, void (*yes)(void*), void (*no)(void*));
    InterfaceButton *m_yesButton;  // 0x1C8
    InterfaceButton *m_noButton;   // 0x1CC
    InterfaceText *m_infoText;     // 0x1D0
    jet::String m_textPopupString; // 0x1D4
    void (*m_yesCallback)(void*);  // 0x1D8
    void (*m_noCallback)(void*);   // 0x1DC
    void *m_userData;              // 0x1E0
};

#endif

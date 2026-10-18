// source/game/interfaces/popups/PopupQuestion.cpp
//
// Not ported: KeyEventManager::ClearKeyEvents.
#include "sound/soundmgr.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "PopupQuestion.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/PopupMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"

// @0x4456a8 (the layer is gui::QuestionPopup's name @0x47e104)
PopupQuestion::PopupQuestion(const jet::String &text, void *userData, void (*yes)(void*), void (*no)(void*))
    : BasePopup(jet::String("QuestionPopup")), m_yesButton(0), m_noButton(0), m_infoText(0), m_yesCallback(0), m_noCallback(0), m_userData(0) {
    Setup(text, userData, yes, no);
}

// @0x44577c: the buttons say what the caller wants
PopupQuestion::PopupQuestion(const jet::String &text, void *userData, void (*yes)(void*), void (*no)(void*), const jet::String &yesText, const jet::String &noText)
    : BasePopup(jet::String("QuestionPopup")), m_yesButton(0), m_noButton(0), m_infoText(0), m_yesCallback(0), m_noCallback(0), m_userData(0) {
    Setup(text, userData, yes, no);
    m_yesButton->SetText(yesText);
    m_noButton->SetText(noText);
}

void PopupQuestion::Setup(const jet::String &text, void *userData, void (*yes)(void*), void (*no)(void*)) {
    m_textPopupString = text;
    m_yesButton = GetUIButton(jet::String("YesButton"));
    m_noButton = GetUIButton(jet::String("NoButton"));
    m_yesCallback = yes;
    m_noCallback = no;
    m_userData = userData;
    RefreshPopup();
}

PopupQuestion::~PopupQuestion() {}

// @0x4458c0: the back key is the "no"; the popup closes before the callback is called
void PopupQuestion::Update(int dt) {
    BasePopup::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    const int n = mm->ACTION_GetNumReleasedButtons();
    int i = 0;
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);  // and KeyEventManager::ClearKeyEvents (not ported)
        if (m_noCallback) m_noCallback(m_userData);
        PopupMgr::GetInstance()->PopPopup(true);
        return;
    }
    for (; i < n; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_yesButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            PopupMgr::GetInstance()->PopPopup(true);
            if (m_yesCallback) m_yesCallback(m_userData);
            return;
        }
        if (b == m_noButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            if (m_noCallback) m_noCallback(m_userData);
            PopupMgr::GetInstance()->PopPopup(true);
            return;
        }
    }
}

// @0x445a0c
void PopupQuestion::RefreshPopup() {
    m_infoText = GetUIText(jet::String("PopupInfo"));
    m_infoText->SetText(m_textPopupString.c_str());
}

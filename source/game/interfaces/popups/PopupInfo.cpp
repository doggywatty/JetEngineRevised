// source/game/interfaces/popups/PopupInfo.cpp
//
// Not ported: KeyEventManager::ClearKeyEvents, and the online login (LoginMgr: the buttons of the
// account popups steal / merge / cancel the social login), which the .so calls where marked.
#include "sound/soundmgr.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "PopupInfo.h"
#include "game/Game.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/PopupMgr.h"
#include "game/interfaces/interfaceobjects/Interface3DCostumePos.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"

// @0x3bcee0 (PopupInfo::k_popupConfig): the text and the buttons (yes, no, ok, cancel) of each type
namespace {
struct PopupConfig { const char *text; bool yes, no, ok, cancel; };
const PopupConfig k_popupConfig[4] = {
    { "", false, false, true, false },
    { "POPUP_STEAL_ACOUNT", true, false, false, true },
    { "POPUP_MERGE_ACOUNT", true, true, false, true },
    { "POPUP_MERGE_OR_STEAL_ACOUNT", true, true, false, true },
};
}

// @0x440e34
PopupInfo::PopupInfo(EPopupInfoTypeDef type, const std::string &text)
    : BasePopup(jet::String("InfoPopup")), m_type(type), m_yesButton(0), m_noButton(0), m_okButton(0), m_cancelButton(0), m_infoText(0), m_param(0) {
    Setup(text);
}

// @0x440c9c
PopupInfo::PopupInfo(EPopupInfoTypeDef type, const std::string &text, int param)
    : BasePopup(jet::String("InfoPopup2")), m_type(type), m_yesButton(0), m_noButton(0), m_okButton(0), m_cancelButton(0), m_infoText(0), m_param(0) {
    Setup(text);
    m_param = param;
}

void PopupInfo::Setup(const std::string &text) {
    m_textPopupString = jet::String(text.c_str());
    m_yesButton = GetUIButton(jet::String("YesButton"));
    m_noButton = GetUIButton(jet::String("NoButton"));
    m_okButton = GetUIButton(jet::String("OkButton"));
    m_cancelButton = GetUIButton(jet::String("CancelButton"));
    RefreshPopup();
}

PopupInfo::~PopupInfo() {}

// @0x441024: the back key is the "ok"; a button released does what it is for
void PopupInfo::Update(int dt) {
    if (m_refreshOnNextFrame) RefreshPopup();
    BasePopup::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    const int n = mm->ACTION_GetNumReleasedButtons();
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        // KeyEventManager::ClearKeyEvents (not ported)
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
        OnOkButton();
    }
    for (int i = 0; i < n; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_yesButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            OnYesButton();
            return;
        }
        if (b == m_noButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            OnNoButton();
            return;
        }
        if (b == m_okButton) {
            SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_menu_click, 0);
            OnOkButton();
            return;
        }
        if (b == m_cancelButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            OnCancelButton();
            return;
        }
    }
}

// @0x4411dc: the text and the buttons the type shows
void PopupInfo::RefreshPopup() {
    m_refreshOnNextFrame = false;
    m_infoText = GetUIText(jet::String("PopupInfo"));
    if (m_type == Info) {
        m_infoText->SetText(m_textPopupString.c_str());
        m_yesButton->SetVisible(false);
        m_noButton->SetVisible(false);
        m_okButton->SetVisible(true);
        m_cancelButton->SetVisible(false);
    } else {
        const PopupConfig &c = k_popupConfig[m_type];
        m_infoText->SetText(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String(c.text)).c_str());
        m_yesButton->SetVisible(c.yes);
        m_noButton->SetVisible(c.no);
        m_okButton->SetVisible(c.ok);
        m_cancelButton->SetVisible(c.cancel);
    }
    RefreshCostumes();
}

// @0x441358: the account popups log in to the social network stealing or merging the account (LoginMgr is not ported)
void PopupInfo::OnYesButton() {
    if (m_type == StealAccount) {
        // LoginMgr::LoginAndStealSNS (@0x46f490)
        PopupMgr::GetInstance()->PopPopup(true);
    } else if (m_type == MergeAccount || m_type == MergeOrStealAccount) {
        if (m_type == MergeAccount) {
            // LoginMgr::LoginAndMergeSNS (@0x46f514)
        }
        PopupMgr::GetInstance()->PopPopup(true);
    }
}

// @0x4413e0
void PopupInfo::OnNoButton() {
    if (m_type == MergeAccount || m_type == MergeOrStealAccount) {
        if (m_type == MergeOrStealAccount) {
            // LoginMgr::LoginAndStealSNS (@0x46f490)
        }
        PopupMgr::GetInstance()->PopPopup(true);
    }
}

// @0x441438: the plain information closes; the one the main menu made asks for the shop
void PopupInfo::OnOkButton() {
    if (m_type != Info) return;
    if (m_param == 100) Game::GoToShop = 1;
    PopupMgr::GetInstance()->PopPopup(true);
}

// @0x441484
void PopupInfo::OnCancelButton() {
    if (m_type != StealAccount && m_type != MergeAccount && m_type != MergeOrStealAccount) return;
    // LoginMgr::CancelSNSLogin (@0x46f9c0)
    PopupMgr::GetInstance()->PopPopup(false);
}

// @0x4414d8: the successful purchase is a screen of its own
int PopupInfo::GetCurrentTrackingScreen() const {
    if (m_type != Info) return -1;
    return m_textPopupString.Equals(babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String("STR_POPUP_SUCCESSFUL_PURCHASE")).c_str()) ? 0x1bca2 : -1;
}

// @0x441578: the costume of the popup stands idle
void PopupInfo::RefreshCostumes() {
    if (Interface3DCostumePos *c = GetUI3DCostumePos(jet::String("Interface3DCostumePos"))) c->SetAnimixerState(Interface3DCostumePos::k_Minion_Costume_State_Name_Idle);
}

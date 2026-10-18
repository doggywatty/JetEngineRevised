// source/game/interfaces/popups/PopupGoToLocation.cpp
//
// Not ported: the tracking of the purchases (GameTrackingMgr) and the IAP layout, which the .so never shows (the flag at 0x1CD is written false
// by the constructor and nowhere else): its real money purchase (Store, Player::HandleDeliverIAPItem) is left out.
#include "sound/soundmgr.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "PopupGoToLocation.h"
#include "game/Game.h"
#include "game/Player.h"
#include "game/entities/LocationInfoEntity.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/PopupMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/interfaces/menus/Menu_Goals.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"

bool PopupGoToLocation::isPopupLocation = false;

static const jet::String &Text(const char *key) { return babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String(key)); }

// @0x4b6cc4
PopupGoToLocation::PopupGoToLocation(int location, bool justUnlocked, std::function<void()> onClose)
    : BasePopup(jet::String("GoToLocationPopup")), m_info(0), m_closing(false), m_isIAP(false), m_justUnlocked(justUnlocked), m_iapPurchaseInProgress(false),
      m_unlocked(false) {
    m_gameStateDependency = GameState::GetCrtState();
    m_menuDependency = MenuMgr::GetInstance()->GetCurrentPlayingMenu();
    m_info = LocationsMgr::GetInstance()->GetLocationInfo(location);
    m_unlocked = Player::GetInstance()->IsLocationUnlocked(location);
    GetUIObject(jet::String("IAP_Parent"))->SetVisible(m_isIAP);
    GetUIObject(jet::String("NoIAP_Parent"))->SetVisible(!m_isIAP);
    m_lockedByProgressBackButton = GetUIButton(jet::String("LockedByProgress_BackButton"));
    m_dlcBackButton = GetUIButton(jet::String("DLCNeeded_BackButton"));
    m_dlcGoToPageButton = GetUIButton(jet::String("DLCNeeded_GoToDLCPage"));
    m_canFastTravelBackButton = GetUIButton(jet::String("CanFastTravel_BackButton"));
    m_canFastTravelBuyButton = GetUIButton(jet::String("CanFastTravel_BuyButton"));
    m_iapBackButton = GetUIButton(jet::String("IAP_BackButton"));
    m_iapBuyButton = GetUIButton(jet::String("IAPBuyButton"));
    m_onClose = onClose;
    ShowLocationSpecificData();
    RefreshPopup();
    isPopupLocation = true;
}

// @0x4b7334
PopupGoToLocation::~PopupGoToLocation() {}

// @0x4b73b0: the buttons: buying the travel, the download page of the data, the way back
void PopupGoToLocation::Update(int dt) {
    BasePopup::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    int n = mm->ACTION_GetNumReleasedButtons();
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
        PopupMgr::GetInstance()->PopPopup(true);
        return;
    }
    for (int i = 0; i < n; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_canFastTravelBuyButton) {
            BuyFastTravel(0, 0);
            break;
        }
        if (b == m_dlcGoToPageButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            // the Goals menu on its locations tab, where the data of the location is downloaded
            Menu_Goals *goals = static_cast<Menu_Goals*>(mm->GetMenu(Menu_Goals::k_menuName));
            if (goals != mm->GetCurrentPlayingMenu()) {
                goals->SetEnterState(Menu_Goals::Goals_Locations);
                mm->PushMenu(Menu_Goals::k_menuName);
            } else {
                goals->SetEnterState(Menu_Goals::Goals_Locations);
                goals->OnEnter();
            }
            m_closing = true;
        }
        if (b == m_iapBuyButton) {
            // a real money price is bought through the store (not ported); the others are paid in the game's currencies
            if (m_info->GetUnlockByPricePricePtr()->GetPriceType() != Price::IAP) BuyLocation(0, 0);
            break;
        }
        if (b == m_lockedByProgressBackButton || b == m_dlcBackButton || b == m_canFastTravelBackButton || b == m_iapBackButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_back, 0);
            m_closing = true;
            if (m_onClose) {
                std::function<void()> f = m_onClose;
                f();
                m_onClose = std::function<void()>();
            }
            break;
        }
    }
    if (m_isIAP) {
        bool wasInProgress = m_iapPurchaseInProgress;
        bool changed = false != wasInProgress;  // Store::IsIAPPurchaseInProgress() is false without the store
        if (!m_unlocked && Player::GetInstance()->IsLocationUnlocked(m_info->GetLocationId())) {
            m_unlocked = true;
            SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);
            if (Game::GetInstance()->HasDLCDataFor(m_info->GetLocationId(), true)) {
                changed = false;
                FastTravelToLocation();
            }
        }
        if (!m_closing && changed) RefreshPopup();
    }
    if (m_closing) PopupMgr::GetInstance()->PopPopup(true);
}

// @0x4b7734: which of the groups of the layer is shown, with the texts of the location
void PopupGoToLocation::RefreshPopup() {
    const LocationInfoEntity *info = m_info;
    if (m_isIAP) {
        GetUIText(jet::String("IAPTitleText"))->SetText(info->GetTitle());
        Price *price = static_cast<Price*>(info->GetUnlockByPricePrice());
        m_iapBuyButton->SetText(price->GetPriceAsString());
        GetUIObject(jet::String("IAPBuy_Icon_bananas"))->SetVisible(price->GetPriceType() == Price::Bananas);
        GetUIObject(jet::String("IAPBuy_Icon_tokens"))->SetVisible(price->GetPriceType() == Price::Tokens);
        m_iapPurchaseInProgress = false;  // Store::IsIAPPurchaseInProgress() for an IAP price
        GetUIObject(jet::String("IAPBuyWaitAnim"))->SetVisible(m_iapPurchaseInProgress);
        m_iapBuyButton->SetVisible(!m_iapPurchaseInProgress);
        GetUIText(jet::String("IAP_FreeUnlockConditionText"))->SetText(info->GetFastTravel_LockByProgress_Info());
        return;
    }
    GetUIText(jet::String("TitleText"))->SetText(info->GetTitle());
    InterfaceText *infoText = GetUIText(jet::String("PopupInfo"));
    InterfaceText *infoText2 = GetUIText(jet::String("PopupInfo_2"));
    infoText->SetText(jet::String());
    infoText2->SetText(jet::String());
    bool locked = !Player::GetInstance()->IsLocationUnlocked(info->GetLocationId());
    jet::String canUseInfo;
    if (m_justUnlocked) {
        infoText->SetText(info->GetLocationJustUnlocked_Info());
        infoText2->SetText(info->GetLocationJustUnlocked_Info2());
    } else if (locked) {
        infoText->SetText(info->GetFastTravel_LockByProgress_Info());
        infoText2->SetText(info->GetFastTravel_LockByProgress_Info2());
    } else if (!info->HasTutorialImplementation() && !TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_firstPlay)) {
        infoText->SetText(Text("FINISH_TUTORIAL"));
        locked = true;
    }
    if (m_justUnlocked) locked = true;
    GetUIObject(jet::String("LockedByProgress_parent"))->SetVisible(locked);
    bool hasDLC = Game::GetInstance()->HasDLCDataFor(info->GetLocationId(), true);
    bool dlcNeeded = !hasDLC;
    GetUIObject(jet::String("DLCNeeded_parent"))->SetVisible(dlcNeeded);
    bool canFastTravel = hasDLC && !locked;
    GetUIObject(jet::String("CanFastTravel_parent"))->SetVisible(canFastTravel);
    if (canFastTravel) {
        canUseInfo = info->GetFastTravel_CanUseLocation_Info();
        Price *price = static_cast<Price*>(info->GetFastTravelPrice());
        m_canFastTravelBuyButton->SetText(price->GetPriceAsString());
        GetUIObject(jet::String("CanFastTravel_Icon_bananas"))->SetVisible(price->GetPriceType() == Price::Bananas);
        GetUIObject(jet::String("CanFastTravel_Icon_tokens"))->SetVisible(price->GetPriceType() == Price::Tokens);
    }
    GetUIText(jet::String("CanFastTravel_Info"))->SetText(canUseInfo);
    if (dlcNeeded) {
        m_dlcBackButton->SetVisible(false);
        m_iapBackButton->SetVisible(false);
        m_lockedByProgressBackButton->SetVisible(false);
        m_dlcGoToPageButton->SetVisible(true);
        infoText2->SetText(Text("FREE_DOWNLOAD_MAP"));
        m_dlcGoToPageButton->SetText(Text("OK"));
    }
}

// @0x4b8364: the price of the location is paid; the location is unlocked and travelled to
void PopupGoToLocation::BuyLocation(int a, int b) {
    (void)a;
    (void)b;
    Price *price = static_cast<Price*>(m_info->GetUnlockByPricePrice());
    if (!Player::GetInstance()->ExecuteCost(*price, std::bind(&PopupGoToLocation::BuyLocation, this, std::placeholders::_1, std::placeholders::_2))) return;
    SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);  // and GameTrackingMgr::SendCurrencySpentEvent (not ported)
    Player::GetInstance()->SetLocationUnlocked(m_info->GetLocationId());
    Game::GetInstance()->Save(false, false);
    FastTravelToLocation();
}

// @0x4b84fc: the price of the travel is paid (the popup closes if the player has to go and buy the currency)
void PopupGoToLocation::BuyFastTravel(int a, int b) {
    (void)a;
    (void)b;
    Price *price = static_cast<Price*>(m_info->GetFastTravelPrice());
    if (!Player::GetInstance()->ExecuteCost(*price, std::bind(&PopupGoToLocation::BuyFastTravel, this, std::placeholders::_1, std::placeholders::_2),
                                            std::bind(&PopupGoToLocation::PopPopup, this)))
        return;
    SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);  // and GameTrackingMgr::SendCurrencySpentEvent (not ported)
    FastTravelToLocation();
}

// @0x4b8698: the player runs in the location, the game goes back to the main menu with it
void PopupGoToLocation::FastTravelToLocation() {
    m_closing = true;
    Player::GetInstance()->SetCurrentLocation(m_info->GetLocationId());
    Game::GetInstance()->GoToMainMenu();
    Game::GetInstance()->Save(false, false);
}

// @0x4b8708: only the picture of the location is shown
void PopupGoToLocation::ShowLocationSpecificData() {
    static const char *k_names[6] = { "LaboRoom", "Residential", "MachoLab", "MinionsBeach", "Resi_Halloween", "Mall" };
    const char *prefix = m_isIAP ? "IAP" : "NoIAP";
    InterfaceObject *parent = GetUIObject(jet::String((std::string(prefix) + "LocationSpecific_Parent").c_str()));
    for (unsigned i = 0; i < parent->GetChildCount(); i++) parent->GetChild((int)i)->SetVisible(false);
    int loc = m_info->GetLocationId();
    if (loc < 0 || loc > 5) return;
    GetUIObject(jet::String((std::string(prefix) + "Location_" + k_names[loc]).c_str()))->SetVisible(true);
}

// source/game/interfaces/menus/box/LocationBox.cpp
//
// Not ported: the tracking of the purchases (GameTrackingMgr). The download of the
// data of a location goes through Game's on demand DLC (the manhattan download client is not ported, a request stays at no
// progress).
#include "sound/soundmgr.h"
#include "LocationBox.h"
#include "game/Game.h"
#include "game/Player.h"
#include "game/Statistics.h"
#include "game/entities/LocationInfoEntity.h"
#include "game/gamemanagers/LocationsMgr.h"
#include "game/gamemanagers/BackgroundMgr.h"
#include "game/gamemanagers/TutorialMgr.h"
#include "game/interfaces/PopupMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "game/interfaces/menus/Menu_ResultEndScreen.h"
#include "game/interfaces/popups/PopupGoToLocation.h"
#include "game/interfaces/popups/PopupQuestion.h"
#include "game/online/store/Price.h"
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"

static const jet::String &Text(const char *key) { return babel::Babel::GetInstance()->GetStringMgr()->Get(jet::String(key)); }

static const float k_bytesToMegabytes = 1.0f / 1048576.0f;

// @0x4d3268: the box takes the size of its BoxSize; when the data of the location is not on the device the download button
// carries a bar of its progress
LocationBox::LocationBox(int location)
    : BasicPage(jet::String("LocationBox")), m_info(0), m_needRefresh(false), m_buyButton(0), m_questionButton(0), m_downloadButton(0), m_downloadBar(0), m_goButton(0),
      m_unlocked(false), m_travelled(false) {
    SetSize(GetUIObject(jet::String("BoxSize"))->GetSize());
    m_info = LocationsMgr::GetInstance()->GetLocationInfo(location);
    m_buyButton = GetUIButton(jet::String("Locked_BuyButton"));
    m_questionButton = GetUIButton(jet::String("Locked_QuestionButton"));
    m_downloadButton = GetUIButton(jet::String("DLCNeeded_DownloadButton"));
    Game *game = Game::GetInstance();
    int id = m_info->GetLocationId();
    if (!game->HasDLCDataFor(id, true)) {
        unsigned size = 0;
        float progress = 0.0f;
        if (game->IsDlcAvailableOnServer()) {
            size = game->GetLocationDlcSize(id);
            progress = game->GetLocationDlcProgress(id);
        }
        m_downloadBar = ManualAddPBar(0, m_downloadButton->GetPos(), 0, m_downloadButton->GetSpriteIdx(), -1, 0x116, -1, -1, -1, 1);
        m_downloadBar->SetSize(m_downloadButton->GetSize());
        m_downloadBar->SetAlpha(0x50);
        m_downloadBar->SetValue(progress);
        if (Player::GetInstance()->IsLocationUnlocked(id) && game->IsLocationDlcRequested(id)) {
            m_downloadButton->SetText(jet::String::Format("%s(%.1fM/%.1fM)", Text("CONNECTIVITY_DOWLOADING").c_str(), progress * (float)size * k_bytesToMegabytes,
                                                          (float)size * k_bytesToMegabytes));
            m_downloadBar->SetVisible(true);
        } else {
            m_downloadButton->SetText(jet::String::Format("%s(%.1fM)", Text("CONNECTIVITY_DOWNLOAD_UPDATE").c_str(), (float)size * k_bytesToMegabytes));
            m_downloadBar->SetVisible(false);
        }
    } else
        m_downloadButton->SetVisible(false);
    GetUIButton(jet::String("Unlocked_GoToLocationButton"))->SetVisible(false);
    m_goButton = GetUIButton(jet::String("Unlocked_GoToLocationButtonStart"));
    ShowLocationSpecificData();
    Refresh();
}

// @0x4d3a14
LocationBox::~LocationBox() {}

// @0x4d3a64: the box refreshes when asked or when its location was unlocked, shows the progress of the download of the data
// and answers its buttons
void LocationBox::Update(int dt) {
    if (m_needRefresh || m_unlocked != Player::GetInstance()->IsLocationUnlocked(m_info->GetLocationId())) Refresh();
    BasicPage::Update(dt);
    Game *game = Game::GetInstance();
    int id = m_info->GetLocationId();
    if (m_downloadBar && m_downloadBar->IsVisible()) {
        float progress = game->GetLocationDlcProgress(id);
        m_downloadBar->SetValue(progress);
        float size = (float)game->GetLocationDlcSize(id);
        if (progress > 0) {
            const char *key = game->IsLocationDlcRequested(id) ? "CONNECTIVITY_DOWLOADING" : "CONNECTIVITY_DOWNLOAD_CONTINUE";
            m_downloadButton->SetText(jet::String::Format("%s(%.1fM/%.1fM)", Text(key).c_str(), progress * k_bytesToMegabytes * size, size * k_bytesToMegabytes));
        } else
            m_downloadButton->SetText(Text("CONNECTIVITY_DOWNLOAD_PREPARE"));
        if (game->HasDLCDataFor(id, true)) {
            m_downloadBar->SetVisible(false);
            m_downloadButton->SetVisible(false);
            m_needRefresh = true;
        }
    }
    MenuMgr *mm = MenuMgr::GetInstance();
    for (int i = 0; i < mm->ACTION_GetNumReleasedButtons(); i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_buyButton) {
            mm->DisableTouchesFor(200);
            BuyLocation(0, 0);
            return;
        }
        if (b == m_questionButton) {
            mm->DisableTouchesFor(200);
            PopupMgr::GetInstance()->PushPopup(new PopupGoToLocation(id, false, std::function<void()>()));
            return;
        }
        if (b == m_downloadButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            // the download is asked for once the player confirmed it (a popup), or says there is no connection
            if (game->AreOnlineServicesReady()) {
                jet::String message;
                if (game->IsDlcAvailableOnServer()) {
                    float megabytes = (float)game->GetLocationDlcSize(id) * k_bytesToMegabytes;
                    message = jet::String::Format(Text("DECRIPTION_CONFIRM_DOWLOAD_DLC").c_str(), babel::Babel::GetInstance()->GetFormatter()->FormatMeasure(megabytes, 2).c_str());
                } else
                    message = jet::String::Format(Text("DECRIPTION_CONFIRM_DOWLOAD_DLC").c_str(), babel::Babel::GetInstance()->GetFormatter()->FormatMeasure(7.7f, 2).c_str());
                PopupMgr::GetInstance()->PushPopup(new PopupQuestion(message, this, &LocationBox::ConfirmDownloadDlc, 0));
            } else
                PopupMgr::GetInstance()->PushInfoPopup(std::string(Text("POPUP_NO_INTERNET").c_str()));
            return;
        }
        if (b == m_goButton) {
            mm->DisableTouchesFor(200);
            BuyFastTravel(0, 0);
            return;
        }
    }
}

// @0x4d4178
void LocationBox::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    if (IsVisible()) BasicPage::Render(cam, p);
}

// @0x4d41b0: which of the groups of the layer is shown, by whether the location is unlocked, the data is there and the player
// is in it (and by the first run, before which the buttons are not there)
void LocationBox::Refresh() {
    if (m_travelled) {
        m_travelled = false;
        // (the main menu state plays its own music again: GS_MainMenu::GS_MainMenu stops the old one)
        Game::GetInstance()->GoToMainMenu();
        return;
    }
    m_needRefresh = false;
    int id = m_info->GetLocationId();
    m_unlocked = Player::GetInstance()->IsLocationUnlocked(id);
    int current = BackgroundMgr::GetInstance()->GetLocationOfCurrentBackground();
    bool firstPlayDone = TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_firstPlay);
    m_buyButton->SetVisible(firstPlayDone);
    m_questionButton->SetVisible(firstPlayDone);
    m_downloadButton->SetVisible(firstPlayDone);
    bool notCurrent = current != id;
    m_goButton->SetVisible(firstPlayDone && notCurrent);
    bool locked = (notCurrent && !firstPlayDone) ? true : !m_unlocked;
    GetUIObject(jet::String("Locked_parent"))->SetVisible(locked);
    bool dlcNeeded;
    if (locked) {
        GetUIText(jet::String("Locked_Scene_Title_Text"))->SetText(m_info->GetTitle());
        GetUIText(jet::String("Locked_Multiplier_Text"))->SetText(jet::String::Format("x%d", GetScoreMultiplierValue()));
        InterfaceText *status = GetUIText(jet::String("Locked_status_Text"));
        if (id == 2) status->SetText((jet::String(Text("END_SCREEN_UNLOCK")) + Text("NOT_NEED_DOWNLOAD")).c_str());
        status->SetVisible(firstPlayDone);
        status->SetVisible(false);
        m_buyButton->SetVisible(false);
        GetUIObject(jet::String("Locked_BuyButton_PriceBananas"))->SetVisible(false);
        GetUIObject(jet::String("Locked_BuyButton_PriceTokens"))->SetVisible(false);
        dlcNeeded = false;
    } else
        dlcNeeded = !Game::GetInstance()->HasDLCDataFor(id, true);
    GetUIObject(jet::String("DLCNeeded_parent"))->SetVisible(dlcNeeded);
    if (dlcNeeded) GetUIText(jet::String("DLC_Needed_Scene_Title_Text"))->SetText(m_info->GetTitle());
    bool unlockedShown = !dlcNeeded && !locked;
    GetUIObject(jet::String("Unlocked_parent"))->SetVisible(unlockedShown);
    if (unlockedShown) {
        GetUIText(jet::String("Unlocked_Scene_Title_Text"))->SetText(m_info->GetTitle());
        // LocationInfoEntity::GetFastTravelPrice() is asked and not used
        GetUIObject(jet::String("Unlocked_GoToLocation_bg"))->SetVisible(false);
    }
}

// @0x4d4b24: only the picture of the location is shown
void LocationBox::ShowLocationSpecificData() {
    static const char *k_names[6] = { "SpecificData_LaboRoom", "SpecificData_Residential", "SpecificData_MachoLab", "SpecificData_MinionsBeach", "SpecificData_Resi_Halloween",
                                      "SpecificData_Mall" };
    InterfaceObject *parent = GetUIObject(jet::String("SpecificData_parent"));
    for (unsigned i = 0; i < parent->GetChildCount(); i++) parent->GetChild((int)i)->SetVisible(false);
    int id = m_info->GetLocationId();
    if (id >= 0 && id <= 5) GetUIObject(jet::String(k_names[id]))->SetVisible(true);
}

// @0x4d4f38: the price of the location is paid (the box then is refreshed by the unlocked location), and the player goes there
// when the data is on the device
void LocationBox::BuyLocation(int a, int b) {
    (void)a;
    (void)b;
    Price *price = m_info->GetUnlockByPricePricePtr();
    if (!Player::GetInstance()->ExecuteCost(*price, std::bind(&LocationBox::BuyLocation, this, std::placeholders::_1, std::placeholders::_2))) return;
    SoundMgr::GetInstance()->PlayIfNotPlaying(Menu_Base::k_sfx_buy, 0);  // and GameTrackingMgr::SendCurrencySpentEvent (not ported)
    Player::GetInstance()->SetLocationUnlocked(m_info->GetLocationId());
    Game::GetInstance()->Save(false, false);
    if (Game::GetInstance()->HasDLCDataFor(m_info->GetLocationId(), true)) FastTravelToLocation();
}

// @0x4d5114: the data of the location is asked for (or the download that has begun is cancelled)
void LocationBox::downloadDlc() {
    MenuMgr *mm = MenuMgr::GetInstance();
    mm->DisableTouchesFor(200);
    Game *game = Game::GetInstance();
    int id = m_info->GetLocationId();
    if (game->IsDlcAvailableOnServer()) {
        m_downloadButton->SetText(Text("CONNECTIVITY_DOWLOADING"));
        m_downloadBar->SetVisible(true);
        if (game->IsLocationDlcRequested(id) && game->GetLocationDlcProgress(id) > 0) game->CancelLocationDlc(id);
        else {
            Menu_Base *menu = mm->GetCurrentPlayingMenu();
            bool endScreen = menu && menu->GetMenuName().Equals(Menu_ResultEndScreen::k_menuName);
            game->DownloadLocationDlc(id, endScreen ? Game::DlcTracking_EndScreen : Game::DlcTracking_MainMenu);
        }
    } else
        PopupMgr::GetInstance()->PushInfoPopup(std::string(Text("POPUP_NO_INTERNET").c_str()));
}

// @0x4d533c: the player can only go where the data is
void LocationBox::BuyFastTravel(int, int) {
    if (!Game::GetInstance()->HasDLCDataFor(m_info->GetLocationId(), true)) return;
    FastTravelToLocation();
}

// @0x4d538c: after the first run the player runs in the location (the game goes back to the main menu with it on the next
// refresh)
void LocationBox::FastTravelToLocation() {
    bool ok = TutorialMgr::GetInstance()->IsTutorialFinished(ETutorialId::k_firstPlay) && Game::GetInstance()->HasDLCDataFor(m_info->GetLocationId(), true);
    if (ok) {
        Player::GetInstance()->SetCurrentLocation(m_info->GetLocationId());
        Statistics::GetInstance()->Location_IncreaseEnterLocationCount(m_info->GetLocationId());
        Game::GetInstance()->Save(false, false);
        m_travelled = true;
    }
    m_needRefresh = true;
}

// @0x4d549c
int LocationBox::GetScoreMultiplierValue() { return m_info->GetScoreMultiplierValue(); }

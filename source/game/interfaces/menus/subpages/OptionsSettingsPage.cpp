// source/game/interfaces/menus/subpages/OptionsSettingsPage.cpp
//
// Not ported (see OptionsSettingsPage.h): SoundMgr, PopupMgr, the browser, the permission of the push notifications, the movie
// of the video button, the online profile and the CRM; the control flow around them is the .so's.
#include "OptionsSettingsPage.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "game/Game.h"
#include "game/gamemanagers/FontInfoMgr.h"
#include "game/interfaces/GameSettings.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfacePBar.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/online/store/Store.h"
#include "localization/babel.h"
#include "recorddb.h"
#include "version.h"

// the permission of the push notifications (SimplifiedPN_IsNotificationPermissionEnabled @0x3ef900 asks Android: here it is
// there), and the flag the page keeps while it waits for the answer (isRequestingPermission)
static bool SimplifiedPN_IsNotificationPermissionEnabled() { return true; }
static bool isRequestingPermission = false;

static const babel::Lang::type k_languages[12] = { babel::Lang::EN, babel::Lang::FR, babel::Lang::ES, babel::Lang::IT, babel::Lang::DE, babel::Lang::PT,
                                                   babel::Lang::JA, babel::Lang::KO, babel::Lang::ZH, babel::Lang::RU, babel::Lang::TR, babel::Lang::AR };
static const char *const k_languageButtonNames[12] = { "EN_Button", "FR_Button", "ES_Button", "IT_Button", "DE_Button", "PT_Button",
                                                       "JA_Button", "KO_Button", "ZH_Button", "RU_Button", "TR_Button", "AR_Button" };

// @0x4a4768: the buttons of the languages, the graphs and bars of the ones that come as DLC, the volume bars and buttons
OptionsSettingsPage::OptionsSettingsPage()
    : BasicPage(jet::String("Options_Settings")), m_dlcLanguage(0u), m_dlcLanguageButton(0), m_downloadingButton(0), m_musicBar(0), m_musicMoreButton(0), m_musicLessButton(0),
      m_sfxBar(0), m_sfxMoreButton(0), m_sfxLessButton(0), m_supportButton(0), m_supportLoading(0), m_videoButton(0), m_pushButton(0), m_languageChanged(false) {
    SetSize(GetUIObject(jet::String("Size"))->GetSize());
    for (int i = 0; i < 12; i++) m_languageButtons[k_languages[i]] = GetUIButton(jet::String(k_languageButtonNames[i]));
    // the graphs of the download (an icon on the button) and of the downloading (over the bar): only the languages that come as DLC have them
    const char *const dlcNames[5] = { "JA", "KO", "ZH", "RU", "TR" };
    const babel::Lang::type dlcLangs[5] = { babel::Lang::JA, babel::Lang::KO, babel::Lang::ZH, babel::Lang::RU, babel::Lang::TR };
    for (int i = 0; i < 12; i++) {
        m_languageDownloadGraphs[k_languages[i]] = 0;
        m_languageDownloadingGraphs[k_languages[i]] = 0;
    }
    for (int i = 0; i < 5; i++) {
        m_languageDownloadGraphs[dlcLangs[i]] = GetUIGraph(jet::String(dlcNames[i]) + jet::String("_download"));
        m_languageDownloadingGraphs[dlcLangs[i]] = GetUIGraph(jet::String(dlcNames[i]) + jet::String("_downloading"));
    }
    // what each language needs from the DLC: the fonts of Japanese, Chinese, Korean and Turkish/Russian
    for (int i = 0; i < 12; i++) {
        m_languageDlcAssets[k_languages[i]] = -1;
        m_languageDownloadBars[k_languages[i]] = 0;
        m_languageDownloading[k_languages[i]] = false;
    }
    m_languageDlcAssets[babel::Lang::JA] = Game::OnDemandDlc_FontJa;
    m_languageDlcAssets[babel::Lang::KO] = Game::OnDemandDlc_FontKo;
    m_languageDlcAssets[babel::Lang::ZH] = Game::OnDemandDlc_FontZh;
    m_languageDlcAssets[babel::Lang::RU] = Game::OnDemandDlc_FontTrRu;
    m_languageDlcAssets[babel::Lang::TR] = Game::OnDemandDlc_FontTrRu;
    // the progress of a download is drawn on the button of its language: the sprite and animations of the button
    for (int i = 0; i < 5; i++) {
        InterfaceButton *button = m_languageButtons[dlcLangs[i]];
        math::vec2<float> pos = button->GetPos();
        m_languageDownloadBars[dlcLangs[i]] =
            ManualAddPBar(0, pos, InterfacePBar::LeftToRight, button->GetSpriteIdx(), button->GetIdleAnim(), button->GetSelectedAnim(), -1, -1, -1, 1);
    }
    m_downloadingButton = 0;
    for (std::map<babel::Lang, InterfacePBar*>::iterator it = m_languageDownloadBars.begin(); it != m_languageDownloadBars.end(); ++it) {
        if (!it->second) continue;
        it->second->SetVisible(false);
        if (m_languageDownloadingGraphs[it->first]) m_languageDownloadingGraphs[it->first]->SetParent(it->second);
    }
    m_musicBar = GetUIPBar(jet::String("MusicPBar"));
    m_musicMoreButton = GetUIButton(jet::String("MusicMoreButton"));
    m_musicLessButton = GetUIButton(jet::String("MusicLessButton"));
    m_sfxBar = GetUIPBar(jet::String("SFXPBar"));
    m_sfxMoreButton = GetUIButton(jet::String("SFXMoreButton"));
    m_sfxLessButton = GetUIButton(jet::String("SFXLessButton"));
    m_supportButton = GetUIButton(jet::String("Support_button"));
    m_supportLoading = GetUIObject(jet::String("Support_loading"));
    m_pushButton = GetUIButton(jet::String("PushButton"));
    GetUIGraph(jet::String("PushButton_locked"))->SetVisible(false);
    m_videoButton = GetUIButton(jet::String("Video_Button"));
}

OptionsSettingsPage::~OptionsSettingsPage() {}

// @0x4a64e4: the buttons of the page, then the downloads of the DLC languages
void OptionsSettingsPage::Update(int dt) {
    if (isRequestingPermission && SimplifiedPN_IsNotificationPermissionEnabled()) {
        isRequestingPermission = false;
        m_pushButton->SetSelected(true);
        GameSettings::GetInstance()->SetLocalNotificationsEnabled(true);
    }
    BasicPage::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    Game *game = Game::GetInstance();
    const int count = mm->ACTION_GetNumReleasedButtons();
    for (int i = 0; i < count; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_supportButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            if (game->AreOnlineServicesReady()) {
                // nativeOpenBrowser("https://201205igp.gameloft.com/redir/?from=" + game code + "&op=GAND&ctg=SUPPORT"): an Android call
            }
            // else PopupMgr::PushInfoPopup(POPUP_NO_INTERNET)
            break;
        }
        if (HandleLangButton(b)) {
            mm->RefreshMenus();
            break;
        }
        if (HandleSoundButton(b)) break;
        if (b == m_videoButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            // GS_MoviePlayback::StartClaraMovie(GS_Loading::GoTo_Options): the movie state is not ported (the
            // button is hidden by Refresh)
            return;
        }
        if (b == m_pushButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            GameSettings *gs = GameSettings::GetInstance();
            if (gs->AreLocalNotificationsEnabled()) {
                m_pushButton->SetSelected(false);
                gs->SetLocalNotificationsEnabled(false);
            } else if (!SimplifiedPN_IsNotificationPermissionEnabled()) {
                isRequestingPermission = true;  // SimplifiedPN_RequestNotificationPermission
            } else {
                m_pushButton->SetSelected(true);
                gs->SetLocalNotificationsEnabled(true);
            }
        }
    }
    // the downloads that finished: the languages that were waiting for them are chosen
    if (game->IsDlcOnDemandFinished()) {
        game->SetDisplayedInitialScreens(false);
        bool changed = false;
        const babel::Lang selected = babel::Babel::GetInstance()->GetSelectedLanguage();
        InterfaceButton *downloading = m_downloadingButton;
        for (std::map<babel::Lang, bool>::iterator it = m_languageDownloading.begin(); it != m_languageDownloading.end(); ++it) {
            if (!it->second) continue;
            if (m_languageButtons[it->first] != downloading) {
                if (HandleLangButton(m_languageButtons[it->first])) mm->RefreshMenus();
                changed = true;
            }
        }
        if (downloading) {
            HandleLangButton(downloading);
        } else if (changed) {
            HandleLangButton(m_languageButtons[selected]);
        } else {
            goto bars;
        }
        mm->RefreshMenus();
    }
bars:
    // the bars of the downloads fill, and show while the DLC is not all there
    for (std::map<babel::Lang, InterfacePBar*>::iterator it = m_languageDownloadBars.begin(); it != m_languageDownloadBars.end(); ++it) {
        const babel::Lang lang = it->first;
        if (m_languageDlcAssets[lang] == -1 || !m_languageDownloadBars[lang]) continue;
        if (!m_languageDownloadBars[lang]->IsVisible()) continue;
        const float progress = game->GetOnDemandDlcProgress(Game::k_onDemandDlcAssetNames[m_languageDlcAssets[lang]]);
        const bool finished = progress == 1.0f ? game->IsDlcOnDemandFinished() : false;
        m_languageDownloadBars[lang]->SetValue(progress);
        m_languageDownloadBars[lang]->SetVisible(!finished);
        m_languageButtons[lang]->SetVisible(finished);
        m_languageDownloadGraphs[lang]->SetVisible(!finished);
    }
    // IsIAPLinkOpenerBusy @0x4a4760: the link of the support is never being opened here
    m_supportLoading->SetVisible(false);
}

// @0x4a6aac
void OptionsSettingsPage::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    if (IsVisible()) BasicPage::Render(cam, p);
}

// @0x4a6ae4: no download is the one of the button last pressed; the label of the languages is not shown
void OptionsSettingsPage::OnEnter() {
    m_downloadingButton = 0;
    Refresh();
    GetUIText(jet::String("LanguageText"))->SetVisible(false);
    GetUIText(jet::String("LanguageText"))->SetVisible(false);
    Game *game = Game::GetInstance();
    if (!game->IsDlcOnDemandFinished()) return;
    for (std::map<babel::Lang, bool>::iterator it = m_languageDownloading.begin(); it != m_languageDownloading.end(); ++it)
        if (it->second) it->second = false;
}

// @0x4a6c08: the language of the game is the selected button, the bars of the downloads show for the languages that download,
// the download icon for those that cannot show their fonts yet; the bars of the volumes show the volumes
void OptionsSettingsPage::Refresh() {
    GetUIObject(jet::String("SoundBG"))->SetVisible(true);
    GetUIText(jet::String("LanguageText"))->SetVisible(true);
    m_videoButton->SetVisible(false);
    const babel::Lang selected = babel::Babel::GetInstance()->GetSelectedLanguage();
    m_languageButtons[selected]->SetSelected(true);
    Game::GetInstance()->SetDlcLanguage(selected);
    for (std::map<babel::Lang, InterfaceGraph*>::iterator it = m_languageDownloadGraphs.begin(); it != m_languageDownloadGraphs.end(); ++it) {
        const babel::Lang lang = it->first;
        if (m_languageDownloadBars[lang]) m_languageDownloadBars[lang]->SetVisible(m_languageDownloading[lang]);
        if (it->second) {
            const bool canLoad = FontInfoMgr::GetInstance()->CanLoadFontsFor(lang);
            it->second->SetVisible(m_languageDownloading[lang] ? false : !canLoad);
        }
        m_languageButtons[lang]->SetVisible(!m_languageDownloading[lang]);
    }
    // the volumes are the saved ones: SoundMgr::GetGroupVolume gives what GameSettings saved of them
    m_musicBar->SetValue(GameSettings::GetInstance()->GetSavedMusicVolume());
    m_sfxBar->SetValue(GameSettings::GetInstance()->GetSavedSoundVolume());
    m_pushButton->SetSelected(GameSettings::GetInstance()->AreLocalNotificationsEnabled());
}

// @0x4a6f04: a language button: a language the fonts of are there is chosen (and saved); one that needs the DLC asks to download it
bool OptionsSettingsPage::HandleLangButton(InterfaceButton *button) {
    for (std::map<babel::Lang, InterfaceButton*>::iterator it = m_languageButtons.begin(); it != m_languageButtons.end(); ++it) {
        if (button != it->second) continue;
        const babel::Lang lang = it->first;
        SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
        m_downloadingButton = 0;
        babel::Babel *babel = babel::Babel::GetInstance();
        const babel::Lang current = babel->GetSelectedLanguage();
        if (current == lang) return true;
        Game *game = Game::GetInstance();
        if (FontInfoMgr::GetInstance()->CanLoadFontsFor(lang)) {
            m_languageButtons[current]->SetSelected(false);
            button->SetSelected(true);
            game->LoadFonts(lang);
            babel->SelectLanguage(lang, true);
            // Store::RefreshIAPTokenList: the list of the tokens of the store is not ported
            m_languageDownloading[lang] = false;
            game->SetDlcLanguage(lang);
            GameSettings *gs = GameSettings::GetInstance();
            gs->GetGameSettings().Set(jet::String("Language"), clara::Record((int)lang.value()));
            gs->SaveGameSettings();
            m_languageChanged = true;
            return true;
        }
        if (m_languageDlcAssets[lang] != -1) {
            if (!game->AreOnlineServicesReady()) {
                // PopupMgr::PushInfoPopup(POPUP_NO_INTERNET)
                return false;
            }
            // the question "download the fonts (size)?" (PopupQuestion, answered by ConfirmDownloadDlc, which does nothing)
            m_dlcLanguage = lang;
            m_dlcLanguageButton = button;
        }
        m_languageChanged = false;
        return false;
    }
    return false;
}

// @0x4a7568: the DLC of the language starts to download; its bar shows in place of its button
void OptionsSettingsPage::downloadDlc() {
    Game *game = Game::GetInstance();
    game->SetDisplayedInitialScreens(true);
    m_downloadingButton = m_dlcLanguageButton;
    game->DownloadOnDemandDlc(Game::k_onDemandDlcAssetNames[m_languageDlcAssets[m_dlcLanguage]], Game::DlcTracking_None);
    game->SetDlcLanguage(m_dlcLanguage);
    m_languageDownloading[m_dlcLanguage] = true;
    if (!m_languageDownloadBars[m_dlcLanguage]) return;
    m_languageDownloadBars[m_dlcLanguage]->SetVisible(true);
    m_downloadingButton->SetVisible(false);
}

// @0x4a7630: the buttons of the volumes move the volume of their group by a tenth
bool OptionsSettingsPage::HandleSoundButton(InterfaceButton *button) {
    if (button != m_musicLessButton && button != m_musicMoreButton && button != m_sfxLessButton && button != m_sfxMoreButton) return false;
    const bool music = button == m_musicLessButton || button == m_musicMoreButton;
    const bool more = button == m_musicMoreButton || button == m_sfxMoreButton;
    InterfacePBar *bar = music ? m_musicBar : m_sfxBar;
    GameSettings *gs = GameSettings::GetInstance();
    float volume = (music ? gs->GetSavedMusicVolume() : gs->GetSavedSoundVolume()) + (more ? 0.1f : -0.1f);
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    bar->SetValue(volume);
    SoundMgr *sm = SoundMgr::GetInstance();
    if (music) {
        gs->SetMusicVolume(volume);
        sm->SetGroupVolume(SoundMgr::k_musicGroupLabel, volume, 0);
        if (volume > 0) Game::GetInstance()->OnGameMusicPlay();
    } else {
        gs->SetSoundVolume(volume);
        sm->SetGroupVolume(SoundMgr::k_sfxGroupLabel, volume, 0);
        sm->SetGroupVolume(SoundMgr::k_uiGroupLabel, volume, 0);
    }
    SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
    return true;
}

// @0x4a77dc: leaving the options after a language was chosen sends it to the online profile and the CRM (not ported)
void OptionsSettingsPage::OnMenuQuit() {
    Game::GetInstance()->SetDisplayedInitialScreens(false);
    if (!m_languageChanged) return;
    // Store::GetInstance()->[0x88] = true (the store reloads its tokens' texts); StandardProfileMgr::AddFieldToProfile("language", the
    // code of the language) and UpdateProfile; CrmConfigMgr::UpdateCRMLanguage
    m_languageChanged = false;
}

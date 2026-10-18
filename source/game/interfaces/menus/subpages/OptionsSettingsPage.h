// source/game/interfaces/menus/subpages/OptionsSettingsPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_OPTIONSSETTINGSPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_OPTIONSSETTINGSPAGE_H

#include <map>
#include "game/interfaces/interfaceobjects/BasicPage.h"
#include "localization/lang.h"

class InterfacePBar;

// The settings of the options ("Options_Settings" layer): the music and sound volumes, the language buttons (the languages
// whose fonts come as on demand DLC show a download icon and a progress bar on their button), the support button and the
// push notifications button. 0x28c bytes in the .so.
// Not ported: the sounds (SoundMgr: the volumes are the saved ones of GameSettings, which SoundMgr applies), the popups
// (PopupMgr: "no internet", the question before a download), the browser (an Android native), the permission of the push
// notifications (SimplifiedPN: granted), the movie of the video button (hidden by Refresh, GS_MoviePlayback is not
// ported), and the online profile and CRM updates of a changed language.
class OptionsSettingsPage : public BasicPage {
public:
    OptionsSettingsPage();
    virtual ~OptionsSettingsPage();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnMenuQuit();
    void OnEnter();
    void Refresh();
private:
    bool HandleLangButton(InterfaceButton*);
    bool HandleSoundButton(InterfaceButton*);
    void downloadDlc();
    std::map<babel::Lang, InterfaceButton*> m_languageButtons;          // 0x1BC
    babel::Lang m_dlcLanguage;                                          // 0x1D4: the language a download was asked for
    InterfaceButton *m_dlcLanguageButton;                               // 0x1D8
    std::map<babel::Lang, int> m_languageDlcAssets;                     // 0x1DC: Game::OnDemandDlcAssets, -1 for none
    InterfaceButton *m_downloadingButton;                               // 0x1F4
    std::map<babel::Lang, InterfacePBar*> m_languageDownloadBars;       // 0x1F8
    std::map<babel::Lang, bool> m_languageDownloading;                  // 0x210
    std::map<babel::Lang, InterfaceGraph*> m_languageDownloadGraphs;    // 0x228
    std::map<babel::Lang, InterfaceGraph*> m_languageDownloadingGraphs; // 0x240
    InterfacePBar *m_musicBar;          // 0x258
    InterfaceButton *m_musicMoreButton; // 0x25C
    InterfaceButton *m_musicLessButton; // 0x260
    InterfacePBar *m_sfxBar;            // 0x264
    InterfaceButton *m_sfxMoreButton;   // 0x268
    InterfaceButton *m_sfxLessButton;   // 0x26C
    InterfaceButton *m_supportButton;   // 0x270
    InterfaceObject *m_supportLoading;  // 0x274
    InterfaceButton *m_videoButton;     // 0x278
    InterfaceButton *m_pushButton;      // 0x284
    bool m_languageChanged;             // 0x288
};

#endif

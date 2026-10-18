// source/game/interfaces/menus/subpages/OptionsAboutPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_OPTIONSABOUTPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_OPTIONSABOUTPAGE_H

#include "game/interfaces/interfaceobjects/BasicPage.h"

// The about page of the options ("Options_About"): the contact / privacy / terms buttons and under them the credits,
// the texts STR_ABOUT_START, STR_ABOUT_VERSION, STR_ABOUT_001 ... drawn line by line in the font and palette each
// one names ("#Ffp" in front of the text)
class OptionsAboutPage : public BasicPage {  // sizeof 0x1D4
public:
    OptionsAboutPage();
    virtual ~OptionsAboutPage();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void Refresh();
    static const jet::String k_startText, k_versionText, k_startNumberText, k_CMCCText, k_CTText, k_OperatorText;
private:
    void ParseFontTypeAndPalette(jet::String&, int&, int&);
    jet::String m_startText;                // 0x1BC
    InterfaceText *m_textArea;              // 0x1C0
    InterfaceButton *m_privacyPolicyButton; // 0x1C4
    InterfaceButton *m_customerCareButton;  // 0x1C8
    InterfaceButton *m_termsButton;         // 0x1CC
    bool m_showContact;                     // 0x1D0
};

#endif

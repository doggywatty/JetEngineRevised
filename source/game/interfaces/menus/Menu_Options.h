// source/game/interfaces/menus/Menu_Options.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_OPTIONS_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_OPTIONS_H

#include <vector>
#include "Menu_Base.h"

class InterfaceScrollBar;
class OptionsSettingsPage;
class OptionsConnectionPage;
class OptionsHelpPage;
class OptionsAboutPage;
class RedeemCodePage;

// the globals the options and the banner share (@0xd0b904 isMenuOption, @0xd0b8ee isBannerVisible, @0xd0b90c isUpdateOnce,
// @0xd0df28 g_bBackToMenuOption)
extern bool isMenuOption;
extern bool isBannerVisible;
extern bool isUpdateOnce;
extern bool g_bBackToMenuOption;

// The options ("Options_Common" layer): a row of tabs over a list that holds the page of the tab (settings, help, about;
// the redeem code and the connectivity ones are only there with the social features, which this build has off) and a back
// button. 0x178 bytes in the .so.
// Not ported: the sounds (SoundMgr), the tracking of the screens (GameTrackingMgr), the debug logs and the ads banner's
// own handling (only its flag).
class Menu_Options : public Menu_Base {
public:
    // the tab shown; the order of the tab buttons
    enum EStateDef { Options_Settings = 0, Options_Help = 1, Options_About = 2, Options_RedeemCode = 3, Options_Connection = 4, Options_None = 5 };
    Menu_Options();
    virtual ~Menu_Options();
    virtual const jet::String &GetMenuName() const { return k_menuName; }
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Refresh();
    virtual void OnEnter();
    virtual void OnQuit();
    virtual bool RenderGameInBG() { return true; }
    virtual bool ShowBanner() const { return false; }
    virtual int GetCurrentTrackingScreen() const { return GetTrackingState(m_tab); }
    bool AnyTabWithNotification() { return false; }
    // @0x48c320: the connectivity tab opens when the social features are there, the content is checked at the next refresh
    void RequestCheckForContent();
    // @0x48c368
    bool CanAllowDlcErrorMessages();
    static const jet::String k_menuName;

private:
    void SetTab(int tab);                                                    // @0x48c1f0
    static int GetTrackingState(int tab);                                    // @0x48c234
    void UpdateTabNotify(int tab, bool force, bool notify, const jet::String &name0, const jet::String &name1);  // @0x48c258

    std::vector<InterfaceButton*> m_tabButtons;   // 0x124
    InterfaceButton *m_backButton;                // 0x130
    InterfaceButton *m_ccButton;                  // 0x134
    InterfaceList *m_subpagesParent;              // 0x138
    InterfaceScrollBar *m_subpagesScroll;         // 0x13c
    InterfaceText *m_textCategory;                // 0x140
    std::vector<jet::String> m_tabHeaderStrings;  // 0x144
    int m_tab;                                    // 0x150
    int m_shownTab;                               // 0x154: the tab the pages were last refreshed for
    bool m_checkContent;                          // 0x158
    OptionsSettingsPage *m_settingsPage;          // 0x15c
    OptionsConnectionPage *m_connectionPage;      // 0x160
    OptionsHelpPage *m_helpPage;                  // 0x164
    OptionsAboutPage *m_aboutPage;                // 0x168
    RedeemCodePage *m_redeemCodePage;             // 0x16c
};

// The contact information of the countries ("Options_Contact" layer), opened from the about page.
class ContactBox;
class InterfaceGrid;
class Menu_OptionsContact : public Menu_Base {  // sizeof 0x13c
public:
    Menu_OptionsContact();
    virtual ~Menu_OptionsContact();
    virtual const jet::String &GetMenuName() const { return k_menuName; }
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnEnter();
    virtual void OnQuit();
    static const jet::String k_menuName;
private:
    InterfaceButton *m_backButton;           // 0x124
    InterfaceGrid *m_grid;                   // 0x128
    InterfaceScrollBar *m_scrollBar;         // 0x12c
    std::vector<ContactBox*> m_boxes;        // 0x130
};

#endif

// source/game/interfaces/menus/subpages/OptionsHelpPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_OPTIONSHELPPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_OPTIONSHELPPAGE_H

#include <set>
#include "game/interfaces/interfaceobjects/BasicPage.h"

// The help of the options ("Options_Help"): the texts HELP_TITLE_nn / HELP_DESCRIPTION_nn one page at a time, turned
// with the arrows or a swipe; some pages are left out (the platform's, the social ones without social features)
class OptionsHelpPage : public BasicPage {  // sizeof 0x218
public:
    OptionsHelpPage();
    virtual ~OptionsHelpPage();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void OnEnter();
    void Refresh();
    static const jet::String k_keyTitle, k_keyDescription;
private:
    void SetText();
    void HandleArrows();
    void PreviousPage();
    void NextPage();
    void SetFirstPage();
    void DisablePages();
    void DisablePage(jet::String);
    void UpdateBySwipe();
    jet::String m_keyTitle;            // 0x1BC
    jet::String m_keyDescription;      // 0x1C0
    unsigned m_currentElement;         // 0x1C4: the number of the keys shown
    unsigned m_totalElements;          // 0x1C8
    InterfaceButton *m_previousButton; // 0x1CC
    InterfaceButton *m_nextButton;     // 0x1D0
    InterfaceText *m_pageText;         // 0x1D4
    unsigned m_currentPage;            // 0x1D8
    unsigned m_totalPages;             // 0x1DC
    InterfaceText *m_titleText;        // 0x1E0
    InterfaceText *m_contentsText;     // 0x1E4
    math::rect<float> m_contentsArea;  // 0x1E8: where a swipe turns the page
    math::vec2<float> m_lastTouch;     // 0x1F8: where the swipe started
    std::set<jet::String> m_disabledPages;  // 0x200
};

#endif

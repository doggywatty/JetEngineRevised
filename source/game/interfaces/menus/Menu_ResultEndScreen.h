// source/game/interfaces/menus/Menu_ResultEndScreen.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_RESULTENDSCREEN_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_RESULTENDSCREEN_H

// See Menu_Shop.h for why these additions were needed.
#include "Menu_Base.h"
#include "jenum.h"
#include "core/delayer.h"
#include "core/interputils.h"
#include "game/online/tracking/tracking.h"
class EndScreenBasePage;
class BananasAndTokensBox;
class Price;

class Menu_ResultEndScreen : public Menu_Base {  // sizeof 0x1AC
public:
    // Not in the PDB header, and not recovered from disassembly -- Menu_ResultEndScreen::Exit()
    // (0x4913b4) just stores the value unconditionally with no visible bounds check, so its real
    // range isn't known. Placeholder only; nothing in this session's minimal .cpp calls Exit() or
    // reads m_exitOption meaningfully. Give this real values if a later session wires Exit() up.
    struct EExitOptionsDef { enum type { Exit_None = 0, Exit_MainMenu = 1 }; };
    virtual const jet::String &GetMenuName() const;
    Menu_ResultEndScreen();
    virtual ~Menu_ResultEndScreen();
    virtual bool UpdateGameInBG();
    virtual bool RenderGameInBG();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnEnter();
    virtual void OnQuit();
    virtual void Refresh();
    void SetFirstShow();
    // @0x492688: the costume of the result screen's photo ("Common_photo") takes its idle state
    void RefreshCostumes();
    void AddClaimedPriceAmount(const Price&);
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
private:
    void Exit(safe_enum<Menu_ResultEndScreen::EExitOptionsDef,enum Menu_ResultEndScreen::EExitOptionsDef::type>);
    void Subpages_SetTargetPage(int);
    void Subpages_Update(int);
    void Subpages_SetOffset(const math::vec2<float>&);
    void Subpages_OnEnterInit();
    void Subpages_Refresh();
    void Subpages_CheckShowPrevNextButtons();
    void Subpages_UpdateAutoScrolling(int);
    void Subpages_UpdateNextButtonPress(bool);
    void Subpages_SetOffsetDelayer(float, float);
    void UpdateClaimedInfo();
public:
    static const jet::String k_menuName;
private:
    safe_enum<Menu_ResultEndScreen::EExitOptionsDef,enum Menu_ResultEndScreen::EExitOptionsDef::type> m_exitOption;  // 0x124
    InterfaceText *m_subpageInfoTitle;  // 0x128
    int m_curSubPageId;  // 0x12C
    int m_targetPageId;  // 0x130
    std::vector<EndScreenBasePage *,std::allocator<EndScreenBasePage *> > m_subPages;  // 0x134
    InterfaceButton *m_playAgainButton;  // 0x148
    InterfaceButton *m_nextNextButton;  // 0x14C
    InterfaceButton *m_prevPageButton;  // 0x150
    InterfaceButton *m_nextPageButton;  // 0x154
    InterfaceButton *m_shareButton;  // 0x158
    InterfaceButton *m_backToMenuButton;  // 0x15C
    InterfaceButton *m_shopButton;  // 0x160
    InterfaceText *m_textBananas;  // 0x164
    InterfaceArea *m_subpagesParentArea;  // 0x168
    math::rect<float> m_subpagesClip;  // 0x16C
    BananasAndTokensBox *m_bananasAndTokensBox;  // 0x17C
    bool m_isFirstShow;  // 0x180
    static const int k_autoScrollingPageTimer;
    bool m_isAutoScrollingEnabled;  // 0x181
    int m_autoScrollinPageTimer;  // 0x184
    bool m_isNextPageEnabled;  // 0x188
    static const int k_pageOffsetDelayTime;
    jet::core::Delayer<float,jet::core::SmootherStep> m_pageOffsetDelayer;  // 0x18C
    int m_claimedBananas;  // 0x1A4
    int m_claimedTokens;  // 0x1A8
};

#endif

// source/game/interfaces/popups/BasePopup.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_BASEPOPUP_H
#define SOURCE_GAME_INTERFACES_POPUPS_BASEPOPUP_H

#include "game/interfaces/interfaceobjects/BasicPage.h"

class GameState;
class Menu_Base;

// A window over the game: a page of the Aurora layer it is made from, shown on the PopupMgr's stack. It can depend on a
// game state or a menu (it is only shown there) and can tell if it may be shown while the game runs.
class BasePopup : public BasicPage {  // sizeof 0x1C8
public:
    explicit BasePopup(const jet::String &layerName);
    virtual ~BasePopup();
    virtual void Update(int dt) { BasicPage::Update(dt); }                          // @0x4aa354
    virtual void Render(const jet::scene::Camera &c, jet::video::Painter &p) { BasicPage::Render(c, p); }
    virtual void RefreshPopup() { RefreshCostumes(); }                             // @0x4aa35c
    virtual bool CanShowInActionPhase() const { return m_canShowInActionPhase; }   // @0x43e688
    virtual void SetGameStateDependency(GameState *s) { m_gameStateDependency = s; }  // @0x43e690
    virtual GameState *GetGameStateDependency() const { return m_gameStateDependency; }
    virtual void SetMenuDependency(Menu_Base *m) { m_menuDependency = m; }
    virtual Menu_Base *GetMenuDependency() const { return m_menuDependency; }
    virtual bool ShowBanner() const { return true; }                                // @0x43e6b0
    virtual int GetCurrentTrackingScreen() const { return -1; }                     // @0x43e6b8
    virtual void RefreshCostumes() {}                                               // @0x43e6c0
    virtual void OnSuspend() {}
    virtual void OnResume() {}
    // @0x4aa370: the colour the render target clears to while the popup is up
    virtual void GetClearColor(float c[4]) const { c[0] = 0.4f; c[1] = 0.6f; c[2] = 0.8f; c[3] = 1.0f; }
    static const jet::String k_sfx_menu_message_window_pop_up;
protected:
    bool m_refreshOnNextFrame;     // 0x1BC
    bool m_canShowInActionPhase;   // 0x1BD
    GameState *m_gameStateDependency;  // 0x1C0
    Menu_Base *m_menuDependency;       // 0x1C4
};

#endif

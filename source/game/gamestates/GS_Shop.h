// source/game/gamestates/GS_Shop.h
#ifndef SOURCE_GAME_GAMESTATES_GS_SHOP_H
#define SOURCE_GAME_GAMESTATES_GS_SHOP_H

#include "GameState.h"
#include "game/interfaces/menus/Menu_Shop.h"

class CameraController;

// The shop: its menu on top of the level. It remembers the camera the level had and gives it back when it is suspended.
// Not ported: the curvature of the world that the .so saves and flattens for the shop (render state), and the sound groups
// that are stopped when it opens over something that is not the main menu (SoundMgr).
class GS_Shop : public GameState {  // sizeof 0x1C
public:
    virtual const jet::String &GetStateName();
    GS_Shop();
    virtual ~GS_Shop();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState() {}
    virtual void RenderState();
    // opens the shop (when it is not) at the tab
    static void GoToTab(Menu_Shop::EShopTabs);
    static const jet::String k_stateName;
    // @0xd0df48 and @0xd0df49: the shop was asked for the bonus tab, and it was called by the main menu
    static bool checkCallShop, checkBonusBack;
private:
    CameraController *m_cameraController;  // 0x10
};

#endif

// source/game/gamestates/GS_Shop.cpp
#include "GS_Shop.h"
#include "GS_MainMenu.h"
#include "camera/cameramgr.h"
#include "game/GameLevel.h"
#include "game/entities/LevelDef.h"
#include "game/interfaces/menus/MenuMgr.h"

const jet::String GS_Shop::k_stateName("GS_Shop");
bool GS_Shop::checkCallShop = false, GS_Shop::checkBonusBack = false;

// @0x503e2c: the camera is kept, and the shop's menu is put on
GS_Shop::GS_Shop() : m_cameraController(CameraMgr::GetInstance()->GetCurrentController()) { MenuMgr::GetInstance()->PushMenu(Menu_Shop::k_menuName); }
// @0x503f98
GS_Shop::~GS_Shop() {}
const jet::String &GS_Shop::GetStateName() { return k_stateName; }

// @0x504018: the level takes the shop's setup and camera; the costumes tab shows its costumes again
void GS_Shop::ResumeState() {
    LevelDef::GetInstance()->OnInitShopMenu();
    CameraMgr::GetInstance()->Update(99999);
    if (Menu_Shop *m = static_cast<Menu_Shop*>(MenuMgr::GetInstance()->GetMenu(Menu_Shop::k_menuName)))
        if (m->GetCurrentTab() == Menu_Shop::Shop_Costumes) m->RefreshCostumes();
}

// @0x50409c: the camera the level had comes back
void GS_Shop::SuspendState() {
    CameraMgr *cm = CameraMgr::GetInstance();
    if (!cm) return;
    cm->SetCamera(m_cameraController, 0, 0);
    cm->Update(99999);
}

void GS_Shop::RenderState() { GameLevel::GetInstance()->Render(); }

// @0x495508
void GS_Shop::GoToTab(Menu_Shop::EShopTabs tab) {
    GameState *cur = GameState::GetCrtState();
    if (!cur->GetStateName().Equals(k_stateName)) {
        if (cur->GetStateName().Equals(GS_MainMenu::k_stateName)) static_cast<GS_MainMenu*>(cur)->StopMainMenuSfx();
        GameState::PushState(new GS_Shop(), true);
    }
    checkBonusBack = tab == Menu_Shop::Shop_Bonus;
    static_cast<Menu_Shop*>(MenuMgr::GetInstance()->GetMenu(Menu_Shop::k_menuName))->GoToTab(tab, false, false);
}

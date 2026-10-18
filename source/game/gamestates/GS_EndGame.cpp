// source/game/gamestates/GS_EndGame.cpp
#include "GS_EndGame.h"
#include "game/GameLevel.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/interfaces/menus/Menu_DeathRevive.h"
#include "game/interfaces/menus/Menu_ResultEndScreen.h"

const jet::String GS_EndGame::k_stateName("GS_EndGame");

// @0x4fc030
const jet::String &GS_EndGame::GetStateName() { return k_stateName; }

// @0x4fc04c: the revive menu is offered when the level allows a revive (GameLevel::CanRevive, always true in the .so): the menu gets its
// buy incentive (Menu_DeathRevive::ComputeBuyIncentiveData), its flag at +0x12D is set, and it is pushed. Without a revive the run goes
// on to the result screen at once (GameLevel::OnMinionNoRevive). The state id is 4.
// Not ported: the .so then sets the DLC installer's performance mode (DLCManager::SetPerformanceSettings(1), 0x4656fc); the DLC
// installer is not in this tree.
GS_EndGame::GS_EndGame() {
    GameLevel *level = GameLevel::GetInstance();
    if (level->CanRevive()) {
        MenuMgr *menus = MenuMgr::GetInstance();
        Menu_DeathRevive *menu = static_cast<Menu_DeathRevive*>(menus->GetMenu(Menu_DeathRevive::k_menuName));
        menu->ComputeBuyIncentiveData();
        menu->SetFlag12D(true);
        menus->PushMenu(Menu_DeathRevive::k_menuName);
    } else {
        level->OnMinionNoRevive();
    }
    m_stateID = 4;
}

GS_EndGame::~GS_EndGame() {}

// @0x4fc168: the revive menu on top restarts its die timer (Menu_DeathRevive::ResetDieTime); the result screen refreshes its costumes
void GS_EndGame::ResumeState() {
    Menu_Base *menu = MenuMgr::GetInstance()->GetCurrentPlayingMenu();
    if (!menu) return;
    if (menu->GetMenuName().Equals(Menu_DeathRevive::k_menuName)) {
        static_cast<Menu_DeathRevive*>(menu)->ResetDieTime();
        return;
    }
    if (menu->GetMenuName().Equals(Menu_ResultEndScreen::k_menuName)) static_cast<Menu_ResultEndScreen*>(menu)->RefreshCostumes();
}

// @0x4fc1f8: empty in the .so
void GS_EndGame::SuspendState() {}

// @0x4fc1fc: empty in the .so
void GS_EndGame::UpdateState() {}

// @0x4fc200: the level is drawn under the menus
void GS_EndGame::RenderState() { GameLevel::GetInstance()->Render(); }

// source/game/interfaces/PopupMgr.cpp
//
// Not ported: TrackingScreenMgr::Update after a refresh, and
// SetRTClearColor (the render target's clear colour, render state).
#include "PopupMgr.h"
#include "menus/Menu_Goals.h"
#include "sound/soundmgr.h"
#include "menus/MenuMgr.h"
#include "menus/Menu_Base.h"
#include "popups/BasePopup.h"
#include "popups/PopupInfo.h"
#include "game/gamemanagers/PainterMgr.h"
#include "game/gamestates/GameState.h"
#include "game/input/TouchMgr.h"

// @0x4a9984
PopupMgr::PopupMgr() : m_updating(false), m_justPushed(true) {}

PopupMgr::~PopupMgr() {
    m_updating = false;
    _ClearDelayedDestructionArray();
    for (size_t i = 0; i < m_popupPool.size(); i++) delete m_popupPool[i];
}

// @0x4a9aa4: the one on top is updated; the one the pool just got makes its pop-up sound
void PopupMgr::Update(int dt) {
    if (m_popupPool.empty()) return;
    MenuMgr::GetInstance()->ACTION_Reset();
    if (m_justPushed) {
        m_justPushed = false;
        // the click when Menu_Goals::checkSoundClickOpen was set (and it is cleared), else the pop-up sound
        if (Menu_Goals::checkSoundClickOpen) {
            Menu_Goals::checkSoundClickOpen = false;
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
        } else {
            SoundMgr::GetInstance()->Play(BasePopup::k_sfx_menu_message_window_pop_up, 0);
        }
    }
    m_updating = true;
    m_popupPool.back()->Update(dt);
    m_updating = false;
    _ClearDelayedDestructionArray();
}

// @0x4a9c00
void PopupMgr::PushPopup(BasePopup *p) {
    m_popupPool.push_back(p);
    RefreshPopup();
    m_justPushed = true;
}

// @0x4a9c30
void PopupMgr::PushInfoPopup(const std::string &text) { PushPopup(new PopupInfo(PopupInfo::Info, text)); }

// @0x4a9ce4: the last popup goes away, the menu under it is refreshed; the popup is deleted when the update is done
void PopupMgr::PopPopup(bool) {
    if (m_popupPool.size() <= 1) {
        if (Menu_Base *m = MenuMgr::GetInstance()->GetCurrentPlayingMenu()) {
            m->OnRefreshAfterPopup();
            m->Refresh();
        }
    }
    if (!m_popupPool.empty()) {
        m_justPushed = true;
        m_delayedDestructionArray.push_back(m_popupPool.back());
        m_popupPool.pop_back();
    }
    RefreshPopup();
}

// @0x4a9d8c
bool PopupMgr::IsCurrentPopup(BasePopup *p) const { return !m_popupPool.empty() && m_popupPool.back() == p; }

// @0x4a9db4: what is touched is forgotten, the popup on top refreshes
void PopupMgr::RefreshPopup() {
    if (!m_popupPool.empty()) {
        TouchMgr::GetInstance()->Invalidate();
        m_popupPool.back()->RefreshPopup();
    }
    // TrackingScreenMgr::Update (tracking is not ported)
}

// @0x4a9e20: every popup of the stack, on top of the state
void PopupMgr::Render() {
    if (m_popupPool.empty()) return;
    MenuMgr *mm = MenuMgr::GetInstance();
    mm->SetCameraDataForParticles(*mm->GetCamera());
    jet::scene::Camera *cam = mm->GetCamera();
    jet::video::Painter &painter = PainterMgr::GetInstance()->Get2DPainter();
    for (size_t i = 0; i < m_popupPool.size(); i++) m_popupPool[i]->Render(*cam, painter);
}

// @0x4a9ed4: not while a popup is being updated
void PopupMgr::_ClearDelayedDestructionArray() {
    if (m_updating) return;
    while (!m_delayedDestructionArray.empty()) {
        BasePopup *p = m_delayedDestructionArray.front();
        if (p) delete p;
        m_delayedDestructionArray.erase(m_delayedDestructionArray.begin());
    }
}

// @0x4a9f30: the popup on top counts when its state and menu are the current ones, and (in the action phase) when it may be
// shown while the game runs
bool PopupMgr::IsEnabled(bool inActionPhase) const {
    if (m_popupPool.empty()) return false;
    BasePopup *p = m_popupPool.back();
    if (GameState *s = p->GetGameStateDependency())
        if (s != GameState::GetCrtState()) return false;
    if (MenuMgr::GetInstance()) {
        if (Menu_Base *m = p->GetMenuDependency())
            if (m != MenuMgr::GetInstance()->GetCurrentPlayingMenu()) return false;
    }
    return inActionPhase ? p->CanShowInActionPhase() : true;
}

// @0x4a9ff8
void PopupMgr::RemovePopupsDependingOnGameState(GameState *s) {
    if (!s || m_popupPool.empty()) return;
    const bool justPushed = m_justPushed;
    while (!m_popupPool.empty() && m_popupPool.back()->GetGameStateDependency() == s) PopPopup(true);
    for (size_t i = 0; i < m_popupPool.size();) {
        if (m_popupPool[i]->GetGameStateDependency() == s) {
            m_delayedDestructionArray.push_back(m_popupPool[i]);
            m_popupPool.erase(m_popupPool.begin() + (long)i);
        } else
            i++;
    }
    m_justPushed = justPushed;
    _ClearDelayedDestructionArray();
}

// @0x4aa0dc
void PopupMgr::RemovePopupsDependingOnMenu(Menu_Base *menu) {
    if (!menu || m_popupPool.empty()) return;
    const bool justPushed = m_justPushed;
    while (!m_popupPool.empty() && m_popupPool.back()->GetMenuDependency() == menu) PopPopup(true);
    for (size_t i = 0; i < m_popupPool.size();) {
        if (m_popupPool[i]->GetMenuDependency() == menu) {
            m_delayedDestructionArray.push_back(m_popupPool[i]);
            m_popupPool.erase(m_popupPool.begin() + (long)i);
        } else
            i++;
    }
    m_justPushed = justPushed;
    _ClearDelayedDestructionArray();
}

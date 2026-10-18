// source/game/interfaces/popups/BasePopup.cpp
#include "BasePopup.h"

const jet::String BasePopup::k_sfx_menu_message_window_pop_up("ev_sfx_menu_message_window_pop_up");

// @0x4aa2b8
BasePopup::BasePopup(const jet::String &layerName)
    : BasicPage(layerName), m_refreshOnNextFrame(false), m_canShowInActionPhase(true), m_gameStateDependency(0), m_menuDependency(0) {}

// @0x4aa304
BasePopup::~BasePopup() {}

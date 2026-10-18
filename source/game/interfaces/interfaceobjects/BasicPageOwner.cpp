// source/game/interfaces/interfaceobjects/BasicPageOwner.cpp
#include "BasicPageOwner.h"
#include "BasicPage.h"
#include "game/interfaces/menus/MenuMgr.h"

// @0x447b98
BasicPageOwner::~BasicPageOwner() { delete m_bgPage; }

// @0x447bfc: the page of the layer, the one before gone
void BasicPageOwner::InitFromGUILevel(const jet::String &layer) {
    delete m_bgPage;
    m_bgPage = new BasicPage(layer);
}

// @0x447c48
void BasicPageOwner::UpdateBasicPage(int dt) {
    MenuMgr::GetInstance()->ACTION_Reset();
    m_bgPage->Update(dt);
}

// @0x447c8c
void BasicPageOwner::RenderBasicPage(const jet::scene::Camera &cam, jet::video::Painter &painter) { m_bgPage->Render(cam, painter); }

InterfaceObject *BasicPageOwner::GetUIObject(const jet::String &n) { return m_bgPage->GetUIObject(n); }
InterfaceButton *BasicPageOwner::GetUIButton(const jet::String &n) { return m_bgPage->GetUIButton(n); }
InterfaceText *BasicPageOwner::GetUIText(const jet::String &n) { return m_bgPage->GetUIText(n); }
InterfacePBar *BasicPageOwner::GetUIPBar(const jet::String &n) { return m_bgPage->GetUIPBar(n); }
InterfaceGraph *BasicPageOwner::GetUIGraph(const jet::String &n) { return m_bgPage->GetUIGraph(n); }
int BasicPageOwner::GetNumReleasedButtons() { return MenuMgr::GetInstance()->ACTION_GetNumReleasedButtons(); }
InterfaceButton *BasicPageOwner::GetReleasedButton(int i) { return MenuMgr::GetInstance()->ACTION_GetReleasedButton(i); }

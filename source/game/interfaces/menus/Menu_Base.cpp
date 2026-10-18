// source/game/interfaces/menus/Menu_Base.cpp
#include "Menu_Base.h"
#include "MenuMgr.h"
#include "game/input/TouchMgr.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"

// the initialisers @0x3bebb8, @0x3bebd8 and @0x3bec00
const jet::String Menu_Base::k_sfx_menu_click("ev_sfx_menu_click");
const jet::String Menu_Base::k_sfx_menu_back("ev_sfx_menu_back");
const jet::String Menu_Base::k_sfx_buy("ev_sfx_buy");

Menu_Base::Menu_Base(const jet::String &guiLevelName)
    : m_state(-1), m_guiLevelName(guiLevelName), m_basicPage(0), m_refreshOnNextFrame(false), m_refreshAfterPopup(false), m_introTime(0), m_outroTime(0) {
    for (int i = 0; i < 3; i++)
        for (int k = 0; k < 22; k++) m_menu3DConfig.layer[i].v[k] = 0;
}

Menu_Base::~Menu_Base() { delete m_basicPage; }

// Init @0x454b08: the page of the Aurora layer and the 3D layer defaults
void Menu_Base::Init() {
    delete m_basicPage;
    m_basicPage = new BasicPage(m_guiLevelName);
    const float pi = 3.1415927f, hp = 1.5707964f;
    const float z[3] = { -0.03f, -0.01f, -0.005f };
    for (int i = 0; i < 3; i++) {
        float *v = m_menu3DConfig.layer[i].v;
        for (int k = 0; k < 22; k++) v[k] = 0;
        v[2] = hp; v[4] = -0.01f; v[8] = pi; v[10] = z[i]; v[14] = -hp; v[16] = 0.01f; v[18] = 0.1f; v[19] = 0.1f; v[20] = 7; v[21] = 8;
    }
    m_introTime = 500;
    m_outroTime = 200;
}

void Menu_Base::Update(int dt) {
    MenuMgr::GetInstance()->ACTION_Reset();
    m_basicPage->Update(dt);
}

void Menu_Base::Render(const jet::scene::Camera &cam, jet::video::Painter &p) { m_basicPage->Render(cam, p); }

// SetState @0x454d10: 0 enters the menu, 2 leaves it
void Menu_Base::SetState(int s) {
    if (s == 0) {
        TouchMgr::GetInstance()->Invalidate();
        BasicPage_Reset();
        OnEnter();
    } else if (s == 2) {
        BasicPage_OnMenuQuit();
        OnQuit();
    }
}

void Menu_Base::SetOffset(const math::vec2<float> &o) { m_basicPage->SetOffset(o); }
InterfaceObject *Menu_Base::GetUIObject(const jet::String &n) { return m_basicPage->GetUIObject(n); }
int Menu_Base::GetNumReleasedButtons() { return MenuMgr::GetInstance()->ACTION_GetNumReleasedButtons(); }
InterfaceButton *Menu_Base::GetReleasedButton(int i) { return MenuMgr::GetInstance()->ACTION_GetReleasedButton(i); }
void Menu_Base::BasicPage_Reset() { m_basicPage->Reset(); }
void Menu_Base::BasicPage_OnMenuQuit() { m_basicPage->OnMenuQuit(); }

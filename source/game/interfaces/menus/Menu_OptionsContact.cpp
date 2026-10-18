// source/game/interfaces/menus/Menu_OptionsContact.cpp
//
// Not ported: KeyEventManager::ClearKeyEvents. The countries come from GameplayDef's "CountryContactInfoArray" (the .so keeps
// them in the GameplayDef entity, which is not ported: they are read from its clara data as GameplayDef::NotifyParamChanged
// @0x568a40 does).
#include "Menu_Options.h"
#include "MenuMgr.h"
#include "box/ContactBox.h"
#include "entity.h"
#include "param.h"
#include "game/GameLevel.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGrid.h"
#include "game/interfaces/interfaceobjects/InterfaceScrollBar.h"
#include <cstdlib>

const jet::String Menu_OptionsContact::k_menuName("Menu_OptionsContact");

// @0x48c3ec
Menu_OptionsContact::Menu_OptionsContact() : Menu_Base(jet::String("Options_Contact")), m_backButton(0), m_grid(0), m_scrollBar(0) {}

// @0x48c4d8
Menu_OptionsContact::~Menu_OptionsContact() {
    for (size_t i = 0; i < m_boxes.size(); i++) delete m_boxes[i];
}

// @0x48c548
void Menu_OptionsContact::Init() {
    Menu_Base::Init();
    m_backButton = GetUIButton(jet::String("BackButton"));
    m_grid = GetUIGrid(jet::String("ContactInfoGrid"));
    m_scrollBar = static_cast<InterfaceScrollBar*>(GetUIObject(jet::String("InterfaceScrollBar")));
}

// the countries of the gameplay definition (@0x56ad34: the name, the phone numbers and the flag's sprite, its animation as a number)
static std::vector<CountryContactInfo> LoadCountryContactInfos() {
    std::vector<CountryContactInfo> infos;
    clara::DataEntity *def = GameLevel::GetInstance()->GetGameplayDef();
    clara::Param *array = def ? def->FindParamByName(jet::String("CountryContactInfoArray")) : 0;
    if (!array) return infos;
    for (unsigned i = 0; i < array->GetComponentCount(); i++) {
        clara::DataEntity *e = array->GetAsEntity(i);
        if (!e) continue;
        CountryContactInfo info;
        e->GetParam(jet::String("CountryName"), info.name, 0);
        e->GetParam(jet::String("PhoneNumbers"), info.phones, 0);
        if (clara::Param *flag = e->FindParamByName(jet::String("CountryFlag"))) {
            const clara::Param::SpriteValue &s = flag->GetAsSprite(0);
            info.file = s.file;
            info.flag = atoi(s.anim.c_str());
        }
        infos.push_back(info);
    }
    return infos;
}

// @0x48c6f8: the countries two by two, the last one alone
void Menu_OptionsContact::OnEnter() {
    for (size_t i = 0; i < m_boxes.size(); i++) delete m_boxes[i];
    m_boxes.clear();
    const std::vector<CountryContactInfo> infos = LoadCountryContactInfos();
    size_t i = 0;
    for (; i + 1 < infos.size(); i += 2) {
        ContactBox *box = new ContactBox(infos[i], infos[i + 1]);
        box->SetParent(m_grid);
        m_boxes.push_back(box);
    }
    if (i < infos.size()) {
        ContactBox *box = new ContactBox(infos[infos.size() - 1]);
        box->SetParent(m_grid);
        m_boxes.push_back(box);
    }
    m_grid->SetRows((int)m_boxes.size());
}

// @0x48c840
void Menu_OptionsContact::OnQuit() {
    for (size_t i = 0; i < m_boxes.size(); i++) delete m_boxes[i];
    m_boxes.clear();
}

// @0x48c860: the back key leaves; the scroll bar is drawn over the boxes
void Menu_OptionsContact::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    if (GameState::GetCrtState()->IsBackKeyPressed()) {
        // KeyEventManager::ClearKeyEvents
        MenuMgr::GetInstance()->PopMenu();
        return;
    }
    m_scrollBar->SetVisible(false);
    Menu_Base::Render(cam, p);
    m_scrollBar->SetVisible(true);
    m_scrollBar->Render(cam, p);
}

// @0x48c908
void Menu_OptionsContact::Update(int dt) {
    Menu_Base::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    bool leave = GameState::GetCrtState()->IsBackKeyPressed();
    for (int i = 0; !leave && i < mm->ACTION_GetNumReleasedButtons(); i++)
        if (mm->ACTION_GetReleasedButton(i) == m_backButton) leave = true;
    if (leave) {
        // KeyEventManager::ClearKeyEvents
        mm->PopMenu();
    }
}

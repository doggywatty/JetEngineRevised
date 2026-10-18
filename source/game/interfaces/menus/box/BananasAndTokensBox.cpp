// source/game/interfaces/menus/box/BananasAndTokensBox.cpp
#include "BananasAndTokensBox.h"
#include "../MenuMgr.h"
#include "game/Player.h"
#include "game/gamestates/GS_Shop.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"

// the number with its thousands apart
static jet::String Measure(int v) {
    std::string d = std::to_string(v < 0 ? -v : v), out;
    for (size_t i = 0; i < d.size(); i++) {
        if (i && (d.size() - i) % 3 == 0) out += ',';
        out += d[i];
    }
    return jet::String(((v < 0 ? "-" : "") + out).c_str());
}

// @0x4ce500
BananasAndTokensBox::BananasAndTokensBox(InterfaceObject *parent, bool touchable) : BasicPage(jet::String("BananasAndTokens")), m_touchable(touchable) {
    Init();
    if (parent) {
        SetPos(parent->GetPos());
        SetParent(parent);
    }
}

// @0x4ce624
void BananasAndTokensBox::Init() {
    SetOffset(GetUIObject(jet::String("Counter"))->GetPos() * -1.0f);
    m_textTokensValue = GetUIText(jet::String("Tokens_Value"));
    m_textBananasValue = GetUIText(jet::String("Bananas_Value"));
    m_bananasButton = new InterfaceButton();
    m_bananasButton->InitFrom(GetUIObject(jet::String("Bananas_TouchArea")));
    m_bananasButton->SetActive(m_touchable);
    m_bananasAdd = GetUIButton(jet::String("Bananas_Add"));
    m_bananasAdd->SetActive(m_touchable);
    m_tokensButton = new InterfaceButton();
    m_tokensButton->InitFrom(GetUIObject(jet::String("Tokens_TouchArea")));
    m_tokensButton->SetActive(m_touchable);
    m_tokensAdd = GetUIButton(jet::String("Tokens_Add"));
    m_tokensAdd->SetActive(m_touchable);
    // @0x4cea08: this build's layer has no tokens counter; the tokens row and both touch areas / add buttons are hidden
    m_tokensButton->SetVisible(false);
    m_tokensAdd->SetVisible(false);
    m_textTokensValue->SetVisible(false);
    m_bananasButton->SetVisible(false);
    m_bananasAdd->SetVisible(false);
}

BananasAndTokensBox::~BananasAndTokensBox() {
    delete m_bananasButton;
    delete m_tokensButton;
}

// @0x4ceba4
void BananasAndTokensBox::Update(int dt) {
    BasicPage::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    for (int i = 0; i < mm->ACTION_GetNumReleasedButtons(); i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b != m_bananasButton && b != m_tokensButton && b != m_bananasAdd && b != m_tokensAdd) continue;
        mm->DisableTouchesFor(200);
        GS_Shop::GoToTab(b == m_bananasButton || b == m_bananasAdd ? Menu_Shop::Shop_Bananas : Menu_Shop::Shop_Tokens);
        break;
    }
}

void BananasAndTokensBox::Render(const jet::scene::Camera &c, jet::video::Painter &p) {
    RefreshValues();
    BasicPage::Render(c, p);
}

// @0x4ced38
void BananasAndTokensBox::RefreshValues() {
    m_textTokensValue->SetText(Measure(Player::GetInstance()->GetTokens()));
    m_textBananasValue->SetText(Measure(Player::GetInstance()->GetBananas()));
}

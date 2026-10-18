// source/game/interfaces/menus/box/ContactBox.cpp
#include "ContactBox.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"

// @0x4d02e0
ContactBox::ContactBox() : BasicPage(jet::String("Options_Contact_box")) { Init(0, 0); }
// @0x4d0358
ContactBox::ContactBox(const CountryContactInfo &a, const CountryContactInfo &b) : BasicPage(jet::String("Options_Contact_box")) { Init(&a, &b); }
// @0x4d03d8
ContactBox::ContactBox(const CountryContactInfo &a) : BasicPage(jet::String("Options_Contact_box")) { Init(&a, 0); }
ContactBox::~ContactBox() {}

// @0x4d043c: the box is as large as its "Size"; the first country on the left, the second on the right (or the text alone
// when there are none); each text is the name of the country, a line and its phone numbers
void ContactBox::Init(const CountryContactInfo *a, const CountryContactInfo *b) {
    SetSize(GetUIObject(jet::String("Size"))->GetSize());
    m_textOnlyParent = GetUIObject(jet::String("TextOnly_Parent"));
    m_twoCountriesParent = GetUIObject(jet::String("TwoCountries_Parent"));
    m_firstText = GetUIText(jet::String("TwoCountriesFirstContactText"));
    m_secondText = GetUIText(jet::String("TwoCountriesSecondContactText"));
    m_firstFlag = GetUIGraph(jet::String("TwoCountriesFirstFlagGraph"));
    m_secondFlag = GetUIGraph(jet::String("TwoCountriesSecondFlagGraph"));
    const bool none = !a && !b;
    m_textOnlyParent->SetVisible(none);
    m_twoCountriesParent->SetVisible(!none);
    m_firstText->SetVisible(a != 0);
    m_firstFlag->SetVisible(a != 0);
    babel::StringMgr *sm = babel::Babel::GetInstance()->GetStringMgr();
    if (a) {
        m_firstText->SetText(sm->Get(a->name) + "\n" + sm->Get(a->phones));
        m_firstFlag->SetFrame(a->flag);
    }
    m_secondText->SetVisible(b != 0);
    m_secondFlag->SetVisible(b != 0);
    if (b) {
        m_secondText->SetText(sm->Get(b->name) + "\n" + sm->Get(b->phones));
        m_secondFlag->SetFrame(b->flag);
    }
}

// source/game/interfaces/menus/box/ContactBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_CONTACTBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_CONTACTBOX_H

#include "game/interfaces/interfaceobjects/BasicPage.h"

// The contact of one country of the gameplay definition (GameplayDef's "CountryContactInfoArray", 0x10 bytes in the .so)
struct CountryContactInfo {
    jet::String name;      // 0x0: the key of the name of the country
    jet::String file;      // 0x4: the sprite of the flag
    int flag;              // 0x8: the frame of the flag
    jet::String phones;    // 0xC: the key of the phone numbers
    CountryContactInfo() : flag(0) {}
};

// The contact information of one or two countries side by side ("Options_Contact_box" layer): the flag and the text of
// each, or only a text when there are none
class ContactBox : public BasicPage {  // sizeof 0x1d4
public:
    ContactBox();
    ContactBox(const CountryContactInfo &a, const CountryContactInfo &b);
    explicit ContactBox(const CountryContactInfo &a);
    virtual ~ContactBox();
private:
    void Init(const CountryContactInfo *a, const CountryContactInfo *b);  // @0x4d043c
    InterfaceObject *m_textOnlyParent;     // 0x1bc
    InterfaceObject *m_twoCountriesParent; // 0x1c0
    InterfaceText *m_firstText;            // 0x1c4
    InterfaceText *m_secondText;           // 0x1c8
    InterfaceGraph *m_firstFlag;           // 0x1cc
    InterfaceGraph *m_secondFlag;          // 0x1d0
};

#endif

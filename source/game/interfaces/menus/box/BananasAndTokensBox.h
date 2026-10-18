// source/game/interfaces/menus/box/BananasAndTokensBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_BANANASANDTOKENSBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_BANANASANDTOKENSBOX_H

#include "game/interfaces/interfaceobjects/BasicPage.h"

class InterfaceText;
class InterfaceButton;

// The counters of bananas and tokens ("BananasAndTokens" layer); touching one opens that tab of the shop
class BananasAndTokensBox : public BasicPage {
public:
    BananasAndTokensBox(InterfaceObject *parent, bool touchable);
    virtual ~BananasAndTokensBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void RefreshValues();
private:
    void Init();
    InterfaceText *m_textTokensValue, *m_textBananasValue;
    InterfaceButton *m_bananasButton, *m_tokensButton, *m_bananasAdd, *m_tokensAdd;
    bool m_touchable;
};

#endif

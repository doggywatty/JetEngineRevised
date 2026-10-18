// source/game/interfaces/menus/Menu_SendChallengeType.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_SENDCHALLENGETYPE_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_SENDCHALLENGETYPE_H

// See Menu_Shop.h for why these additions were needed.
#include "Menu_Base.h"

class Menu_SendChallengeType : public Menu_Base {  // sizeof 0x13C
public:
    virtual const jet::String &GetMenuName() const;
    Menu_SendChallengeType();
    virtual ~Menu_SendChallengeType();
    virtual bool UpdateGameInBG();
    virtual bool RenderGameInBG();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnEnter();
    virtual void OnQuit();
private:
    bool CheckTauntButtonPress(InterfaceButton*);
public:
    static const jet::String k_menuName;
private:
    InterfaceText *m_selectText;  // 0x124
    InterfaceButton *m_scoreButton;  // 0x128
    InterfaceButton *m_distanceButton;  // 0x12C
    InterfaceButton *m_bananasButton;  // 0x130
    InterfaceButton *m_backButton;  // 0x134
    InterfaceObject *m_tauntObj;  // 0x138
};

#endif

// source/game/interfaces/menus/Menu_ChallengeResult.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_CHALLENGERESULT_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_CHALLENGERESULT_H

// See Menu_Shop.h for why these additions were needed.
#include "Menu_Base.h"
#include "game/online/tracking/tracking.h"

class Menu_ChallengeResult : public Menu_Base {  // sizeof 0x134
public:
    virtual const jet::String &GetMenuName() const;
    Menu_ChallengeResult();
    virtual bool UpdateGameInBG();
    virtual bool RenderGameInBG();
    virtual void OnEnter();
    virtual void OnQuit();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
private:
    void Exit();
    void SetMenuChallengeWon(bool);
public:
    static const jet::String k_menuName;
private:
    bool m_shouldExit;  // 0x124
    bool m_wasChallengeWon;  // 0x125
    InterfaceText *m_challengerText;  // 0x128
    InterfaceArea *m_challengerAvatarArea;  // 0x12C
    InterfaceObject *m_tauntObj;  // 0x130
};

#endif

// source/game/interfaces/menus/Menu_MoviePlayback.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_MOVIEPLAYBACK_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_MOVIEPLAYBACK_H

// See Menu_Shop.h for why these additions were needed.
#include "Menu_Base.h"
#include "game/online/tracking/tracking.h"
class InterfaceArea;

class Menu_MoviePlayback : public Menu_Base {  // sizeof 0x134
public:
    virtual const jet::String &GetMenuName() const;
    Menu_MoviePlayback();
    virtual ~Menu_MoviePlayback();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnEnter();
    virtual void OnQuit();
    virtual void Refresh();
    virtual bool UpdateGameInBG();
    virtual bool RenderGameInBG();
    void EnableSkipButton(bool);
    bool ShouldSkipMovie();
    void ShowSubtitle(const char*);
    void HideSubtitle();
    virtual TRACKING_SCREEN GetCurrentTrackingScreen();
    static const jet::String k_menuName;
private:
    InterfaceArea *m_skipButtonArea;  // 0x124
    InterfaceButton *m_skipButton;  // 0x128
    InterfaceText *m_subtitleText;  // 0x12C
    bool m_skipMovie;  // 0x130
};

#endif

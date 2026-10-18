// source/game/interfaces/menus/Menu_Cheats.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_CHEATS_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_CHEATS_H

class Menu_Cheats : public Menu_Base {  // sizeof 0x12C
public:
    virtual const jet::String &GetMenuName();
    Menu_Cheats();
    virtual ~Menu_Cheats();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnEnter();
    virtual void OnQuit();
    virtual void Refresh();
    void SetNewSeed(int);
    virtual bool RenderGameInBG();
    static const jet::String k_menuName;
private:
    bool m_refreshCheatsEnabled;  // 0x124
    InterfaceList *m_cheatsList;  // 0x128
};

#endif

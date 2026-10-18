// source/game/interfaces/menus/Menu_InitialLanguage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_INITIALLANGUAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_INITIALLANGUAGE_H

class Menu_InitialLanguage : public Menu_Base {  // sizeof 0x160
public:
    virtual const jet::String &GetMenuName();
    Menu_InitialLanguage();
    virtual ~Menu_InitialLanguage();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Refresh();
    virtual void OnEnter();
    virtual bool RenderGameInBG();
    virtual bool UpdateGameInBG();
private:
    bool HandleLangButton(InterfaceButton*);
public:
    static const jet::String k_menuName;
private:
    std::map<babel::Lang,InterfaceButton *,std::less<babel::Lang>,std::allocator<std::pair<babel::Lang const ,InterfaceButton *> > > m_languageButtons;  // 0x124
    std::map<babel::Lang,InterfaceGraph *,std::less<babel::Lang>,std::allocator<std::pair<babel::Lang const ,InterfaceGraph *> > > m_languageDownloadGraphs;  // 0x140
    bool m_goToIntro;  // 0x15C
};

#endif

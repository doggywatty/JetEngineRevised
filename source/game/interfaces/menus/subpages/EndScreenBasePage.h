// source/game/interfaces/menus/subpages/EndScreenBasePage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_ENDSCREENBASEPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_ENDSCREENBASEPAGE_H

class EndScreenBasePage : public BasicPage {  // sizeof 0x1C8
public:
    virtual const jet::String &GetUITemplateName();
    EndScreenBasePage(const jet::String&);
    virtual ~EndScreenBasePage();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnEnter();
    virtual void Refresh();
    virtual void SetInfoTitle(const jet::String&);
    virtual const jet::String &GetInfoTitle();
    virtual bool IsAutoScrollTimerLocked();
    virtual bool ShouldResetChildren();
    static const jet::String k_uiTmplName;
private:
    jet::String m_infoTitle;  // 0x1C4
};

#endif

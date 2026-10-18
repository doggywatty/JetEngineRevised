// source/game/interfaces/menus/Menu_Base.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_BASE_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_BASE_H

#include "core/pstring.h"
#include "smath.h"

class BasicPage;
class InterfaceObject;
class InterfaceButton;
class InterfaceGraph;
class InterfaceText;
class InterfaceArea;
class InterfaceList;
class InterfaceGrid;
class InterfacePBar;
class InterfaceSlide;
class Interface3DTouchInteract;
class Interface3DBodyPart;
class Interface3DCostumePos;
namespace jet::scene { class Camera; }
namespace jet::video { class Painter; }

// The three 3D layers a menu can put on screen: 22 floats each (@0x454b08 gives the defaults); the euler
// rotation is at 6, the position at 9
struct Layer3DConfig {
    float v[22];
    math::vec3<float> Euler() const { return math::vec3<float>(v[6], v[7], v[8]); }
    math::vec3<float> Pos() const { return math::vec3<float>(v[9], v[10], v[11]); }
};
struct Menu3DConfig { Layer3DConfig layer[3]; };

// A screen of GUI elements made from an Aurora layer (a BasicPage), shown on the MenuMgr's stack
class Menu_Base {
public:
    // the events of the soundpack the menus play (@0xd0d02c, @0xd0d030, @0xd0d034)
    static const jet::String k_sfx_menu_click, k_sfx_menu_back, k_sfx_buy;
    virtual const jet::String &GetMenuName() const = 0;
    Menu_Base(const jet::String &guiLevelName);
    virtual ~Menu_Base();
    virtual void Init();
    virtual void Update(int dt);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void OnMouseWheel(int) {}
    virtual void SetState(int);
    virtual void SetOffset(const math::vec2<float>&);
    virtual void Refresh() {}
    virtual void RefreshOnNextFrame() { m_refreshOnNextFrame = true; }    // @0x47f494
    virtual void OnRefreshAfterPopup() { m_refreshAfterPopup = true; }    // @0x47f4a0
    virtual bool UpdateGameInBG() { return false; }
    virtual bool RenderGameInBG() { return false; }
    virtual void OnEnter() {}
    virtual void OnQuit() {}
    virtual void BasicPage_Reset();
    virtual void BasicPage_OnMenuQuit();
    virtual bool ShowBanner() const { return false; }
    virtual int GetCurrentTrackingScreen() const { return 0; }
    virtual int GetPreviousTrackingScreen() const { return 0; }

    int GetIntroTimer() const { return m_introTime; }
    int GetOutroTimer() const { return m_outroTime; }
    BasicPage *GetPage() const { return m_basicPage; }
    Menu3DConfig &GetMenu3DConfig() { return m_menu3DConfig; }
    int GetNumReleasedButtons();
    InterfaceButton *GetReleasedButton(int);
    InterfaceObject *GetUIObject(const jet::String&);
    InterfaceButton *GetUIButton(const jet::String &n) { return (InterfaceButton*)GetUIObject(n); }
    InterfaceGraph *GetUIGraph(const jet::String &n) { return (InterfaceGraph*)GetUIObject(n); }
    InterfaceText *GetUIText(const jet::String &n) { return (InterfaceText*)GetUIObject(n); }
    InterfaceArea *GetUIArea(const jet::String &n) { return (InterfaceArea*)GetUIObject(n); }
    InterfaceList *GetUIList(const jet::String &n) { return (InterfaceList*)GetUIObject(n); }
    InterfaceGrid *GetUIGrid(const jet::String &n) { return (InterfaceGrid*)GetUIObject(n); }
    InterfacePBar *GetUIPBar(const jet::String &n) { return (InterfacePBar*)GetUIObject(n); }
    InterfaceSlide *GetUISlide(const jet::String &n) { return (InterfaceSlide*)GetUIObject(n); }
    Interface3DTouchInteract *GetUI3DTouchInteract(const jet::String &n) { return (Interface3DTouchInteract*)GetUIObject(n); }
    Interface3DBodyPart *GetUI3DBodyPart(const jet::String &n) { return (Interface3DBodyPart*)GetUIObject(n); }
    Interface3DCostumePos *GetUI3DCostumePos(const jet::String &n) { return (Interface3DCostumePos*)GetUIObject(n); }

    int m_state;                  // 0x4
    jet::String m_guiLevelName;   // 0x8
    BasicPage *m_basicPage;       // 0xC
    Menu3DConfig m_menu3DConfig;  // 0x10
    bool m_refreshOnNextFrame;    // 0x120: the menu refreshes itself in its next update
    bool m_refreshAfterPopup;     // 0x121: a popup was closed, what it changed is to be refreshed
    int m_introTime;              // 0x118
    int m_outroTime;              // 0x11C
};

#endif

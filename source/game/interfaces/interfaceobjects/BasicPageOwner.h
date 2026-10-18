// source/game/interfaces/interfaceobjects/BasicPageOwner.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_BASICPAGEOWNER_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_BASICPAGEOWNER_H

#include "core/pstring.h"

class BasicPage;
class InterfaceObject;
class InterfaceButton;
class InterfaceText;
class InterfacePBar;
class InterfaceGraph;
namespace jet::scene { class Camera; }
namespace jet::video { class Painter; }

// Something that shows one Aurora GUI layer as its page (the game states do)
class BasicPageOwner {  // sizeof 0x8
public:
    BasicPageOwner() : m_bgPage(0) {}
    virtual ~BasicPageOwner();
    void InitFromGUILevel(const jet::String&);
    InterfaceObject *GetUIObject(const jet::String&);
    InterfaceButton *GetUIButton(const jet::String&);
    InterfaceText *GetUIText(const jet::String&);
    InterfacePBar *GetUIPBar(const jet::String&);
    InterfaceGraph *GetUIGraph(const jet::String&);
    void UpdateBasicPage(int dt);
    void RenderBasicPage(const jet::scene::Camera&, jet::video::Painter&);
    BasicPage *GetPage() { return m_bgPage; }
    int GetNumReleasedButtons();
    InterfaceButton *GetReleasedButton(int);
protected:
    BasicPage *m_bgPage;  // 0x4
};

#endif

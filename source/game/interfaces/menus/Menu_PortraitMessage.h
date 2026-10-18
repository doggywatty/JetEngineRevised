// source/game/interfaces/menus/Menu_PortraitMessage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_PORTRAITMESSAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_PORTRAITMESSAGE_H

#include <vector>
#include "Menu_Base.h"
#include "../../input/InputMgr.h"

class InterfaceGraph;
class InterfaceText;

// The menu that shows a message beside one of seven portraits (the message's text and the portrait's graph), and the tutorial graph of an
// input. It is pushed on the MenuMgr's stack while the message is up. 0x184 bytes in the .so. Not ported, and left out of this class: the
// title (SetTitle, EnableParticleTitle, UpdateTitle, RenderTitle: the title's particles and its text), the shooting branch of Render (the tutorial graph 6 follows the
// shot of Menu_Ingame::RenderShooting), the Android toast of the back key in Update, and its KeyEventManager::ClearKeyEvents (KeyEventManager
// is an empty file in the tree).
class Menu_PortraitMessage : public Menu_Base {
public:
    // one message: its text (at +0) and the portrait it is said by (at +4), as the .so's PortraitMessageCrl
    struct PortraitMessageCrl {
        InterfaceText *text;
        InterfaceGraph *graph;
    };
    Menu_PortraitMessage();
    virtual ~Menu_PortraitMessage();
    // @0x48ec34: shows the message on the menu (the menu is pushed when it is not the one playing), with its time (ms)
    static void ShowMessage(unsigned, const jet::String&, int);
    // @0x48ecb0: pops the menu when it is the one playing
    static void HideMessage();
    // @0x48ed04: the tutorial graph of the input is shown (the others are hidden); 0 shows none
    static void ShowTutoGraph(InputMgr::TutorialInputId);
    virtual const jet::String &GetMenuName() const { return k_menuName; }  // @0x481b7c
    virtual void Init();
    virtual void Update(int);
    virtual bool UpdateGameInBG() { return true; }    // @0x481b98
    virtual bool RenderGameInBG() { return true; }    // @0x481ba0
    void HidePortraitMessage();
    // @0x48ee24: the tutorial graph of the input is shown at pos (the graph of input 0 is none). self: the menu's own graph and the portraits'
    // texts and graphs move to half their height first (Menu_Ingame's call). The title's offset of the .so is not ported (no title).
    void SelfShowTutoGraph(int inputId, const math::vec2<float> &pos, bool self);
    virtual int GetCurrentTrackingScreen() const { return -1; }  // @0x48f4a4
    static const jet::String k_menuName;
private:
    // @0x48eba0: the background is shown, the portrait and the text of the message id are shown and the others hidden, the text is set and the
    // time (ms) of the message is kept
    void SetUpMessage(unsigned, const jet::String&, int);
    // the body of ShowTutoGraph (@0x48ed04) on this menu; Init calls it with 0 (no tutorial graph)
    void SetTutoInput(int);
    InterfaceGraph *m_bgGraph;                                // 0x14C
    std::vector<PortraitMessageCrl> m_portraitCtrl;           // 0x150: the seven messages (the .so's begin, end, capacity words at 0x150..0x158)
    int m_tutoInputId;                                        // 0x15C: the input whose tutorial graph is shown (0 for none)
    int m_messageTime;                                        // 0x160: ms the message is still up
    InterfaceGraph *m_tutoGraphs[8];                          // 0x164: the tutorial graph of each input (index 0 is none)
};

#endif

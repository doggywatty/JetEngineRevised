// source/game/interfaces/menus/Menu_PortraitMessage.cpp
#include "Menu_PortraitMessage.h"
#include "MenuMgr.h"
#include "../interfaceobjects/InterfaceGraph.h"
#include "../interfaceobjects/InterfaceText.h"
#include "../../gamestates/GameState.h"

const jet::String Menu_PortraitMessage::k_menuName("PortraitMessage");

// the names of the seven messages' texts and portraits (gui::PortraitMessage::_message_i_Text / _portrait_i_graph in the .so)
static const char *const k_messageTextNames[7] = {"message_0_Text", "message_1_Text", "message_2_Text", "message_3_Text",
                                                  "message_4_Text", "message_5_Text", "message_6_Text"};
static const char *const k_portraitGraphNames[7] = {"portrait_0_graph", "portrait_1_graph", "portrait_2_graph", "portrait_3_graph",
                                                    "portrait_4_graph", "portrait_5_graph", "portrait_6_graph"};

// @0x48dc20
Menu_PortraitMessage::Menu_PortraitMessage() : Menu_Base(k_menuName), m_bgGraph(0), m_tutoInputId(0), m_messageTime(0) {
    for (InterfaceGraph *&g : m_tutoGraphs) g = 0;
}

// @0x48dd60
Menu_PortraitMessage::~Menu_PortraitMessage() {}

// @0x48de00: the seven messages (text, then portrait), the background and the tutorial graphs of the inputs 1..7 (the input 0 has none); the
// tutorial graphs are hidden at the end
void Menu_PortraitMessage::Init() {
    Menu_Base::Init();
    for (int i = 0; i < 7; i++) {
        InterfaceText *text = GetUIText(jet::String(k_messageTextNames[i]));
        InterfaceGraph *graph = GetUIGraph(jet::String(k_portraitGraphNames[i]));
        m_portraitCtrl.push_back(PortraitMessageCrl{text, graph});
    }
    // the tutorial graph of each input, in the .so's order of lookup (TutoDodgeL, TutoDodgeR, TutoDodgeRL, TutoSlide, TutoJump,
    // TutoTouchScreen, TutoTilt), stored at the input's index
    m_tutoGraphs[0] = 0;
    m_tutoGraphs[2] = GetUIGraph(jet::String("TutoDodgeL_Graph"));
    m_tutoGraphs[1] = GetUIGraph(jet::String("TutoDodgeR_Graph"));
    m_tutoGraphs[3] = GetUIGraph(jet::String("TutoDodgeRL_Graph"));
    m_tutoGraphs[5] = GetUIGraph(jet::String("TutoSlide_Graph"));
    m_tutoGraphs[4] = GetUIGraph(jet::String("TutoJump_Graph"));
    m_tutoGraphs[6] = GetUIGraph(jet::String("TutoTouchScreen_Graph"));
    m_tutoGraphs[7] = GetUIGraph(jet::String("TutoTilt_Graph"));
    m_bgGraph = GetUIGraph(jet::String("bg_graph"));
    SetTutoInput(0);
}

// @0x48ea08: the time of the message runs down while it is up; when it is over, the tutorial's graph (if any) is hidden with the message,
// else the menu is popped. The back key keeps the message up (the .so also clears the key events, KeyEventManager is not in the tree).
void Menu_PortraitMessage::Update(int dt) {
    Menu_Base::Update(dt);
    if (GameState::GetCrtState()->IsBackKeyPressed()) return;
    if (m_messageTime == 0) return;
    m_messageTime -= dt;
    if (m_messageTime > 0) return;
    if (m_tutoInputId == 0)
        HideMessage();
    else
        HidePortraitMessage();
}

// @0x48eb34: the background, the portraits and the texts are hidden (the portrait before its text)
void Menu_PortraitMessage::HidePortraitMessage() {
    m_bgGraph->SetVisible(false);
    for (PortraitMessageCrl &c : m_portraitCtrl) {
        c.graph->SetVisible(false);
        c.text->SetVisible(false);
    }
}

// @0x48eba0: the background is shown; the message id's portrait and text are shown and the others hidden; the text is set and the time kept
void Menu_PortraitMessage::SetUpMessage(unsigned id, const jet::String &text, int time) {
    m_bgGraph->SetVisible(true);
    for (unsigned i = 0; i < m_portraitCtrl.size(); i++) {
        const bool shown = i == id;
        m_portraitCtrl[i].graph->SetVisible(shown);
        m_portraitCtrl[i].text->SetVisible(shown);
    }
    m_portraitCtrl[id].text->SetText(text);
    m_messageTime = time;
}

// @0x48ec34: the menu is pushed when it is not the one playing, then the message is set up on it
void Menu_PortraitMessage::ShowMessage(unsigned id, const jet::String &text, int time) {
    MenuMgr *mm = MenuMgr::GetInstance();
    Menu_PortraitMessage *menu = static_cast<Menu_PortraitMessage*>(mm->GetMenu(k_menuName));
    if (menu != mm->GetCurrentPlayingMenu()) mm->PushMenu(k_menuName);
    menu->SetUpMessage(id, text, time);
}

// @0x48ecb0: the menu is popped when it is the one playing
void Menu_PortraitMessage::HideMessage() {
    MenuMgr *mm = MenuMgr::GetInstance();
    if (mm->GetMenu(k_menuName) == mm->GetCurrentPlayingMenu()) mm->PopMenu();
}

// @0x48ed04: the tutorial graph of the input is shown (the others hidden) on the menu, when there is one
void Menu_PortraitMessage::ShowTutoGraph(InputMgr::TutorialInputId id) {
    Menu_PortraitMessage *menu = static_cast<Menu_PortraitMessage*>(MenuMgr::GetInstance()->GetMenu(k_menuName));
    if (menu) menu->SetTutoInput(id.id);
}

// @0x48ee24
void Menu_PortraitMessage::SelfShowTutoGraph(int inputId, const math::vec2<float> &pos, bool self) {
    if (self) {
        // the .so's title offset (the title object at +0x124) is not ported, so its "+30 / +10" is never added here
        const math::vec2<float> bg = m_bgGraph->GetPos();
        m_bgGraph->SetPos(math::vec2<float>(bg.x, bg.y * 0.5f));
        for (PortraitMessageCrl &c : m_portraitCtrl) {
            if (c.text->IsVisible()) {
                const math::vec2<float> t = c.text->GetPos();
                c.text->SetPos(math::vec2<float>(t.x, t.y * 0.5f));
            }
            if (c.graph->IsVisible()) {
                const math::vec2<float> g = c.graph->GetPos();
                c.graph->SetPos(math::vec2<float>(g.x, g.y * 0.5f));
            }
        }
    }
    SetTutoInput(inputId);
    m_tutoGraphs[inputId]->SetPos(pos);
}

// the body of ShowTutoGraph (@0x48ed04), on this menu: the input is kept, and each tutorial graph is shown when it is the input's and restarted
void Menu_PortraitMessage::SetTutoInput(int id) {
    m_tutoInputId = id;
    for (int i = 0; i < 8; i++) {
        if (InterfaceGraph *g = m_tutoGraphs[i]) {
            g->SetVisible(i == id);
            g->ResetAnim();
        }
    }
}

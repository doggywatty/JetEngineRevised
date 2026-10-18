// source/game/interfaces/PopupMgr.h
#ifndef SOURCE_GAME_INTERFACES_POPUPMGR_H
#define SOURCE_GAME_INTERFACES_POPUPMGR_H

#include <string>
#include <vector>
#include "jsingleton.h"

class BasePopup;
class GameState;
class Menu_Base;

// The stack of the popups: the one on top is updated (while it is shown the state and the menus stand still), all of them
// are drawn. A popup that is closed is deleted at the end of the update that closed it.
class PopupMgr : public Singleton<PopupMgr> {  // sizeof 0x2C
public:
    PopupMgr();
    virtual ~PopupMgr();
    void Init() {}
    void PushPopup(BasePopup*);
    void PopPopup(bool);
    void PushInfoPopup(const std::string&);
    void Update(int);
    void Render();
    bool IsEnabled(bool inActionPhase) const;
    bool IsCurrentPopup(BasePopup*) const;
    BasePopup *GetCurrentPopup() const { return m_popupPool.empty() ? 0 : m_popupPool.back(); }
    void RefreshPopup();
    void RemovePopupsDependingOnGameState(GameState*);
    void RemovePopupsDependingOnMenu(Menu_Base*);
private:
    void _ClearDelayedDestructionArray();
    bool m_updating;   // 0x4
    bool m_justPushed; // 0x5
    std::vector<BasePopup*> m_popupPool;               // 0x8
    std::vector<BasePopup*> m_delayedDestructionArray; // 0x14
};

#endif

// source/game/gamestates/GameState.h
#ifndef SOURCE_GAME_GAMESTATES_GAMESTATE_H
#define SOURCE_GAME_GAMESTATES_GAMESTATE_H

#include "core/pstring.h"

// A screen of the game. The one on top of the stack is updated and rendered; a ring of ten keeps
// the ones to go back to
class GameState {  // sizeof 0xC
public:
    virtual const jet::String &GetStateName() = 0;
    GameState() : m_stateID(0), m_backKeyPressed(false), m_menuKeyPressed(false) {}
    virtual ~GameState();
    virtual void ResumeState() = 0;
    virtual void SuspendState() = 0;
    virtual void UpdateState() = 0;
    virtual void RenderState() = 0;
    // the time the game advances by for this frame
    virtual int GetUpdateTimeDelta(int dt) { return dt; }
    static void InitStatesStack();
    static void ReleaseStatesStack();
    static void PopAllPrevStates() { s_resetStack = true; }
    static void PushState(GameState*, bool keepPrev);
    static void SetState(GameState*);
    static void PopState();
    static void UpdateCurrentState(bool update);
    static bool RenderCurrentState();
    static GameState *GetCrtState() { return s_currentState; }
    static GameState *GetPrevState(GameState*);
    bool IsBackKeyPressed() const { return m_backKeyPressed; }
    void SetBackKeyPressed(bool v) { m_backKeyPressed = v; }
    bool IsMenuKeyPressed() const { return m_menuKeyPressed; }
    void SetMenuKeyPressed(bool v) { m_menuKeyPressed = v; }
    int GetStateID() const { return m_stateID; }
    static const int k_statesStackLen = 10;
protected:
    int m_stateID;  // 0x4
private:
    bool m_backKeyPressed;  // 0x8
    bool m_menuKeyPressed;  // 0x9
    static int s_statesStackHead;
    static GameState *s_statesStack[k_statesStackLen];
    static GameState *s_currentState;
    static GameState *s_lastUpdatedState;
    static GameState *s_stateToPop;
    static GameState *s_stateToDelete;
    static bool s_resetStack;
};

#endif

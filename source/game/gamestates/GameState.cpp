// source/game/gamestates/GameState.cpp
#include "GameState.h"
#include "game/GameLevel.h"
#include "game/Game.h"
#include "GS_Loading.h"
#include "GS_MainMenu.h"
#include "system.h"
#include "video/display.h"
#include "video/rendertarget.h"

int GameState::s_statesStackHead = 0;
GameState *GameState::s_statesStack[GameState::k_statesStackLen];
GameState *GameState::s_currentState = 0;
GameState *GameState::s_lastUpdatedState = 0;
GameState *GameState::s_stateToPop = 0;
GameState *GameState::s_stateToDelete = 0;
bool GameState::s_resetStack = false;

GameState::~GameState() {}

// @0x48d314
void GameState::InitStatesStack() {
    s_currentState = 0;
    s_statesStackHead = 0;
    s_resetStack = false;
    for (int i = 0; i < k_statesStackLen; i++) s_statesStack[i] = 0;
}

// @0x48d378
void GameState::ReleaseStatesStack() {
    delete s_currentState;
    s_currentState = 0;
    delete s_stateToPop;
    s_stateToPop = 0;
    delete s_stateToDelete;
    s_stateToDelete = 0;
    for (int i = 0; i < k_statesStackLen; i++) {
        delete s_statesStack[i];
        s_statesStack[i] = 0;
    }
}

// @0x48d47c: the state on top is suspended and, when kept, goes to the ring to come back to
void GameState::PushState(GameState *s, bool keepPrev) {
    // @0x4fba20: the loading state that comes over the main menu ends its sounds and its music
    if (s_currentState && s->GetStateName().Equals(GS_Loading::k_stateName) && s_currentState->GetStateName().Equals(GS_MainMenu::k_stateName)) {
        static_cast<GS_MainMenu*>(s_currentState)->StopMainMenuSfx();
        Game::GetInstance()->StopMenuMusic();
    }
    if (s_currentState) {
        if (keepPrev) {
            delete s_statesStack[s_statesStackHead];
            s_statesStack[s_statesStackHead] = s_currentState;
            if (++s_statesStackHead >= k_statesStackLen) s_statesStackHead = 0;
        } else if (!s_stateToDelete)
            s_stateToDelete = s_currentState;
        s_currentState->SuspendState();
    }
    s_currentState = s;
    s->ResumeState();
}

// @0x48d5c8
void GameState::SetState(GameState *s) {
    PushState(s, true);
    s_resetStack = true;
}

// @0x48d5f8: back to the previous state; the one left is deleted once the frame is over
void GameState::PopState() {
    int prev = s_statesStackHead - 1;
    if (prev < 0) prev = k_statesStackLen - 1;
    if (!s_statesStack[prev] || s_stateToPop) return;
    s_currentState->SuspendState();
    s_stateToPop = s_currentState;
    s_currentState = s_statesStack[prev];
    s_statesStack[prev] = 0;
    s_statesStackHead = prev;
    s_currentState->ResumeState();
}

// @0x48d77c
void GameState::UpdateCurrentState(bool update) {
    GameState *s = s_currentState;
    if (!s) return;
    if (update) s->UpdateState();
    s_lastUpdatedState = s;
    delete s_stateToPop;
    s_stateToPop = 0;
    delete s_stateToDelete;
    s_stateToDelete = 0;
    if (!s_resetStack) return;
    for (int k = 0; k < k_statesStackLen; k++) {
        delete s_statesStack[k];
        s_statesStack[k] = 0;
    }
    s_statesStackHead = 0;
    s_resetStack = false;
}

// @0x48d8b4: renders the state updated this frame and sets the clear colour of the next one
bool GameState::RenderCurrentState() {
    GameState *s = s_currentState;
    if (!s || s != s_lastUpdatedState) return false;
    s->RenderState();
    math::vec4<float> clear(1, 1, 1, 1);
    if (GameLevel::GetInstance()) clear = GameLevel::GetInstance()->GetClearColor();
    jet::System::GetDisplay(0)->GetRenderTarget()->SetClearColor(clear);
    return true;
}

// @0x48da24
GameState *GameState::GetPrevState(GameState *of) {
    int i = s_statesStackHead;
    if (of && of != s_currentState)
        for (i = 0; s_statesStack[i] != of; i++) {}
    if (--i < 0) i = k_statesStackLen - 1;
    return s_statesStack[i];
}

// source/game/gamestates/GS_EndGame.h
#ifndef SOURCE_GAME_GAMESTATES_GS_ENDGAME_H
#define SOURCE_GAME_GAMESTATES_GS_ENDGAME_H

#include "GameState.h"

// The end of a run, pushed by Game::GoToEndScreen (@0x52c5b0) when the minion is dead (GS_Gameplay::CheckGameplayEnd @0x4fc8ec).
// Its constructor offers the revive (Menu_DeathRevive) or, when there is none, goes straight to the result screen
// (GameLevel::OnMinionNoRevive). sizeof 0xC: the GameState base, with the state id 4 (+0x4).
class GS_EndGame : public GameState {
public:
    virtual const jet::String &GetStateName();
    GS_EndGame();
    virtual ~GS_EndGame();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    static const jet::String k_stateName;
};

#endif

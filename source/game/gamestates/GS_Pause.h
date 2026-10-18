// source/game/gamestates/GS_Pause.h
#ifndef SOURCE_GAME_GAMESTATES_GS_PAUSE_H
#define SOURCE_GAME_GAMESTATES_GS_PAUSE_H

#include "GameState.h"

class GS_Pause : public GameState {  // sizeof 0x10
public:
    virtual const jet::String &GetStateName();
    GS_Pause();
    virtual ~GS_Pause();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    static const jet::String k_stateName;
private:
    static const jet::String k_musicLabel;
    int m_music;  // 0xC
};

#endif

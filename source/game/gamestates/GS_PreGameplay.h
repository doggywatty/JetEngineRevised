// source/game/gamestates/GS_PreGameplay.h
#ifndef SOURCE_GAME_GAMESTATES_GS_PREGAMEPLAY_H
#define SOURCE_GAME_GAMESTATES_GS_PREGAMEPLAY_H

#include "GameState.h"
#include "jenum.h"
namespace clara { class Movie; }

class GS_PreGameplay : public GameState {  // sizeof 0x14
public:
    // Verified @0x503c04 (UpdateState): 0 starts the intro movie and moves to 1, 1 waits for it to stop
    // playing and moves to 2, 2 pops this state. See GS_PreGameplay.cpp for the full writeup.
    struct ESubStateDef { enum type { Sub_Init = 0, Sub_PlayingMovie = 1, Sub_Done = 2 }; };
    virtual const jet::String &GetStateName();
    GS_PreGameplay();
    virtual ~GS_PreGameplay();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
private:
    void Movie_StartMovie(clara::Movie*);
    bool Movie_IsPlaying();
    void Movie_Stop();
public:
    static const jet::String k_stateName;
private:
    safe_enum<GS_PreGameplay::ESubStateDef,enum GS_PreGameplay::ESubStateDef::type> m_subState;  // 0xC
    clara::Movie *m_playingMovie;  // 0x10
};

#endif

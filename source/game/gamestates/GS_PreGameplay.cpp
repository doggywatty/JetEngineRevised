// source/game/gamestates/GS_PreGameplay.cpp
//
// The state GS_Loading pushes (verified via armxref: GS_Loading::_UpdateLoadingStep() at 0x500564
// mallocs sizeof(GS_Gameplay)==0x78 bytes and constructs it, then at 0x5005b8 mallocs sizeof(
// GS_PreGameplay)==0x14 and pushes *that* on top) right before real gameplay: plays the level's intro
// movie (if any), then pops itself once the movie finishes so GS_Gameplay underneath takes over.
#include "GS_PreGameplay.h"
#include <cstdio>
#include <cstdlib>
#include "../entities/Minion.h"
#include "GameState.h"
#include "jenum.h"
#include "clara.h"
#include "movie.h"
#include "game/entities/LevelDef.h"
#include "game/GameLevel.h"
#include "game/Game.h"

const jet::String GS_PreGameplay::k_stateName("GS_PreGameplay");

// Verified @0x503af4. The call to LevelDef::OnInitMainMenu() (not a typo -- the real binary's
// GS_PreGameplay constructor really does call that, not a PreGameplay-specific init) is surprising but
// unambiguous in the disassembly: a direct bl to the exact address armdis.py resolves to that symbol,
// not an identical-code-folding collision (LevelDef::OnInitPreGamePlay() is a separate, different
// address). Left as-is rather than "corrected" to what seems more sensible.
GS_PreGameplay::GS_PreGameplay() : m_subState(ESubStateDef::Sub_Init), m_playingMovie(0) {
    LevelDef::GetInstance()->OnInitMainMenu();
}

GS_PreGameplay::~GS_PreGameplay() {}

const jet::String &GS_PreGameplay::GetStateName() { return k_stateName; }

// @0x503bb4: the action phase is off (Game+0x41B, the same flag GS_Gameplay::ResumeState sets on)
void GS_PreGameplay::ResumeState() { Game::GetInstance()->SetIsInActionPhase(false); }
// @0x503bdc: the action phase is off
void GS_PreGameplay::SuspendState() { Game::GetInstance()->SetIsInActionPhase(false); }

// Verified @0x503cb0: tail call to GameLevel::GetInstance()->Render().
void GS_PreGameplay::RenderState() { GameLevel::GetInstance()->Render(); }

// Verified structure @0x503c04: substate 0 starts the level's intro movie (if LevelDef has one for this
// level) and advances to 1; substate 1 waits for it to stop playing then advances to 2; substate 2 pops
// this state so GS_Gameplay (pushed right underneath by GS_Loading) takes over. The real binary also
// calls Minion::ProjectInitialPositionToGround() from substate 0, not reproduced here (no Minion-facing
// equivalent exists in this tree yet), and resolves the movie via LevelDef::GetMovieFromParam() with a
// literal jet::String parameter name this session could not recover from the disassembly (it's loaded
// from a PIC literal pool this session's tools don't resolve back to source text) -- "IntroMovie" below
// is a guess at that name, flagged as such.
void GS_PreGameplay::UpdateState() {
    GameLevel::GetInstance()->Update(false);
    if (m_subState == ESubStateDef::Sub_Init) {
        // @0x503c3c: the minion stands on the ground below where the level put it before the movie of the start
        GameLevel::GetInstance()->GetMinion()->ProjectInitialPositionToGround();
        Movie_StartMovie(LevelDef::GetInstance()->GetMovieFromParam(jet::String("Ref_Movie_NoChekpointInit")));
        m_subState = ESubStateDef::Sub_PlayingMovie;
    }
    if (m_subState == ESubStateDef::Sub_PlayingMovie) {
        if (!Movie_IsPlaying()) m_subState = ESubStateDef::Sub_Done;
    }
    if (m_subState == ESubStateDef::Sub_Done) GameState::PopState();
}

// Adapted, not verified byte-for-byte: the real Movie_StartMovie/IsPlaying/Stop (@0x503cd0/0x503d90/
// 0x503dd0) manipulate a separate clara::Movie::TimeController object (undocumented private offsets
// 0x28-0x2a, 0xc, 0x1c, 0x20) that this codebase's from-scratch clara::Movie (core/claraloader/include/
// movie.h) does not have -- this tree's Movie plays itself via SetPlaying()/IsPlaying() directly with no
// separate controller object, so the real logic doesn't have a 1:1 equivalent to port. Adapted to the
// API that actually exists here instead of guessing at a controller this engine doesn't model.
void GS_PreGameplay::Movie_StartMovie(clara::Movie *m) {
    Movie_Stop();
    m_playingMovie = m;
    if (m) {
        m->SetLooping(false);
        m->SetTime(m->GetStart());
        m->SetPlaying(true);
    }
}

bool GS_PreGameplay::Movie_IsPlaying() { return m_playingMovie && m_playingMovie->IsPlaying(); }

void GS_PreGameplay::Movie_Stop() {
    if (m_playingMovie) m_playingMovie->SetPlaying(false);
    m_playingMovie = 0;
}

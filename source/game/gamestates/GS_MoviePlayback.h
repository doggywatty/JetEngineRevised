// source/game/gamestates/GS_MoviePlayback.h
#ifndef SOURCE_GAME_GAMESTATES_GS_MOVIEPLAYBACK_H
#define SOURCE_GAME_GAMESTATES_GS_MOVIEPLAYBACK_H

class GS_MoviePlayback : public GameState {  // sizeof 0x20
public:
    virtual const jet::String &GetStateName();
    GS_MoviePlayback();
    virtual ~GS_MoviePlayback();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    void SkipMovie();
    static void StartClaraMovie();
private:
    static void OnMovieFinished();
    void UpdateTouches();
    void Orientation_UpdateCamera();
public:
    static const jet::String k_stateName;
private:
    GS_MoviePlayback::MoviePlaybackStates m_currentState;  // 0xC
    bool m_firstLoad;  // 0x10
    safe_enum<jet::video::Display::OrientationDef,enum jet::video::Display::OrientationDef::type> m_preMovieOrientation;  // 0x14
    bool m_skipMovie;  // 0x18
    float m_cachedAspectRatio;  // 0x1C
};

#endif

// source/game/gamestates/GS_NoSpace.h
#ifndef SOURCE_GAME_GAMESTATES_GS_NOSPACE_H
#define SOURCE_GAME_GAMESTATES_GS_NOSPACE_H

class GS_No_Space : public GameState {  // sizeof 0x10
public:
    virtual const jet::String &GetStateName();
    GS_No_Space(bool);
    virtual ~GS_No_Space();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    void CheckSpace();
    static const jet::String k_stateName;
private:
    bool m_goToLanguageScreen;  // 0xC
    bool m_hasSpace;  // 0xD
};

#endif

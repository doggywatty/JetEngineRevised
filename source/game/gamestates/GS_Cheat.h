// source/game/gamestates/GS_Cheat.h
#ifndef SOURCE_GAME_GAMESTATES_GS_CHEAT_H
#define SOURCE_GAME_GAMESTATES_GS_CHEAT_H

class GS_Cheat : public GameState {  // sizeof 0x10
public:
    virtual const jet::String &GetStateName();
    GS_Cheat();
    virtual ~GS_Cheat();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    static const jet::String k_stateName;
private:
    bool m_enableResetRMS;  // 0xC
    bool m_enablecleanGUI;  // 0xD
    bool m_enableGiveMoney;  // 0xE
};

#endif

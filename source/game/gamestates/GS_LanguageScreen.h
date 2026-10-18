// source/game/gamestates/GS_LanguageScreen.h
#ifndef SOURCE_GAME_GAMESTATES_GS_LANGUAGESCREEN_H
#define SOURCE_GAME_GAMESTATES_GS_LANGUAGESCREEN_H

class GS_LanguageScreen : public GameState {  // sizeof 0x10
public:
    virtual const jet::String &GetStateName();
    GS_LanguageScreen();
    virtual ~GS_LanguageScreen();
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    static const jet::String k_stateName;
private:
    int m_languageCheckTimeout;  // 0xC
};

#endif

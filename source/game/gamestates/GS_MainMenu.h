// source/game/gamestates/GS_MainMenu.h
#ifndef SOURCE_GAME_GAMESTATES_GS_MAINMENU_H
#define SOURCE_GAME_GAMESTATES_GS_MAINMENU_H

#include "GameState.h"
#include "smath.h"

class BodyPartOwner;
struct Touch;

// The main menu: the menu page over the lab scene.
// Not ported yet: the 3D touches of the minion
// (they need the body part hit detection) and the statistics reset.
class GS_MainMenu : public GameState {  // sizeof 0x24
public:
    explicit GS_MainMenu(const jet::String &extraMenu);
    virtual ~GS_MainMenu();
    virtual const jet::String &GetStateName() { return k_stateName; }
    virtual void ResumeState();
    virtual void SuspendState();
    virtual void UpdateState();
    virtual void RenderState();
    void StopMainMenuSfx();
    static const jet::String k_stateName;
    static const jet::String k_standardMusicLabel;  // @0xd0df20
    static const jet::String k_standardAmbientLabel;  // @0xd0df24
private:
    void UpdateNormal();
public:
    void StartNormalGamePlay();
    BodyPartOwner *GetTouchedBPOFor(const math::vec2<float>&);
    bool m_wasRenderCalled;  // 0xA
    BodyPartOwner *m_touchedBPO;  // 0xC the owner the last 3D touch was on
    const Touch *m_touch;  // 0x10 the finger on the 3D scene
    int m_music;  // 0x14: the ambience of the menu (a handle of SoundMgr)
    jet::String m_musicLabel;  // 0x18
    jet::String m_ambientLabel;  // 0x1c
    float m_hfov;  // 0x20
};

#endif

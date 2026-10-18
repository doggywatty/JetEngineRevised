// source/game/interfaces/menus/box/MissionsMultiplierBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_MISSIONSMULTIPLIERBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_MISSIONSMULTIPLIERBOX_H

#include "game/interfaces/interfaceobjects/BasicPage.h"

class InterfaceGraph;
class InterfacePBar;
class InterfaceText;

// The multiplier of the score the missions raise: the level now and the next one, and a bar of the points to the next.
// With the flag the box follows the points the player has; without it it is given points to add one by one (the bar grows,
// the number pops up).
// Not ported: the particle systems of the bar and of the text (ps::ParticleSystem) and the debug text of the cheats.
class MissionsMultiplierBox : public BasicPage {  // sizeof 0x1F0
public:
    MissionsMultiplierBox(bool followsSavedPoints);  // @0x4d6380
    virtual ~MissionsMultiplierBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void AddMissionMultiplierPoints(int p) { m_pointsToAdd += p; }  // @0x4d6edc
    bool AnyEffectOn() const { return m_pointsToAdd > 0 || m_multiplierTextEffectTimer > 0; }  // @0x4d6eec
private:
    int GetMultiplierNextLevelPoints();  // @0x4d6c60
    void AddMultiplierPoints(int);  // @0x4d6c88
    void InitMultiplierTextEffect();  // @0x4d6e34
    void UpdateMultiplierTextEffect(int);  // @0x4d6e6c
    void SetMultiplierText(int, float);  // @0x4d6d4c
    InterfacePBar *m_multiplierPBar;  // 0x1BC
    InterfaceGraph *m_barStart;  // 0x1C0
    InterfaceText *m_curMultiplierText;  // 0x1C4
    InterfaceText *m_nextMultiplierText;  // 0x1C8
    bool m_followsSavedPoints;  // 0x1CC
    int m_multiplierLevel;  // 0x1D0
    int m_multiplierLevelPoints;  // 0x1D4
    int m_pointsToAdd;  // 0x1D8
    int m_shownMultiplierLevel;  // 0x1DC
    int m_popState;  // 0x1E0: 1 the bar's start grows, -1 shrinks, 0 rests
    int m_popTimer;  // 0x1E4
    int m_multiplierTextEffectTimer;  // 0x1E8
    int m_pointsAddedCount;  // 0x1EC
};

#endif

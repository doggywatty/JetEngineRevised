// source/game/interfaces/menus/box/PauseMenuChallengeInfoBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_PAUSEMENUCHALLENGEINFOBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_PAUSEMENUCHALLENGEINFOBOX_H

class PauseMenuChallengeInfoBox : public BasicPage {  // sizeof 0x1D0
public:
    PauseMenuChallengeInfoBox();
    virtual ~PauseMenuChallengeInfoBox();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
private:
    void _CheckCostumeReload();
    void _ClearCostumeData();
    jet::String challengeCostumeName;  // 0x1C4
    jet::String animixerStateName;  // 0x1C8
    Interface3DCostumePos *m_i3DCostume;  // 0x1CC
};

#endif

// source/game/interfaces/popups/PopupRedeem.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPREDEEM_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPREDEEM_H

class PopupRedeem : public BasePopup {  // sizeof 0x1F8
public:
    PopupRedeem(bool);
    virtual ~PopupRedeem();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RefreshPopup();
    void SetRewardText(jet::String);
    void Set2DReward(int);
    void Set3DReward(clara::Entity*);
private:
    void LoadText();
    bool m_hasFail;  // 0x1C8
    InterfaceButton *m_okButton;  // 0x1CC
    InterfaceObject *m_failBG;  // 0x1D0
    InterfaceText *m_invalidText;  // 0x1D4
    InterfaceObject *m_prizeBG;  // 0x1D8
    InterfaceText *m_youGot;  // 0x1DC
    InterfaceObject *m_rewardItem;  // 0x1E0
    InterfaceText *m_rewardMsg;  // 0x1E4
    InterfaceObject *m_rewardPosition3D;  // 0x1E8
    InterfaceObject *m_rewardPosition;  // 0x1EC
    InterfaceGraph *m_rewardBananaMultiplier;  // 0x1F0
    InterfaceGraph *m_rewardDespicableMultiplier;  // 0x1F4
};

#endif

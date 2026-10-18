// source/game/interfaces/menus/subpages/EndScreenQuickTipPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_ENDSCREENQUICKTIPPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_ENDSCREENQUICKTIPPAGE_H

class EndScreenQuickTipPage : public EndScreenBasePage {  // sizeof 0x1F0
public:
    virtual const jet::String &GetUITemplateName();
    EndScreenQuickTipPage();
    virtual ~EndScreenQuickTipPage();
    virtual void OnEnter();
    virtual void Update(int);
    virtual void Refresh();
private:
    void RefreshPerkInfo();
    void ComputeBonusEntity();
    void RefreshBonusInfo();
    void ComputeMinionCostume();
    void RefreshMinionCostumeInfo();
    void _Clear3dElement();
    void LoadText();
public:
    static const jet::String k_uiTmplName;
private:
    safe_enum<EndScreenQuickTipPage::EStateDef,enum EndScreenQuickTipPage::EStateDef::type> m_state;  // 0x1C8
    Price *m_upgradePrice;  // 0x1CC
    safe_enum<EndScreenQuickTipPage::EPerkTypeDef,enum EndScreenQuickTipPage::EPerkTypeDef::type> m_perkType;  // 0x1D0
    InterfaceButton *m_perkBuyButton;  // 0x1D4
    Bonus *m_bonusEntity;  // 0x1D8
    InterfaceButton *m_bonusBuyButton;  // 0x1DC
    InterfacePBar *m_bonusLevelBar;  // 0x1E0
    MinionCostume *m_minionCostume;  // 0x1E4
    InterfaceButton *m_minionCostumeBuyButton;  // 0x1E8
    InterfaceObject *m_3dElement;  // 0x1EC
};

#endif

// source/game/interfaces/menus/subpages/EndScreenMissionsPage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_ENDSCREENMISSIONSPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_ENDSCREENMISSIONSPAGE_H

class EndScreenMissionsPage : public EndScreenBasePage {  // sizeof 0x1E0
public:
    virtual const jet::String &GetUITemplateName();
    EndScreenMissionsPage();
    virtual ~EndScreenMissionsPage();
    virtual void Update(int);
    virtual void OnEnter();
    virtual void Refresh();
    virtual bool IsAutoScrollTimerLocked();
private:
    void _ClearMissionBoxes();
    void InitMissionBoxes();
    void UpdateMissionBoxes();
    void InitMissionMultiplier();
    void LoadText();
public:
    static const jet::String k_uiTmplName;
private:
    MissionsMultiplierBox *m_missionsMultiplierBox;  // 0x1C8
    InterfaceArea *m_missionsParentObj;  // 0x1CC
    ustl::vector<MissionsBox *> m_missionsBox;  // 0x1D0
};

#endif

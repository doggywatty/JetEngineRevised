// source/game/interfaces/menus/subpages/RedeemCodePage.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_REDEEMCODEPAGE_H
#define SOURCE_GAME_INTERFACES_MENUS_SUBPAGES_REDEEMCODEPAGE_H

class RedeemCodePage : public BasicPage {  // sizeof 0x218
public:
    RedeemCodePage();
    virtual ~RedeemCodePage();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void Refresh();
private:
    bool RedeemNumbers_ButtonReleased(InterfaceButton*);
    void RedeemNumbers_UpdateCodeText();
    void RedeemNumbers_PopValue();
    void RedeemNumbers_PushValue(int);
    bool HandleButtons(InterfaceButton*);
    void HandleScroll();
    void ShowCodePrize(const ustl::vector<int>&, bool);
    void UpdateButtons();
    void UpdateButtonAtTop(InterfaceListButtonScroll*, InterfaceButton*);
    void UpdateButtonAtBottom(InterfaceListButtonScroll*, InterfaceButton*);
    static const unsigned k_redeemNumbers_MaxValues;
    InterfaceButton *m_redeemNumbers_delete;  // 0x1C4
    InterfaceButton *m_redeemNumbers_validate;  // 0x1C8
    ustl::vector<InterfaceButton *> m_redeemNumbers_CodeValues;  // 0x1CC
    unsigned m_redeemNumbers_OldCodeValuesSize;  // 0x1DC
    ustl::vector<int> m_redeemNumbers_inputedNumbers;  // 0x1E0
    InterfaceText *m_redeemNumbers_CodeText;  // 0x1F0
    InterfaceListButtonScroll *m_listRedeem1;  // 0x1F4
    InterfaceListButtonScroll *m_listRedeem2;  // 0x1F8
    InterfaceListButtonScroll *m_listRedeem3;  // 0x1FC
    InterfaceButton *m_buttonUp1;  // 0x200
    InterfaceButton *m_buttonUp2;  // 0x204
    InterfaceButton *m_buttonUp3;  // 0x208
    InterfaceButton *m_buttonDown1;  // 0x20C
    InterfaceButton *m_buttonDown2;  // 0x210
    InterfaceButton *m_buttonDown3;  // 0x214
};

#endif

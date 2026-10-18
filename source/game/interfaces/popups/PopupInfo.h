// source/game/interfaces/popups/PopupInfo.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPINFO_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPINFO_H

#include <string>
#include "BasePopup.h"

class InterfaceButton;
class InterfaceText;

// A message with its buttons: "ok" for a plain information; the account popups of the online login ask yes / no / cancel
class PopupInfo : public BasePopup {  // sizeof 0x1E4
public:
    enum EPopupInfoTypeDef { Info = 0, StealAccount = 1, MergeAccount = 2, MergeOrStealAccount = 3 };
    // the layer InfoPopup, or InfoPopup2 when the last argument is given (the one the shop popup uses)
    PopupInfo(EPopupInfoTypeDef type, const std::string &text);
    PopupInfo(EPopupInfoTypeDef type, const std::string &text, int param);
    virtual ~PopupInfo();
    virtual void Update(int);
    virtual void RefreshPopup();
    virtual int GetCurrentTrackingScreen() const;
    virtual void RefreshCostumes();
private:
    void Setup(const std::string &text);
    void OnYesButton();
    void OnNoButton();
    void OnOkButton();
    void OnCancelButton();
    EPopupInfoTypeDef m_type;        // 0x1C8
    InterfaceButton *m_yesButton;    // 0x1CC
    InterfaceButton *m_noButton;     // 0x1D0
    InterfaceButton *m_okButton;     // 0x1D4
    InterfaceButton *m_cancelButton; // 0x1D8
    InterfaceText *m_infoText;       // 0x1DC
    jet::String m_textPopupString;   // 0x1E0
    int m_param;                     // 0x1E8: 100 for the popup that asks the main menu to open the shop
};

#endif

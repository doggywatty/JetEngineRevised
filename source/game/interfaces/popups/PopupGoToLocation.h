// source/game/interfaces/popups/PopupGoToLocation.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPGOTOLOCATION_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPGOTOLOCATION_H

#include <functional>
#include "BasePopup.h"

class InterfaceButton;
class LocationInfoEntity;

// What a location says when it is asked about (the "GoToLocationPopup" layer): why it is locked, that it was just unlocked,
// that its data has to be downloaded, or that the player can travel there now (for a price). Depends on the menu and the game
// state it was opened over.
// Not ported: the sounds (SoundMgr), the tracking of the purchases (GameTrackingMgr) and the real money purchase of the
// IAP layout (Store / Player::HandleDeliverIAPItem): the .so only ever writes false in the flag that shows the IAP layout, so
// it is not reachable.
class PopupGoToLocation : public BasePopup {  // sizeof 0x200
public:
    // @0x4b6cc4
    PopupGoToLocation(int location, bool justUnlocked, std::function<void()> onClose);
    virtual ~PopupGoToLocation();
    virtual void Update(int);
    virtual void RefreshPopup();
    virtual int GetCurrentTrackingScreen() const { return 0x1bca4; }  // @0x4827e4
    void BuyLocation(int, int);
    void BuyFastTravel(int, int);
    void FastTravelToLocation();
    void PopPopup() { m_closing = true; }
    static bool isPopupLocation;  // @0x..: set by the constructor
private:
    void ShowLocationSpecificData();
    LocationInfoEntity *m_info;  // 0x1C8
    bool m_closing;              // 0x1CC: the popup is to be popped at the end of its update
    bool m_isIAP;                // 0x1CD
    bool m_justUnlocked;         // 0x1CE
    InterfaceButton *m_lockedByProgressBackButton;  // 0x1D0
    InterfaceButton *m_dlcBackButton;               // 0x1D4
    InterfaceButton *m_dlcGoToPageButton;           // 0x1D8
    InterfaceButton *m_canFastTravelBackButton;     // 0x1DC
    InterfaceButton *m_canFastTravelBuyButton;      // 0x1E0
    bool m_iapPurchaseInProgress;                   // 0x1E4
    bool m_unlocked;                                // 0x1E5
    InterfaceButton *m_iapBackButton;               // 0x1E8
    InterfaceButton *m_iapBuyButton;                // 0x1EC
    std::function<void()> m_onClose;                // 0x1F0
};

#endif

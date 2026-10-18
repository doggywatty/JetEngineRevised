// source/game/interfaces/menus/box/LocationBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_LOCATIONBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_LOCATIONBOX_H

#include "game/interfaces/interfaceobjects/BasicPage.h"

class InterfaceButton;
class InterfacePBar;
class LocationInfoEntity;

// One location of the Goals menu: its picture and name, the multiplier it asks for when it is locked, the download of its data
// when it is not on the device, and the button that takes the player there.
// Not ported: the sounds (SoundMgr) and the tracking of the purchases (GameTrackingMgr).
class LocationBox : public BasicPage {  // sizeof 0x1DC
public:
    LocationBox(int location);  // @0x4d3268
    virtual ~LocationBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void Refresh();  // @0x4d41b0
    int GetScoreMultiplierValue();  // @0x4d549c
    LocationInfoEntity *GetLocationInfo() const { return m_info; }
private:
    void ShowLocationSpecificData();  // @0x4d4b24
    void BuyLocation(int, int);  // @0x4d4f38
    void downloadDlc();  // @0x4d5114
    static void ConfirmDownloadDlc(void *box) { static_cast<LocationBox*>(box)->downloadDlc(); }  // @0x4d5338
    void BuyFastTravel(int, int);  // @0x4d533c
    void FastTravelToLocation();  // @0x4d538c
    LocationInfoEntity *m_info;  // 0x1BC
    bool m_needRefresh;          // 0x1C0
    InterfaceButton *m_buyButton;       // 0x1C4: Locked_BuyButton
    InterfaceButton *m_questionButton;  // 0x1C8: Locked_QuestionButton
    InterfaceButton *m_downloadButton;  // 0x1CC: DLCNeeded_DownloadButton
    InterfacePBar *m_downloadBar;       // 0x1D0: the progress of the download, over the button
    InterfaceButton *m_goButton;        // 0x1D4: Unlocked_GoToLocationButtonStart
    bool m_unlocked;                    // 0x1D8: what the box shows
    bool m_travelled;                   // 0x1D9: the player went to the location, the game goes to the main menu
};

#endif

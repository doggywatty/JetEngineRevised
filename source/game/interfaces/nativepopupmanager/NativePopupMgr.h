// source/game/interfaces/nativepopupmanager/NativePopupMgr.h
#ifndef SOURCE_GAME_INTERFACES_NATIVEPOPUPMANAGER_NATIVEPOPUPMGR_H
#define SOURCE_GAME_INTERFACES_NATIVEPOPUPMANAGER_NATIVEPOPUPMGR_H

class NativePopupMgr : public Singleton<NativePopupMgr> {  // sizeof 0x8
public:
    NativePopupMgr();
    virtual ~NativePopupMgr();
    void addNewPopup(const char*, const char*, const char*, bool, boost::function<void __cdecl(void)>, boost::function<void __cdecl(void)>);
private:
    NativePopupMgrAdapter *popupMgr;  // 0x4
};

#endif

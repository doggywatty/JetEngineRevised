// source/game/interfaces/nativepopupmanager/nativepopupmgradapter.h
#ifndef SOURCE_GAME_INTERFACES_NATIVEPOPUPMANAGER_NATIVEPOPUPMGRADAPTER_H
#define SOURCE_GAME_INTERFACES_NATIVEPOPUPMANAGER_NATIVEPOPUPMGRADAPTER_H

class NativePopupMgrAdapter {  // sizeof 0x4
public:
    virtual ~NativePopupMgrAdapter();
    virtual void addNewPopup(const char*, const char*, const char*, bool, boost::function<void __cdecl(void)>, boost::function<void __cdecl(void)>) = 0;
};

#endif

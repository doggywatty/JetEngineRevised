// source/game/interfaces/nativepopupmanager/nativepopupmgradapterfactory.h
#ifndef SOURCE_GAME_INTERFACES_NATIVEPOPUPMANAGER_NATIVEPOPUPMGRADAPTERFACTORY_H
#define SOURCE_GAME_INTERFACES_NATIVEPOPUPMANAGER_NATIVEPOPUPMGRADAPTERFACTORY_H

class NativePopupMgrAdapterFactory {  // sizeof 0x1
public:
    static NativePopupMgrAdapter *createNewNativePopupMgrAdapter();
private:
    NativePopupMgrAdapterFactory();
    ~NativePopupMgrAdapterFactory();
};

#endif

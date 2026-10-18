// source/game/interfaces/WaitingScreenMgr.h
#ifndef SOURCE_GAME_INTERFACES_WAITINGSCREENMGR_H
#define SOURCE_GAME_INTERFACES_WAITINGSCREENMGR_H

class WaitingScreenMgr : public Singleton<WaitingScreenMgr> {  // sizeof 0x20
public:
    WaitingScreenMgr();
    virtual ~WaitingScreenMgr();
    void Init();
    void Update(int);
    void Render(const jet::scene::Camera&, jet::video::Painter&);
    bool IsEnabled();
    void PushWaitingScreen(void (void*)*, void*, int);
    void PopWaitingScreen();
    void PushWaitingScreenTimer(int);
private:
    void Refresh();
    BasicPage *m_waitingScreen;  // 0x4
    std::vector<WaitingScreenMgr::WaitingData,std::allocator<WaitingScreenMgr::WaitingData> > m_waitingDataList;  // 0x8
    InterfaceButton *m_cancelButton;  // 0x1C
};

#endif

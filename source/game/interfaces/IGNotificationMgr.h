// source/game/interfaces/IGNotificationMgr.h
#ifndef SOURCE_GAME_INTERFACES_IGNOTIFICATIONMGR_H
#define SOURCE_GAME_INTERFACES_IGNOTIFICATIONMGR_H

class IGNotificationMgr : public Singleton<IGNotificationMgr> {  // sizeof 0x30
public:
    IGNotificationMgr();
    virtual ~IGNotificationMgr();
    void Init();
    void Push(BaseIGNotification*);
    void Pop();
    void ForceClearStack();
    void Update(int);
    void Render(const jet::scene::Camera&, jet::video::Painter&);
    void Refresh();
    float GetRenderedHeight();
private:
    void _ClearDelayedDestructionArray();
    std::vector<BaseIGNotification *,std::allocator<BaseIGNotification *> > m_instancesPool;  // 0x4
    std::vector<BaseIGNotification *,std::allocator<BaseIGNotification *> > m_delayedDestructionArray;  // 0x18
    BaseIGNotification *m_lastInstance;  // 0x2C
};

#endif

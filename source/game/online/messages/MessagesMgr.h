// source/game/online/messages/MessagesMgr.h
#ifndef SOURCE_GAME_ONLINE_MESSAGES_MESSAGESMGR_H
#define SOURCE_GAME_ONLINE_MESSAGES_MESSAGESMGR_H

class MessagesMgr : public Singleton<MessagesMgr> {  // sizeof 0x24
public:
    MessagesMgr();
    virtual ~MessagesMgr();
    void Init();
    void Update(int);
    unsigned GetMessagesCount(MessagesMgr::_Inboxes);
    GameMessage *GetMessageAt(MessagesMgr::_Inboxes, unsigned);
    bool SendGameMessage(GameMessage*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool SendGameMessage(GameMessage*, const jet::String&);
    void ForceLoad(MessagesMgr::_Inboxes);
    bool IsReady();
    void RefreshMessagesData();
protected:
    void OnMessageMarkedToBeRemoved(MessagesMgr::_Inboxes);
private:
    int GetStatus(MessagesMgr::_Inboxes);
    void LoadMessages(MessagesMgr::_Inboxes);
    void SaveMessages(MessagesMgr::_Inboxes);
    void ClearMessages(MessagesMgr::_Inboxes);
    static bool sOnInboxLoaded(void*, bool, social::OnlineEventData);
    static bool sOnInboxSaved(void*, bool, social::OnlineEventData);
    static bool sOnSecureboxLoaded(void*, bool, social::OnlineEventData);
    static bool sOnSecureboxSaved(void*, bool, social::OnlineEventData);
    void RemoveMessages(MessagesMgr::_Inboxes);
    static const int k_minimumRefreshIntervalSecs;
    static const int k_minimumSaveIntervalSecs;
    static const int k_minimumLoadIntervalSecs;
    std::map<enum MessagesMgr::_Inboxes,MessagesMgr::_Inbox *,std::less<enum MessagesMgr::_Inboxes>,std::allocator<std::pair<enum MessagesMgr::_Inboxes const ,MessagesMgr::_Inbox *> > > m_inboxes;  // 0x4
    bool m_isInitialized;  // 0x20
};

#endif

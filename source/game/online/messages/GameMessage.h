// source/game/online/messages/GameMessage.h
#ifndef SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGE_H
#define SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGE_H

class GameMessage {  // sizeof 0x34
public:
    static GameMessage *Create(const social::MessageSecure*, const Json::Value&);
    static GameMessage *Create(const social::MessageIn*);
protected:
    GameMessage(const social::MessageIn*);
public:
    GameMessage(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~GameMessage();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetSender();
    GameMessage::GameMessageType GetMessageType();
    MessagesMgr::_Inboxes GetInboxType();
    void MarkToBeRemoved();
    bool NeedsToBeRemoved();
protected:
    const social::MessageIn *GetMsg();
    virtual social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> CreateMsg(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&) = 0;
private:
    static GameMessage *ParseGiftMessage(const social::MessageSecure*, const Json::Value&);
    static GameMessage *ParseCCRestoreMessage(const social::MessageSecure*, const Json::Value&, bool);
protected:
    const social::MessageIn *m_message;  // 0x4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_sender;  // 0x8
    GameMessage::GameMessageType m_msgType;  // 0x28
    MessagesMgr::_Inboxes m_inboxType;  // 0x2C
private:
    bool m_needsToBeRemoved;  // 0x30
};

#endif

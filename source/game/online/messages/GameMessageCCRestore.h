// source/game/online/messages/GameMessageCCRestore.h
#ifndef SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGECCRESTORE_H
#define SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGECCRESTORE_H

class GameMessageCCRestore : public GameMessage {  // sizeof 0x58
public:
    GameMessageCCRestore(social::MessageIn*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    virtual ~GameMessageCCRestore();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSaveKey();
    bool IsMandatory();
protected:
    virtual social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> CreateMsg(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_saveKey;  // 0x34
    bool m_isMandatory;  // 0x54
};

#endif

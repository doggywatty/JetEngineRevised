// source/game/online/messages/GameMessageGift.h
#ifndef SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGEGIFT_H
#define SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGEGIFT_H

class GameMessageGift : public GameMessage {  // sizeof 0x4C
public:
    GameMessageGift(social::MessageIn*, const jet::String&);
    virtual ~GameMessageGift();
    void AddGift(const GameMessageGift::MessageGift&);
    const std::vector<GameMessageGift::MessageGift,std::allocator<GameMessageGift::MessageGift> > &GetGifts();
    jet::String GetGiftMessage();
protected:
    virtual social::IntrusivePointer<social::MessageOut,social::IntrusivePointerNoLock> CreateMsg(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    std::vector<GameMessageGift::MessageGift,std::allocator<GameMessageGift::MessageGift> > m_gifts;  // 0x34
    jet::String m_giftMessage;  // 0x48
};

#endif

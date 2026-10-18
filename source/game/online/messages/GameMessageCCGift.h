// source/game/online/messages/GameMessageCCGift.h
#ifndef SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGECCGIFT_H
#define SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGECCGIFT_H

class GameMessageCCGift : public GameMessageGift {  // sizeof 0x4C
public:
    GameMessageCCGift(social::MessageIn*, const jet::String&);
    virtual ~GameMessageCCGift();
};

#endif

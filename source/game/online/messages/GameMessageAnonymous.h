// source/game/online/messages/GameMessageAnonymous.h
#ifndef SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGEANONYMOUS_H
#define SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGEANONYMOUS_H

class GameMessageAnonymous : public GameMessage {  // sizeof 0x34
public:
    GameMessageAnonymous(social::MessageIn*);
    virtual ~GameMessageAnonymous();
};

#endif

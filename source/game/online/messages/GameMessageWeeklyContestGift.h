// source/game/online/messages/GameMessageWeeklyContestGift.h
#ifndef SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGEWEEKLYCONTESTGIFT_H
#define SOURCE_GAME_ONLINE_MESSAGES_GAMEMESSAGEWEEKLYCONTESTGIFT_H

class GameMessageWeeklyContestGift : public GameMessageGift {  // sizeof 0x4C
public:
    GameMessageWeeklyContestGift(social::MessageIn*, const jet::String&);
    virtual ~GameMessageWeeklyContestGift();
};

#endif

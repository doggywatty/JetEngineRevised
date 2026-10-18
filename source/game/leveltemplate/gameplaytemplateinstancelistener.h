// source/game/leveltemplate/gameplaytemplateinstancelistener.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_GAMEPLAYTEMPLATEINSTANCELISTENER_H
#define SOURCE_GAME_LEVELTEMPLATE_GAMEPLAYTEMPLATEINSTANCELISTENER_H

// What a gameplay template instance tells those that follow it
struct GameplayTemplateInstanceEvent {
    enum Type { Destroyed = 0, BifurcationResolved = 1 };
    int type;
};

class GameplayTemplateInstanceListener {  // sizeof 0x4
public:
    virtual ~GameplayTemplateInstanceListener() {}
    virtual void OnGameplayTemplateInstanceEvent(const GameplayTemplateInstanceEvent&) = 0;
};

#endif

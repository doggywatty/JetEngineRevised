// source/game/entities/input/playerinputcontroller.h
#ifndef SOURCE_GAME_ENTITIES_INPUT_PLAYERINPUTCONTROLLER_H
#define SOURCE_GAME_ENTITIES_INPUT_PLAYERINPUTCONTROLLER_H

class PlayerInputController : public GameEntity {  // sizeof 0x90
public:
    PlayerInputController(const clara::Template*);
    virtual ~PlayerInputController();
    virtual void Update(Minion*) = 0;
    virtual void OnEnabled();
    virtual void OnDisabled();
};

#endif

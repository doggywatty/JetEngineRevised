// source/game/entities/GameEntityCreator.h
#ifndef SOURCE_GAME_ENTITIES_GAMEENTITYCREATOR_H
#define SOURCE_GAME_ENTITIES_GAMEENTITYCREATOR_H

#include "entity.h"

// GameLevel::GameEntityCreator (CreateEntity @0x53954c): the game class for a template name;
// templates without one stay plain clara entities.
class GameEntityCreator : public clara::Creator {
public:
    virtual clara::DataEntity *CreateEntity(const clara::Template *t) const;
};

#endif

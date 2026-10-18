// source/game/entities/ObjectHolder.h
#ifndef SOURCE_GAME_ENTITIES_OBJECTHOLDER_H
#define SOURCE_GAME_ENTITIES_OBJECTHOLDER_H

class Object;

// What makes an object of its own (platform instances, interactive object instances): the level template instance takes the object
// and the entity may go, unless it was asked to keep it
class ObjectHolder {
public:
    virtual ~ObjectHolder() {}
    virtual Object *GetCreatedEntity() const = 0;
    virtual void SetKeepCreatedEntity(bool) = 0;
};

#endif

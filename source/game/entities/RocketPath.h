// source/game/entities/RocketPath.h
#ifndef SOURCE_GAME_ENTITIES_ROCKETPATH_H
#define SOURCE_GAME_ENTITIES_ROCKETPATH_H

#include "GameEntity.h"
#include "pacesettercontroller.h"

// The path the pacesetter follows in the rocket and fluffy modes (the rest of the class comes with the gameplay port)
class RocketPath : public GameEntity, public PacesetterController {  // sizeof 0xA4
public:
    static jet::String k_tmplName;
    explicit RocketPath(const clara::Template *t) : GameEntity(t) {}
    virtual bool GetTransform(Pacesetter*, math::vec3<float>&, math::quat<float>&) = 0;
};

#endif

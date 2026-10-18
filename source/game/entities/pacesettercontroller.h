// source/game/entities/pacesettercontroller.h
#ifndef SOURCE_GAME_ENTITIES_PACESETTERCONTROLLER_H
#define SOURCE_GAME_ENTITIES_PACESETTERCONTROLLER_H

#include "smath.h"

class Pacesetter;

// What takes the place of the lanes in moving a pacesetter (the rocket's path)
class PacesetterController {  // sizeof 0x4
public:
    virtual ~PacesetterController() {}
    virtual bool GetTransform(Pacesetter*, math::vec3<float>&, math::quat<float>&) = 0;
    virtual void StartControl(Pacesetter*) = 0;
    virtual void EndControl(Pacesetter*) = 0;
};

#endif

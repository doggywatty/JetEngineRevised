// source/game/entities/pacesetterfollower.h
#ifndef SOURCE_GAME_ENTITIES_PACESETTERFOLLOWER_H
#define SOURCE_GAME_ENTITIES_PACESETTERFOLLOWER_H

#include "smath.h"
#include "Pacesetter.h"

// What moves along a pacesetter
class PacesetterFollower {  // sizeof 0x8
public:
    PacesetterFollower() : m_pacesetter(0) {}
    virtual void GoTo(const math::vec3<float>&, const math::quat<float>&) = 0;
    virtual ~PacesetterFollower() {}
    void SetPacesetter(Pacesetter *p) { m_pacesetter = p; }
    Pacesetter *GetPacesetter() const { return m_pacesetter; }
    virtual void OnGameplayTemplateInstanceDestroyed() {}
    virtual void NotifyInBonusGameplay(bool) {}
    virtual void OnPacesetterEvent(const Pacesetter::PacesetterEvent&) {}
private:
    Pacesetter *m_pacesetter;  // 0x4
};

#endif

// source/game/entities/ParticleSystemEntity.h
#ifndef SOURCE_GAME_ENTITIES_PARTICLESYSTEMENTITY_H
#define SOURCE_GAME_ENTITIES_PARTICLESYSTEMENTITY_H

#include <memory>
#include "GameEntity.h"
#include "particlesystem.h"

// A particle system placed in the level: made from its "Template" on PostInit, updated by hand and moving with the entity
class ParticleSystemEntity : public GameEntity {  // sizeof 0xA8
public:
    explicit ParticleSystemEntity(const clara::Template *t) : GameEntity(t), m_oldPos(0, 0, 0), m_isOnDeserialize(false) { m_type = 8; }
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void SetVisible(bool);
    virtual void SetEnabled(bool);
    virtual void NotifyTransformChanged(bool);
    static jet::String k_tmplName;
private:
    boost::shared_ptr<ps::ParticleSystem> m_ps;  // 0x8C
    math::vec3<float> m_oldPos;                  // 0x94
    bool m_isOnDeserialize;                      // 0xA0
};

#endif

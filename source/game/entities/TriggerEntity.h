// source/game/entities/TriggerEntity.h
#ifndef SOURCE_GAME_ENTITIES_TRIGGERENTITY_H
#define SOURCE_GAME_ENTITIES_TRIGGERENTITY_H

// A collision box of the level that runs its script's OnEnter when a collision starts touching it and OnExit when it stops (the lab's
// trigger resumes the creation of the templates this way). Its collisions are gathered by CollidesWith and told in Update.
#include <vector>
#include "Collision3d.h"
#include "collision3dlistener.h"

class ScriptData;

class TriggerEntity : public Collision3d, public Collision3dListener {  // sizeof 0xD0
public:
    // a collision touching the trigger: whether it was inside at the last update, and whether it is inside now
    struct Collider {  // sizeof 0x8
        GameEntity *entity;
        bool inside;
        bool touched;
    };
    virtual const jet::String &GetBaseTemplateName();
    TriggerEntity(const clara::Template*);
    virtual ~TriggerEntity();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    virtual GameEntity *GetAsEntity();
    virtual bool IsOutOfScene();
private:
    void OnEnter(GameEntity*);
    void OnExit(GameEntity*);
public:
    static jet::String k_tmplName;
private:
    std::vector<TriggerEntity::Collider> m_colliders;  // 0xB8 (the listener's base is at 0xB8, its vector at 0xBC)
    ScriptData *m_scriptData;  // 0xC8
    static const jet::String k_onEnter;
    static const jet::String k_onExit;
};

#endif

// source/game/entities/Object.h
#ifndef SOURCE_GAME_ENTITIES_OBJECT_H
#define SOURCE_GAME_ENTITIES_OBJECT_H

#include <list>
#include <vector>
#include "GameEntity.h"
#include "collision3dlistener.h"

class Deco3d;
class EffectDef;
class Collision3d;

// A placed thing made of a deco (picked among the "objectDeco" references) the model of the platforms and gameplay
// objects, with its collision (Collision3d, from "objectCollision3d"). Not ported yet: the sounds and particles attached to the entity.
class Object : public GameEntity, public Collision3dListener {  // sizeof 0xC8
public:
    static const jet::String k_tmplName, k_objectDeco, k_objectCollision3d, k_effectOnContact, k_effectOnContactSpawnAll, k_movieLinkedDeco;
    explicit Object(const clara::Template *t);
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void NotifyAnimationChanged();
    virtual bool GetLinkPointTransform(const jet::String &point, math::vec3<float> &pos, math::quat<float> &rot) const;
    virtual bool IsOutOfScene() const;
    virtual void SetPersistenceType(int t);
    virtual void OnPositionedInWorld();
    // Collision3dListener: what touches the object's collision (@0x57faec, @0x518704)
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    virtual GameEntity *GetAsEntity() { return this; }
    Collision3d *GetCollision() const { return m_collision; }
    Deco3d *GetDeco() const { return m_deco; }
    // the first of the decos made by AddLinkedEntity (the list at 0x94; the collision is at 0xC4)
    Deco3d *GetLinkedDeco() const { return m_linkedDecos.empty() ? 0 : m_linkedDecos.front(); }
    // the deco made for this object takes the place of another one
    void AdoptDeco3d(Deco3d *d);
    void DestroyDeco3d();
    // the entity linked to the deco (as an extra copy of the deco)
    void AddLinkedEntity(Deco3d *d);
    bool CanLoadModel() const;
    // the layer the effects of an interface copy play in (EffectMgr)
    int GetEffectLayerId() const { return m_effectLayerId; }
    // @0x57f7d8: the effects start at the object's place and rotation (the link point of the linked deco's when the object is
    // linked to a deco); randomOne starts one of them picked at random, else every one
    void SpawnEffects(const std::vector<const EffectDef*> &effects, bool randomOne);
    bool m_decoTransferred;  // 0x90
protected:
    void InitDeco3d(Deco3d *ref);
    void InitMovieDeco();
    void InitEffectOnContact();
    // @0x57f59c: the collision of the data, linked to the object
    void InitCollision3D();
    // the deco asked by the data (one of the "objectDeco" references, chosen once), or the default
    Deco3d *GetDecoReference(unsigned *index, bool onlyLoadable) const;
    std::list<Deco3d*> m_linkedDecos;       // 0x94
    unsigned m_ctorFlags;                   // 0x9C
    int m_effectLayerId;                    // 0xA0: the effects layer of an interface copy
    std::vector<const EffectDef*> m_effectsOnContact;  // 0xA4
    Collision3d *m_collision = 0;           // 0xC4
    Deco3d *m_deco;                         // 0xB0
    mutable Deco3d *m_decoReference;        // 0xB4
    Deco3d *m_defaultDeco;                  // 0xB8
    mutable unsigned m_decoIndex;           // 0xBC
    bool m_effectOnContactSpawnAll;         // 0xC0
};

#endif

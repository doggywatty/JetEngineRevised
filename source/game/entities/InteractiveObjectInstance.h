// source/game/entities/InteractiveObjectInstance.h
#ifndef SOURCE_GAME_ENTITIES_INTERACTIVEOBJECTINSTANCE_H
#define SOURCE_GAME_ENTITIES_INTERACTIVEOBJECTINSTANCE_H

#include "GameEntity.h"
#include "ObjectHolder.h"

class InteractiveObjectTemplate;
class Object;

// An interactive object placed in a level template (type 0xE): it makes a copy of the InteractiveObjectTemplate its "Template" parameter names,
// at its own place, and the level template instance takes that object (the entity is then not needed, unless it is asked to keep it).
class InteractiveObjectInstance : public GameEntity, public ObjectHolder {  // 0x94 bytes in the .so
public:
    explicit InteractiveObjectInstance(const clara::Template*);
    virtual ~InteractiveObjectInstance();
    virtual void Init();
    virtual void PostInit();
    virtual const math::vec3<float> &GetPosition() const;
    virtual void SetPosition(const math::vec3<float>&);
    virtual const math::quat<float> &GetRotation() const;
    virtual void SetRotation(const math::quat<float>&);
    virtual void Translate(const math::vec3<float>&);
    virtual void SetPersistenceType(int t);
    virtual void SetKeepCreatedEntity(bool b) { m_keepCreatedEntity = b; }
    virtual Object *GetCreatedEntity() const;
    static jet::String k_tmplName;
private:
    Object *m_createdObject;       // 0x8C
    bool m_keepCreatedEntity;      // 0x90
    static const jet::String k_template;
};

#endif

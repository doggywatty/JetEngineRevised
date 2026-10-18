// source/game/entities/BananaInstance.h
#ifndef SOURCE_GAME_ENTITIES_BANANAINSTANCE_H
#define SOURCE_GAME_ENTITIES_BANANAINSTANCE_H

class BananaInstance : public GameEntity {  // sizeof 0x98
public:
    BananaInstance(const clara::Template*);
    virtual ~BananaInstance();
    virtual void Init();
    virtual void PostInit();
    virtual const math::vec3<float> &GetPosition();
    virtual void SetPosition(const math::vec3<float>&);
    virtual const math::quat<float> &GetRotation();
    virtual void SetRotation(const math::quat<float>&);
    virtual void Translate(const math::vec3<float>&);
    virtual void SetPersistenceType(safe_enum<GameEntity::PersistenceTypeDef,enum GameEntity::PersistenceTypeDef::type>);
    void SetKeepCreatedEntity(bool);
    Object *GetCreatedEntity();
    static jet::String k_tmplName;
private:
    Object *m_createdObject;  // 0x90
    bool m_keepCreatedEntity;  // 0x94
    static const jet::String k_template;
};

#endif

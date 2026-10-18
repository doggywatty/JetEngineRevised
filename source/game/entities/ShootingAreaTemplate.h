// source/game/entities/ShootingAreaTemplate.h
#ifndef SOURCE_GAME_ENTITIES_SHOOTINGAREATEMPLATE_H
#define SOURCE_GAME_ENTITIES_SHOOTINGAREATEMPLATE_H

class ShootingAreaTemplate : public Object {  // sizeof 0xE0
public:
    ShootingAreaTemplate(const clara::Template*);
    virtual ~ShootingAreaTemplate();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    static jet::String k_tmplName;
private:
    bool m_isColliding;  // 0xDC
    bool m_wasColliding;  // 0xDD
};

#endif

// framework/include/physics/bullet/bulletcollisiondrawer.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETCOLLISIONDRAWER_H
#define FRAMEWORK_INCLUDE_PHYSICS_BULLET_BULLETCOLLISIONDRAWER_H

class BulletCollisionDrawer : public btIDebugDraw {  // sizeof 0x8
public:
    BulletCollisionDrawer(CollisionDrawer*);
    virtual void drawLine(const btVector3&, const btVector3&, const btVector3&);
    virtual void drawContactPoint(const btVector3&, const btVector3&, float, int, const btVector3&);
    virtual void reportErrorWarning(const char*);
    virtual void draw3dText(const btVector3&, const char*);
    virtual void setDebugMode(int);
    virtual int getDebugMode();
private:
    CollisionDrawer *m_drawer;  // 0x4
};

#endif

// framework/include/physics/collisiondrawer.h
#ifndef FRAMEWORK_INCLUDE_PHYSICS_COLLISIONDRAWER_H
#define FRAMEWORK_INCLUDE_PHYSICS_COLLISIONDRAWER_H

class CollisionDrawer : public DebugDrawer {  // sizeof 0xC
public:
    CollisionDrawer(jet::video::Painter*, jet::scene::Camera*);
    virtual void DrawLine(const math::vec3<float>&, const math::vec3<float>&, const math::vec3<float>&);
    virtual void DrawContactPoint(const math::vec3<float>&, const math::vec3<float>&, float, unsigned, const math::vec3<float>&);
    virtual void DrawTextA(const math::vec3<float>&, const jet::String&);
    virtual void Alert(const jet::String&);
    void DrawVolume(const math::mat4<float>&, const math::vec3<float>&, const math::vec3<float>&);
    void DrawCapsule(const math::vec3<float>&, const math::vec3<float>&, const float, const math::vec3<float>&);
    void DrawTriangle(const math::vec3<float>&, const math::vec3<float>&, const math::vec3<float>&, const math::vec3<float>&);
private:
    jet::video::Painter *m_painter;  // 0x4
    jet::scene::Camera *m_camera;  // 0x8
};

#endif

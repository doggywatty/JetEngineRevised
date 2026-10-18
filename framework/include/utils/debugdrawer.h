// framework/include/utils/debugdrawer.h
#ifndef FRAMEWORK_INCLUDE_UTILS_DEBUGDRAWER_H
#define FRAMEWORK_INCLUDE_UTILS_DEBUGDRAWER_H

class DebugDrawer {  // sizeof 0x4
public:
    virtual ~DebugDrawer();
    virtual void DrawLine(const math::vec3<float>&, const math::vec3<float>&, const math::vec3<float>&) = 0;
    virtual void DrawContactPoint(const math::vec3<float>&, const math::vec3<float>&, float, unsigned, const math::vec3<float>&) = 0;
    virtual void DrawTextA(const math::vec3<float>&, const jet::String&) = 0;
    virtual void Alert(const jet::String&) = 0;
};

#endif

// core/jet/include/scene/occluder.h
#ifndef CORE_JET_INCLUDE_SCENE_OCCLUDER_H
#define CORE_JET_INCLUDE_SCENE_OCCLUDER_H

#include "smath.h"
#include "jenum.h"

namespace jet::scene {
class Camera;

class Occluder {
public:
    struct TypeDef { enum type { Sphere, Box, Plane }; };
    virtual ~Occluder() {}
    virtual safe_enum<TypeDef, TypeDef::type> GetType() = 0;
    const math::vec3<float> &GetPosition() const { return m_position; }
    const math::quat<float> &GetRotation() const { return m_rotation; }
    void SetTransform(const math::vec3<float> &p, const math::quat<float> &q) { m_position = p; m_rotation = q; }
    virtual bool Setup(const Camera*) = 0;
    virtual bool Occlude(const math::vec3<float>&, const float) = 0;
    virtual void GetBoundingSphere(math::vec3<float>&, float&) = 0;
protected:
    math::vec3<float> m_position;
    math::quat<float> m_rotation;
};
}

#endif

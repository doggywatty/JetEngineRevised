// core/jet/include/video/lightdata.h
#ifndef CORE_JET_INCLUDE_VIDEO_LIGHTDATA_H
#define CORE_JET_INCLUDE_VIDEO_LIGHTDATA_H

#include "jenum.h"
#include "smath.h"
#include "video/color.h"

namespace jet::video {
class LightData {
public:
    // Not extracted from the .so (no string-based parser found for it, unlike most other small
    // enums here); Point/Directional/Spot is the conventional ordering, not independently verified.
    struct TypeDef { enum type { Point, Directional, Spot }; };

    LightData() : m_type(TypeDef::Point), m_radius(0) {}
    virtual ~LightData() {}
    LightData &operator=(const LightData&) = default;

    safe_enum<TypeDef,TypeDef::type> GetType() { return m_type; }
    void SetType(safe_enum<TypeDef,TypeDef::type> t) { m_type = t; }
    const math::vec3<float> &GetPosition() { return m_position; }
    void SetPosition(const math::vec3<float> &p) { m_position = p; }
    const math::vec3<float> &GetDirection() { return m_direction; }
    void SetDirection(const math::vec3<float> &d) { m_direction = d; }
    const math::vec3<float> &GetAmbientColor() { return m_ambientColor; }
    void SetAmbientColor(const math::vec3<float> &c) { m_ambientColor = c; }
    void SetAmbientColor(const Color &c) { m_ambientColor = math::vec3<float>(c.r, c.g, c.b); }
    const math::vec3<float> &GetDiffuseColor() { return m_diffuseColor; }
    void SetDiffuseColor(const math::vec3<float> &c) { m_diffuseColor = c; }
    void SetDiffuseColor(const Color &c) { m_diffuseColor = math::vec3<float>(c.r, c.g, c.b); }
    const math::vec3<float> &GetSpecularColor() { return m_specularColor; }
    void SetSpecularColor(const math::vec3<float> &c) { m_specularColor = c; }
    void SetSpecularColor(const Color &c) { m_specularColor = math::vec3<float>(c.r, c.g, c.b); }
    float GetRadius() { return m_radius; }
    void SetRadius(float r) { m_radius = r; }
protected:
    safe_enum<TypeDef,TypeDef::type> m_type;
    math::vec3<float> m_position;
    math::vec3<float> m_direction;
    float m_radius;
    math::vec3<float> m_ambientColor;
    math::vec3<float> m_diffuseColor;
    math::vec3<float> m_specularColor;
};
}

#endif

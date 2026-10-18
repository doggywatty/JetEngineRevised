// framework/src/camera/cinematiccamera.cpp
#include "camera/cinematiccamera.h"

jet::String CinematicCamera::k_cameraType("CinematicCamera");

// @0x5ab218's matrix: the local rotation and translation
math::mat4<float> CinematicCamera::GetLocalTransform() const {
    math::mat4<float> m;
    m.setIdentity();
    math::mat3<float> r = m_localRotation.getAsMat3();
    m.m[0] = r.m[0]; m.m[1] = r.m[1]; m.m[2] = r.m[2];
    m.m[4] = r.m[3]; m.m[5] = r.m[4]; m.m[6] = r.m[5];
    m.m[8] = r.m[6]; m.m[9] = r.m[7]; m.m[10] = r.m[8];
    m.m[12] = m_localPosition.x; m.m[13] = m_localPosition.y; m.m[14] = m_localPosition.z;
    return m;
}

// @0x5ab06c: the animated camera's position seen through the local transform
math::vec3<float> CinematicCamera::GetPosition() const {
    math::vec3<float> p(0, 0, 0);
    if (m_animatedCamera) p = m_animatedCamera->GetPosition();
    math::mat4<float> m = GetLocalTransform();
    return math::vec3<float>(p.x * m.m[0] + p.y * m.m[4] + p.z * m.m[8] + m.m[12],
                             p.x * m.m[1] + p.y * m.m[5] + p.z * m.m[9] + m.m[13],
                             p.x * m.m[2] + p.y * m.m[6] + p.z * m.m[10] + m.m[14]);
}

// @0x5ab124: models look down +Y, the scene camera down -Y
math::quat<float> CinematicCamera::GetRotation() const {
    math::quat<float> r(0, 0, 0, 1);
    if (m_animatedCamera) r = m_animatedCamera->GetRotation();
    float s, c;
    math::sincos(0.7853982f, s, c);
    return m_localRotation * r * math::quat<float>(s, 0, 0, c) * math::quat<float>(math::quat<float>::axisZ, 3.1415927f);
}

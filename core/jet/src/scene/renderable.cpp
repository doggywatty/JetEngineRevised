// core/jet/src/scene/renderable.cpp
#include "scene/renderable.h"

namespace jet::scene {

Renderable::Renderable()
    : m_center(0, 0, 0), m_radius(0), m_distanceToCamera(0), m_screenRadius(0), m_cullStatus(CullStatusDef::Inside),
      m_boundingBox(math::vec3<float>(-1, -1, -1), math::vec3<float>(1, 1, 1)),
      m_useDistanceCulling(true), m_useFrustumCulling(true), m_useScreenRadiusCulling(true), m_hasBoundingBox(false) {}

}

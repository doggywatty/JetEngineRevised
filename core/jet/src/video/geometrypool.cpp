// core/jet/src/video/geometrypool.cpp
#include "video/geometrypool.h"

namespace jet::video {

GeometryPool::GeometryPool(unsigned maxGeometries)
    : m_maxGeometries(maxGeometries), m_cursor(0), m_isInitialized(true) {}

GeometryPool::~GeometryPool() {}

boost::shared_ptr<Geometry> GeometryPool::GetGeometry() {
    if (m_cursor < m_geometries.size()) return m_geometries[m_cursor++];
    boost::shared_ptr<Geometry> g = CreateGeometry ? CreateGeometry() : boost::shared_ptr<Geometry>();
    if (m_geometries.size() < m_maxGeometries) m_geometries.push_back(g);
    m_cursor++;
    return g;
}

void GeometryPool::Reset() { m_cursor = 0; }

}

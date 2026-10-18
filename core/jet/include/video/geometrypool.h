// core/jet/include/video/geometrypool.h
#ifndef CORE_JET_INCLUDE_VIDEO_GEOMETRYPOOL_H
#define CORE_JET_INCLUDE_VIDEO_GEOMETRYPOOL_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include "video/geometry.h"

namespace jet::video {
// Round-robin geometry cache: GetGeometry() hands out the next slot each call, growing
// (via CreateGeometry) up to m_maxGeometries; Reset() rewinds the cursor to slot 0 so a
// caller can reuse the same geometries next frame without reallocating.
class GeometryPool {  // sizeof 0x40
public:
    boost::shared_ptr<jet::video::Geometry> GetGeometry();
    void Reset();
    GeometryPool(unsigned);
    ~GeometryPool();
    boost::function0<boost::shared_ptr<jet::video::Geometry> > CreateGeometry;  // 0x0
private:
    std::vector<boost::shared_ptr<jet::video::Geometry> > m_geometries;  // 0x20
    unsigned m_maxGeometries;  // 0x34
    unsigned m_cursor;
    bool m_isInitialized;  // 0x38
};
}

#endif

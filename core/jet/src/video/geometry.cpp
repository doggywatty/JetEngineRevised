// core/jet/src/video/geometry.cpp
#include "video/geometry.h"
#include "video/gles20/gles20geometry.h"

namespace jet::video {

unsigned Geometry::ms_attributeCPUAccessVertexLimit = 0;
unsigned Geometry::ms_indexCPUAccessVertexLimit = 0;
bool Geometry::ms_useMapBufferData = false;
bool Geometry::ms_useVAO = false;

Geometry::~Geometry() {}

// The original dispatches through the active jet::video::Driver's geometry factory (a vtable
// slot); GLES2 is the only backend this engine ever shipped with, so New() just builds one
// directly until a Driver abstraction is worth adding for its own sake.
boost::shared_ptr<Geometry> Geometry::New() {
    return boost::shared_ptr<Geometry>(new GLES20Geometry());
}

void Geometry::SetAttributeCPUAccess(unsigned v) {
    if (v > ms_attributeCPUAccessVertexLimit) ms_attributeCPUAccessVertexLimit = v;
}
unsigned Geometry::GetAttributeCPUAccess() { return ms_attributeCPUAccessVertexLimit; }

void Geometry::SetIndexCPUAccess(unsigned v) {
    if (v > ms_indexCPUAccessVertexLimit) ms_indexCPUAccessVertexLimit = v;
}
unsigned Geometry::GetIndexCPUAccess() { return ms_indexCPUAccessVertexLimit; }

void Geometry::SetUseVAO(bool v) { ms_useVAO = v; }
bool Geometry::GetUseVAO() { return ms_useVAO; }
bool Geometry::GetUseMapBufferData() { return ms_useMapBufferData; }
void Geometry::SetUseMapBufferData(bool v) { ms_useMapBufferData = v; }

}

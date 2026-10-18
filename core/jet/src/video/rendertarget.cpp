// core/jet/src/video/rendertarget.cpp
#include "video/rendertarget.h"
#include "video/gles20/gles20rendertarget.h"
#include "pugixml.hpp"

namespace jet::video {

RenderTarget::~RenderTarget() {}

boost::shared_ptr<RenderTarget> RenderTarget::New() {
    return boost::shared_ptr<RenderTarget>(new GLES20RenderTarget());
}

// Verified against libdespicablemefree.so 0.0.3 (RenderTarget::Load @0x61d0f8): dispatches to
// the concrete LoadV100 override when the node has no "version" attribute or it reads 100.
void RenderTarget::Load(const pugi::xml_node &node) {
    pugi::xml_attribute versionAttr = node.attribute("version");
    int version = versionAttr ? versionAttr.as_int() : 100;
    if (version == 100) LoadV100(node);
}

void RenderTarget::LoadV100(const pugi::xml_node&) {}

}

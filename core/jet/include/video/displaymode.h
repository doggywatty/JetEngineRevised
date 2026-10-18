// core/jet/include/video/displaymode.h
#ifndef CORE_JET_INCLUDE_VIDEO_DISPLAYMODE_H
#define CORE_JET_INCLUDE_VIDEO_DISPLAYMODE_H

#include "jenum.h"
#include "smath.h"

namespace jet::video {
struct DisplayMode {
    struct ColorDef { enum type { RGB565, RGBA8888 }; };
    struct DepthDef { enum type { None, Depth16, Depth24 }; };
    struct StencilDef { enum type { None, Stencil8 }; };
    struct AADef { enum type { None, MSAA2x, MSAA4x }; };

    DisplayMode() : size(0, 0), color(ColorDef::RGBA8888), depth(DepthDef::Depth24),
                    stencil(StencilDef::None), aa(AADef::None) {}
    math::vec2<unsigned int> size;
    safe_enum<ColorDef,ColorDef::type> color;
    safe_enum<DepthDef,DepthDef::type> depth;
    safe_enum<StencilDef,StencilDef::type> stencil;
    safe_enum<AADef,AADef::type> aa;
};
}

#endif

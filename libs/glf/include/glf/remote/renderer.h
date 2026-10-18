// libs/glf/include/glf/remote/renderer.h
#ifndef LIBS_GLF_INCLUDE_GLF_REMOTE_RENDERER_H
#define LIBS_GLF_INCLUDE_GLF_REMOTE_RENDERER_H

namespace glf::remote {
class Renderer {  // sizeof 0x4
public:
    Renderer();
    virtual ~Renderer();
    virtual void Begin2D(int, int) = 0;
    virtual void DrawPrimitive(glf::remote::Renderer::PrimitiveType, const float*, int, const glf::Vec4<float>&, float) = 0;
    virtual void End2D() = 0;
};
}

#endif

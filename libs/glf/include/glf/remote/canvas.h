// libs/glf/include/glf/remote/canvas.h
#ifndef LIBS_GLF_INCLUDE_GLF_REMOTE_CANVAS_H
#define LIBS_GLF_INCLUDE_GLF_REMOTE_CANVAS_H

namespace glf::remote {
class Canvas {  // sizeof 0x50
public:
    Canvas();
    glf::Point<float> ConvertPosScreen2Canvas(glf::Point<short>);
    glf::Point<float> ConvertLength2Size(float);
    void PostDrawShape(const glf::remote::Canvas::Shape&);
    void PostSignal(glf::remote::Canvas::Color, bool);
    void Update(glf::remote::Renderer*);
private:
    void cvBegin(glf::remote::Renderer::PrimitiveType);
    void cvColor(glf::remote::Canvas::Color, float);
    void cvOrientation(glf::remote::Canvas::OrientationType);
    void cvCircle2fv(const float*, const float*, bool);
    void cvCross2fv(const float*, const float*, bool);
    void cvDrawShape(const glf::remote::Canvas::Shape&, float);
    void cvEnd();
    void cvPoint2fv(const float*, bool);
    void cvRect2fv(const float*, const float*, bool);
    void cvTriangle2fv(const float*, const float*, bool);
    void cvVertex2f(float, float, float, float);
    std::list<glf::remote::Canvas::Shape,std::allocator<glf::remote::Canvas::Shape> > mShapes;  // 0x0
    float mLastUpdateTimeMS;  // 0x18
    glf::remote::Renderer *mCurrentRenderer;  // 0x1C
    glf::Vec4<float> mCurrentColor;  // 0x20
    float mCurrentOrientation;  // 0x30
    glf::remote::Renderer::PrimitiveType mCurrentType;  // 0x34
    unsigned mCurrentVertexCount;  // 0x38
    std::vector<glf::Vec4<float>,std::allocator<glf::Vec4<float> > > mCurrentVertices;  // 0x3C
};
}

#endif

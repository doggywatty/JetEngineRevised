// framework/include/lutinterpolator.h
#ifndef FRAMEWORK_INCLUDE_LUTINTERPOLATOR_H
#define FRAMEWORK_INCLUDE_LUTINTERPOLATOR_H

class LUTInterpolator {  // sizeof 0x5A0
public:
    LUTInterpolator(const math::vec2<unsigned int>&);
    void InterpolateTo(const boost::shared_ptr<jet::video::Texture>&, unsigned);
    void Interpolate(const boost::shared_ptr<jet::video::Texture>&, const boost::shared_ptr<jet::video::Texture>&, unsigned);
    const boost::shared_ptr<jet::video::Texture> &GetFinalLUT();
    void Process(unsigned);
protected:
    math::vec2<unsigned int> m_size;  // 0x0
    jet::video::Material m_blendMaterial;  // 0x8
    unsigned m_currentRenderTargetIndex;  // 0xF8
    boost::shared_ptr<jet::video::RenderTarget> m_renderTargets[2];  // 0xFC
    jet::video::Painter m_painter;  // 0x110
    jet::video::Painter::Vertex m_painterVertices[4];  // 0x528
    unsigned m_blendTime;  // 0x598
    unsigned m_blendInterval;  // 0x59C
};

#endif

// source/game/gamemanagers/PainterMgr.cpp
#include "PainterMgr.h"
#include "video/driver.h"

// @0x48a5c4: 5000 vertices per batch
PainterMgr::PainterMgr()
    : m_3dPainter(5000), m_2dPainter(5000), m_3dDebugPainter(5000), m_2dDebugPainter(5000), m_genericPainter(5000) {}

PainterMgr::~PainterMgr() {}

// @0x48a8c8: alpha blended unlit materials, "color3d" lit by depth for the 3D painter, "basic2d" for the screen ones
void PainterMgr::Init() {
    struct Setup { jet::video::Material &mat; jet::video::Painter &painter; const char *technique; bool cull, depth; };
    Setup setups[] = {
        { m_2dMaterial, m_2dPainter, "basic2d.xml", false, false },
        { m_3dMaterial, m_3dPainter, "color3d.xml", true, true },
        { m_2dDebugMaterial, m_2dDebugPainter, "basic2d.xml", false, false },
        { m_3dDebugMaterial, m_3dDebugPainter, "color3d.xml", false, false },
    };
    for (Setup &s : setups) {
        s.mat.SetColor(0xffffffff);
        s.mat.SetRenderTechnique(jet::String(s.technique));
        jet::video::RenderState &rs = s.mat.GetRenderState(0);
        rs.SetBlending(true);
        rs.SetBlendFormula(jet::video::BlendFormula(jet::video::BlendFormula::ComboDef::Alpha));
        rs.SetCulling(s.cull);
        rs.SetDepthTest(s.depth);
        rs.SetDepthWrite(false);
        s.painter.SetMaterial(s.mat);
    }
}

// @0x48abfc
void PainterMgr::Flush() {
    m_3dPainter.Flush();
    m_3dDebugPainter.Flush();
    m_2dPainter.Flush();
    m_2dDebugPainter.Flush();
    m_genericPainter.Flush();
    jet::video::Driver::GetCurrent()->Flush();
}

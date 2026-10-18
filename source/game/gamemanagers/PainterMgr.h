// source/game/gamemanagers/PainterMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_PAINTERMGR_H
#define SOURCE_GAME_GAMEMANAGERS_PAINTERMGR_H

#include "jsingleton.h"
#include "video/material.h"
#include "video/painter.h"

// The game's painters: scene 3D and GUI 2D, their debug twins and a generic one
class PainterMgr : public Singleton<PainterMgr> {  // sizeof 0x1554
public:
    PainterMgr();
    virtual ~PainterMgr();
    void Init();
    jet::video::Painter &Get3DPainter() { return m_3dPainter; }
    jet::video::Painter &Get2DPainter() { return m_2dPainter; }
    jet::video::Painter &Get3DDebugPainter() { return m_3dDebugPainter; }
    jet::video::Painter &Get2DDebugPainter() { return m_2dDebugPainter; }
    jet::video::Painter &GetGenericPainter() { return m_genericPainter; }
    void Flush();
private:
    jet::video::Painter m_3dPainter;  // 0x4
    jet::video::Painter m_2dPainter;  // 0x3B4
    jet::video::Painter m_3dDebugPainter;  // 0x764
    jet::video::Painter m_2dDebugPainter;  // 0xB14
    jet::video::Painter m_genericPainter;  // 0xEC4
    jet::video::Material m_3dMaterial;  // 0x1274
    jet::video::Material m_2dMaterial;  // 0x1364
    jet::video::Material m_3dDebugMaterial;  // 0x1454
    jet::video::Material m_2dDebugMaterial;  // 0x1544
};

#endif

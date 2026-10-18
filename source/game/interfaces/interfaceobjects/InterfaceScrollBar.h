// source/game/interfaces/interfaceobjects/InterfaceScrollBar.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACESCROLLBAR_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACESCROLLBAR_H

#include "InterfaceObject.h"

// The track (a frame repeated along the bar) and the thumb of a scrolled list
class InterfaceScrollBar : public InterfaceObject {  // sizeof 0x1CC
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceScrollBar();
    InterfaceScrollBar(AuroraLevelData *level, int id);
    virtual ~InterfaceScrollBar();
    void Init(math::vec2<float> pos, int sprite, int trackFrame, int thumbFrame, const math::vec2<float> &size);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset() {}
    // 0 at the start, 1 at the end of the track
    void SetScrollPos(float p) { m_scrollPos = p; }
    // @0x4c453c
    void SetValue(float v) { m_scrollPos = v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v; }
    void SetOpacity(float o) { m_opacity = o; }
    void SetHorizontal(bool h) { m_horizontal = h; }
    static const jet::String k_uiTmplName;
private:
    int m_spriteIdx;  // 0x1A4
    int m_trackFrame;  // 0x1A8
    int m_thumbFrame;  // 0x1AC
    float m_scrollPos;  // 0x1B0
    float m_opacity;  // 0x1B4: nothing is drawn at 0
    math::vec2<float> m_thumbSize;  // 0x1B8
    math::vec2<float> m_trackTileSize;  // 0x1C0
    bool m_horizontal;  // 0x1C8
};

#endif

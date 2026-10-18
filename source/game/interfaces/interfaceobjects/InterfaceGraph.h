// source/game/interfaces/interfaceobjects/InterfaceGraph.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEGRAPH_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEGRAPH_H

#include "InterfaceObject.h"
#include "sprites/sprite.h"
#include "sprites/spriteanim.h"

// A frame (or an animation) of one of the GUI sprites ("gui_graph_item_sprites")
class InterfaceGraph : public InterfaceObject {  // sizeof 0x1BC
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceGraph();
    InterfaceGraph(AuroraLevelData *level, int id);
    virtual ~InterfaceGraph();
    // frame < 0 plays the animation of that index instead
    void Init(math::vec2<float> pos, int sprite, int frame, int anim);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void SetAnim(int anim, bool loop);
    bool IsAnimEnded();
    int GetAnim() { return m_animIdx; }
    int GetSprite() { return m_spriteIdx; }
    int GetFrame() { return m_frameIdx; }
    int GetCurrentAFrame();
    void SetFrame(int frame);
    void ResetAnim();
    void SetColor(unsigned argb) { m_color = argb; }
    static const jet::String k_uiTmplName;
private:
    int m_spriteIdx;  // 0x1A4
    int m_frameIdx;  // 0x1A8
    int m_animIdx;  // 0x1AC
    SpriteAnim *m_anim;  // 0x1B0
    safe_enum<Sprite::SpriteBlendingDef, Sprite::SpriteBlendingDef::type> m_blending;  // 0x1B4
    unsigned m_color;  // 0x1B8: 0xAARRGGBB tint, white by default
};

#endif

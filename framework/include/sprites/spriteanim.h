// framework/include/sprites/spriteanim.h
#ifndef FRAMEWORK_INCLUDE_SPRITES_SPRITEANIM_H
#define FRAMEWORK_INCLUDE_SPRITES_SPRITEANIM_H

#include "jenum.h"
#include "smath.h"
#include "video/painter.h"
#include "sprites/sprite.h"

// plays one Aurora animation of a sprite: times are in 1/30 s frame units
class SpriteAnim {  // sizeof 0x28
public:
    SpriteAnim(Sprite*);
    ~SpriteAnim();
    void SetAnim(int, bool loop);
    void ResetAnim();
    void UpdateAnim(int ms);
    void RenderAnim(jet::video::Painter&, const math::mat3<float>&);
    void RenderAnim(jet::video::Painter&, math::vec2<float>);
    void RenderAnim(jet::video::Painter&);
    void RenderAnimOffset(jet::video::Painter&, float, float);
    void SetSprite(Sprite*);
    void SetPos(float x, float y) { m_posX = x; m_posY = y; }
    void GetPos(float &x, float &y) const { x = m_posX; y = m_posY; }
    bool IsAnimEnded() const { return m_time <= m_lastTime; }
    int GetAnimId() const { return m_animId; }
    int GetAframe() const { return m_aFrame; }
    void SetBlend(safe_enum<Sprite::SpriteBlendingDef,Sprite::SpriteBlendingDef::type> b) { m_blend = b; }
    void SetLoop(bool l) { m_loop = l; }
    Sprite *GetSprite() const { return m_sprite; }
private:
    Sprite *m_sprite;      // 0x0
    int m_animId;          // 0x4
    float m_time;          // 0x8
    float m_lastTime;      // 0xC
    float m_aFrameTime;    // 0x10
    int m_aFrame;          // 0x14
    bool m_loop;           // 0x18
    float m_posX;          // 0x1C
    float m_posY;          // 0x20
    safe_enum<Sprite::SpriteBlendingDef,Sprite::SpriteBlendingDef::type> m_blend;  // 0x24
};

#endif

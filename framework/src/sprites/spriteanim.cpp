// framework/src/sprites/spriteanim.cpp
#include "sprites/spriteanim.h"

SpriteAnim::SpriteAnim(Sprite *s) : m_sprite(s), m_animId(-1), m_time(0), m_lastTime(-1.0f), m_aFrameTime(0), m_aFrame(0),
    m_loop(false), m_posX(0), m_posY(0), m_blend(Sprite::SpriteBlendingDef::Alpha) {}
SpriteAnim::~SpriteAnim() { m_sprite = 0; }

// @0x577d30
void SpriteAnim::SetSprite(Sprite *s) {
    m_sprite = s; m_animId = -1; m_time = 0; m_lastTime = -1.0f; m_aFrameTime = 0; m_aFrame = 0;
    m_loop = false; m_posX = 0; m_posY = 0; m_blend = Sprite::SpriteBlendingDef::Alpha;
}

// @0x577d70
void SpriteAnim::SetAnim(int id, bool loop) {
    if (id >= 0 && id < m_sprite->GetNumberOfAnims()) {
        m_loop = loop; m_time = 0; m_lastTime = -1.0f; m_aFrame = 0; m_aFrameTime = 0;
    } else
        id = -1;
    m_animId = id;
}

void SpriteAnim::ResetAnim() { m_time = 0; m_aFrameTime = 0; m_lastTime = -1.0f; m_aFrame = 0; }

// @0x577de0
void SpriteAnim::UpdateAnim(int ms) {
    if (m_animId < 0) return;
    m_lastTime = m_time;
    const float dt = (float)ms * 0.033333335f;
    m_time += dt;
    m_aFrameTime += dt;
    float ft = (float)m_sprite->GetAFrameTime(m_animId, m_aFrame);
    for (;;) {
        if (m_aFrameTime < ft) return;
        if (!m_loop && m_aFrameTime == ft) return;
        m_aFrameTime -= ft;
        int prev = m_aFrame++;
        if (m_aFrame < m_sprite->GetNumberOfAnimationFrames(m_animId)) {
            ft = (float)m_sprite->GetAFrameTime(m_animId, m_aFrame);
        } else if (m_loop) {
            m_aFrame = 0;
            m_time = m_aFrameTime;
            ft = (float)m_sprite->GetAFrameTime(m_animId, 0);
        } else {
            m_time -= m_aFrameTime;
            m_lastTime = m_time;
            m_aFrame = prev;
            m_aFrameTime = ft;
        }
    }
}

// @0x577f24
void SpriteAnim::RenderAnim(jet::video::Painter &p, const math::mat3<float> &m) {
    if (m_animId < 0) return;
    Sprite::SetBlending(m_blend);
    m_sprite->PaintAFrame(p, (unsigned)m_animId, (unsigned)m_aFrame, m);
    Sprite::SetBlending(Sprite::SpriteBlendingDef::Alpha);
}

// @0x577edc
void SpriteAnim::RenderAnim(jet::video::Painter &p, math::vec2<float> pos) {
    math::mat3<float> m;
    m.m[6] = pos.x;
    m.m[7] = pos.y;
    RenderAnim(p, m);
}

void SpriteAnim::RenderAnim(jet::video::Painter &p) { RenderAnim(p, math::vec2<float>(m_posX, m_posY)); }
void SpriteAnim::RenderAnimOffset(jet::video::Painter &p, float x, float y) { RenderAnim(p, math::vec2<float>(x + m_posX, y + m_posY)); }

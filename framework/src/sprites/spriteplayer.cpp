// framework/src/sprites/spriteplayer.cpp
#include "sprites/sprite.h"
#include "sprites/spritemgr.h"
#include "sprites/spriteplayer.h"
#include <cmath>

const int SpritePlayer::k_invalidIdx = -1;

// @0x642410
void SpritePlayer::Init(float x, float y, float z) {
    m_position = math::vec3<float>(x, y, z);
    m_scale = math::vec3<float>(1, 1, 1);
    m_currAnim = -1;
    m_currentColor = jet::video::Color(1, 1, 1, 1);
    m_animLoop = false;
    m_animFinished = false;
    m_currFrame = 0;
    m_currFrameTime = 0;
    m_flags = 0;
}

// @0x6422e8
SpritePlayer::SpritePlayer(const jet::String &sprite, float x, float y, float z)
    : m_position(0, 0, 0), m_scale(0, 0, 0), m_rotation(0, 0, 0, 1), m_sprite(0) {
    Init(x, y, z);
    SetSprite(sprite);
}

// @0x642380
SpritePlayer::SpritePlayer(Sprite *sprite, float x, float y, float z)
    : m_position(0, 0, 0), m_scale(0, 0, 0), m_rotation(0, 0, 0, 1), m_sprite(0) {
    Init(x, y, z);
    m_sprite = sprite;
}

// @0x642474
SpritePlayer::~SpritePlayer() {}

void SpritePlayer::SetPosition(const math::vec3<float> &p) { m_position = p; }
void SpritePlayer::SetPosition(float x, float y, float z) { m_position = math::vec3<float>(x, y, z); }
const math::vec3<float> &SpritePlayer::GetPosition() { return m_position; }
void SpritePlayer::SetRotation(const math::quat<float> &q) { m_rotation = q; }
void SpritePlayer::SetScale(const math::vec3<float> &s) { m_scale = s; }
// @0x642aac: the sprite (the current animation's first frame, or the frame) is scaled to fit the rectangle, no more than
// maxScale, and centered in it
void SpritePlayer::PutIntoRect(const math::rect<float> &r, float maxScale) {
    math::rect<float> fr(0, 0, 0, 0);
    math::mat3<float> m;
    m.setIdentity();
    if (m_currAnim != -1) m_sprite->GetAFrameRect(fr, (unsigned)m_currAnim, 0, m);
    else m_sprite->GetFrameRect(fr, (unsigned)m_currFrame, m);
    float sx = (r.lowerRightCorner.x - r.upperLeftCorner.x) / (fr.lowerRightCorner.x - fr.upperLeftCorner.x);
    float sy = (r.lowerRightCorner.y - r.upperLeftCorner.y) / (fr.lowerRightCorner.y - fr.upperLeftCorner.y);
    float s = sy > sx ? sx : sy;
    if (s > maxScale) s = maxScale;
    m_scale = math::vec3<float>(s, s, s);
    m_position.y = (r.lowerRightCorner.y + r.upperLeftCorner.y) * 0.5f + (fr.upperLeftCorner.y + fr.lowerRightCorner.y) * -0.5f * s;
    m_position.x = (r.lowerRightCorner.x + r.upperLeftCorner.x) * 0.5f + (fr.upperLeftCorner.x + fr.lowerRightCorner.x) * -0.5f * s;
}
void SpritePlayer::SetScale(float s) { m_scale = math::vec3<float>(s, s, s); }
const math::vec3<float> &SpritePlayer::GetScale() { return m_scale; }
void SpritePlayer::SetCurrentColor(jet::video::Color c) { m_currentColor = c; }
jet::video::Color SpritePlayer::GetCurrentColor() { return m_currentColor; }
Sprite *SpritePlayer::GetSprite() { return m_sprite; }
const Sprite *SpritePlayer::GetSprite() const { return m_sprite; }

// @0x6428f8
void SpritePlayer::SetSprite(const jet::String &name) { m_sprite = SpriteMgr::GetInstance()->LoadSprite(name, false); }
// @0x64292c
void SpritePlayer::SetSprite(Sprite *s) { m_sprite = s; }

// @0x642880
void SpritePlayer::SetFrame(int f) {
    m_currFrame = f;
    m_currAnim = -1;
    m_currFrameTime = 0;
}
// @0x642898
void SpritePlayer::SetAnim(int anim, bool loop) {
    m_currAnim = anim;
    m_animLoop = loop;
    ResetAnim();
}
// @0x6428a4
void SpritePlayer::ResetAnim() {
    m_currFrame = 0;
    m_currFrameTime = 0;
    m_animFinished = false;
}
int SpritePlayer::GetAnimFrame() { return m_currFrame; }
int SpritePlayer::GetAnimFrameTime() { return m_currFrameTime; }
void SpritePlayer::SetAnimFrame(int f) {
    m_currFrame = f;
    m_currFrameTime = 0;
    m_animFinished = false;
}
void SpritePlayer::SetAnimFrameTime(int t) { m_currFrameTime = t; }
int SpritePlayer::GetNumberOfAnimationFrames(int anim) { return m_sprite->GetNumberOfAnimationFrames(anim); }
int SpritePlayer::GetAnim() { return m_currAnim; }
int SpritePlayer::GetFrame() { return m_currFrame; }
bool SpritePlayer::IsAnimFinished() { return m_animFinished; }
bool SpritePlayer::IsAnimLooped() { return m_animLoop; }

// @0x6424a8: the time of an animation frame is its ticks of 33.33 ms; a finished animation stays on its last frame
void SpritePlayer::Update(int dt) {
    if (!m_sprite || m_currAnim == -1) return;
    const float k = 33.333332f;
    int t = (int)((float)m_sprite->GetAFrameTime(m_currAnim, m_currFrame) * k);
    while (t != 0 && m_currFrameTime >= t) {
        m_currFrameTime -= t;
        if (m_currFrame < m_sprite->GetNumberOfAnimationFrames(m_currAnim) - 1) m_currFrame++;
        else if (!m_animLoop) {
            m_animFinished = true;
            break;
        } else
            m_currFrame = 0;
        t = (int)((float)m_sprite->GetAFrameTime(m_currAnim, m_currFrame) * k);
    }
    if (m_animFinished) {
        // @0x642554 is reached with the time still adding up
    }
    m_currFrameTime += dt;
}

// @0x64259c
void SpritePlayer::Render(jet::video::Painter &p) { Render(p, m_position, m_scale, m_rotation); }

// @0x642610: the rotation, scale and position as the painter's transform, the sprite in its own color and alpha blended
void SpritePlayer::Render(jet::video::Painter &p, math::vec3<float> pos, math::vec3<float> scale, math::quat<float> q) {
    const jet::video::Color saved = m_sprite->GetCurrentColor();
    const float x = q.x, y = q.y, z = q.z, w = q.w;
    math::mat4<float> m;
    m.setIdentity();
    m.m[0] = 1 - 2 * y * y - 2 * z * z;
    m.m[1] = 2 * x * y - 2 * z * w;
    m.m[2] = 2 * y * w + 2 * x * z;
    m.m[4] = 2 * z * w + 2 * x * y;
    m.m[5] = 1 - 2 * z * z - 2 * x * x;
    m.m[6] = 2 * y * z - 2 * x * w;
    m.m[8] = 2 * x * z - 2 * y * w;
    m.m[9] = 2 * y * z + 2 * x * w;
    m.m[10] = 1 - 2 * y * y - 2 * x * x;
    m.m[12] = pos.x;
    m.m[13] = pos.y;
    m.m[14] = pos.z;
    const float eps = 1.1920929e-07f;
    if (std::fabs(scale.x - 1) > eps || std::fabs(scale.y - 1) > eps || std::fabs(scale.z - 1) > eps) {
        m.m[0] *= scale.x; m.m[1] *= scale.x; m.m[2] *= scale.x;
        m.m[4] *= scale.y; m.m[5] *= scale.y; m.m[6] *= scale.y;
        m.m[8] *= scale.z; m.m[9] *= scale.z; m.m[10] *= scale.z;
    }
    p.SetTransform(m);
    m_sprite->SetCurrentColor(m_currentColor);
    Sprite::SetBlending(Sprite::SpriteBlendingDef::Alpha);
    math::mat3<float> id;
    if (m_currAnim != -1) m_sprite->PaintAFrame(p, (unsigned)m_currAnim, (unsigned)m_currFrame, id);
    else m_sprite->PaintFrame(p, (unsigned)m_currFrame, id);
    m_sprite->SetCurrentColor(saved);
    math::mat4<float> ident;
    ident.setIdentity();
    p.SetTransform(ident);
}

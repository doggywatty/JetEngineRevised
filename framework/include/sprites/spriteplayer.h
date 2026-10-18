// framework/include/sprites/spriteplayer.h
#ifndef FRAMEWORK_INCLUDE_SPRITES_SPRITEPLAYER_H
#define FRAMEWORK_INCLUDE_SPRITES_SPRITEPLAYER_H

class SpritePlayer {  // sizeof 0x4C
public:
    SpritePlayer(Sprite*, float, float, float);
    SpritePlayer(const jet::String&, float, float, float);
    ~SpritePlayer();
    void SetPosition(const math::vec3<float>&);
    void SetPosition(float, float, float);
    const math::vec3<float> &GetPosition();
    void SetRotation(const math::quat<float>&);
    void SetRotation(float);
    math::quat<float> GetRotation();
    float GetRotationAngle();
    void SetScale(const math::vec3<float>&);
    void SetScale(float);
    const math::vec3<float> &GetScale();
    void PutIntoRect(const math::rect<float>&, float);
    void SetAlpha(float);
    float GetAlpha();
    void SetSprite(const jet::String&);
    void SetSprite(Sprite*);
    const Sprite *GetSprite() const;
    Sprite *GetSprite();
    void Update(int);
private:
    void Render(jet::video::Painter&, math::vec3<float>, math::vec3<float>, math::quat<float>);
public:
    void Render(jet::video::Painter&);
    void SetFlags(int);
    int GetFlags();
    void DisableFlag(int);
    void EnableFlag(int);
    void SetFrame(int);
    void SetAnim(int, bool);
    int GetAnimFrame();
    void SetAnimFrame(int);
    int GetAnimFrameTime();
    void SetAnimFrameTime(int);
    void ResetAnim();
    int GetAnim();
    int GetFrame();
    int GetNumberOfAnimationFrames(int);
    void SetAnimLooped(bool);
    bool IsAnimLooped();
    bool IsAnimValid();
    bool IsAnimFinished();
    void SetRenderTechnique(jet::video::RenderTechnique*);
    void SetRenderTechnique(const jet::String&);
    void SetCurrentColor(jet::video::Color);
    jet::video::Color GetCurrentColor();
private:
    void Init(float, float, float);
public:
    static const int k_invalidIdx;
private:
    math::vec3<float> m_position;  // 0x4
    math::vec3<float> m_scale;  // 0x10
    math::quat<float> m_rotation;  // 0x1C
    jet::video::Color m_currentColor;  // 0x2C
    bool m_animLoop;  // 0x30
    bool m_animFinished;  // 0x31
    int m_currAnim;  // 0x34
    int m_currFrame;  // 0x38
    int m_currFrameTime;  // 0x3C
    int m_flags;  // 0x40
    Sprite *m_sprite;  // 0x44
    jet::String m_renderTechniqueName;  // 0x48
};

#endif

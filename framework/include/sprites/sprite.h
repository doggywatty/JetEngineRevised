// framework/include/sprites/sprite.h
#ifndef FRAMEWORK_INCLUDE_SPRITES_SPRITE_H
#define FRAMEWORK_INCLUDE_SPRITES_SPRITE_H

#include "sprites/asprite.h"
#include "jenum.h"
#include "video/material.h"
#include "video/painter.h"

// Aurora sprite painted through the Painter with the "sprite2d" technique (Sprite @0x640564).
class Sprite : public ASprite {
public:
    struct SpriteBlendingDef {
        // _PaintPrepareRender @0x640e84: 0 alpha, 1/2 additive, 3 multiply, 4 opaque
        enum type { Alpha, Additive, Additive2, Multiply, Opaque };
    };
    typedef safe_enum<SpriteBlendingDef, SpriteBlendingDef::type> SpriteBlending;

    Sprite();
    virtual ~Sprite();
    virtual void Unload();
    // Load @0x6407f8: "<name>.bsprite", texture "sprite_<image>" (or "sprite_<name>.tga") and its
    // optional "__alpha" companion
    bool Load(const jet::String &name, bool canLoadWithoutTGA);
    static Sprite *LoadSprite(const jet::String &name, bool canLoadWithoutTGA);

    static void SetBlending(SpriteBlending b) { s_crtBlending = b; }
    static SpriteBlending GetBlending() { return s_crtBlending; }
    static jet::video::Material &GetMaterial();
    // @0x5795dc: the technique every sprite paints with ("sprite2d.xml" again for an empty name)
    static void SetOverridenRenderTechnique(const jet::String&);
    void SetPositionScale(float s) { m_posScale = s; }
    float GetPositionScale() const { return m_posScale; }
    void SetTextureScale(float s) { m_textureScale = s; }
    static void SetDefaultPositionScale(float s) { s_posScale = s; }
    static float GetDefaultPositionScale() { return s_posScale; }
    void SetTempMaterial(jet::video::Material *m) { m_tempMaterial = m; }
    // @0x63ac2c: the logical rect in painted pixels (scaled by the position scale)
    virtual bool GetFrameLogicalRect(int frame, int idx, math::vec2<float> &pos, math::vec2<float> &size) const;

    void PaintAnim(jet::video::Painter&, unsigned anim, int timeMs, const math::mat3<float>&);
    void PaintAnim(jet::video::Painter&, unsigned anim, int timeMs, const math::vec2<float>&);
    void PaintAFrame(jet::video::Painter&, unsigned anim, unsigned aframe, const math::mat3<float>&);
    void PaintAFrame(jet::video::Painter&, unsigned anim, unsigned aframe, const math::vec2<float>&);
    void PaintFrame(jet::video::Painter&, unsigned frame, const math::mat3<float>&);
    void PaintFrame(jet::video::Painter&, unsigned frame, const math::vec2<float>&);
    void PaintFModule(jet::video::Painter&, unsigned frame, unsigned fmodule, const math::mat3<float>&);
    void PaintModule(jet::video::Painter&, unsigned module, const math::mat3<float>&);
    void PaintModule(jet::video::Painter&, unsigned module, const math::vec2<float>&);

    // @0x58a034: the radial progress bar shader's "size" and "initial" (the module in the texture) and the angles it fills between
    void UpdateProgressBarUniforms(unsigned module, float minAngle, float maxAngle);
    void _PaintPrepareRender(jet::video::Painter&);
    // @0x640b90: the frame's modules, between _PaintPrepareRender and _PaintRenderDone
    void _PaintFrame(jet::video::Painter&, unsigned, const math::mat3<float>&);
    // @0x4bfdcc: the textures the prepare put in the sprite's material go
    void _PaintRenderDone();

private:
    void _PaintAnim(jet::video::Painter&, unsigned, int, const math::mat3<float>&);
    void _PaintAFrame(jet::video::Painter&, unsigned, unsigned, const math::mat3<float>&);
    void _PaintFModule(jet::video::Painter&, unsigned, unsigned, const math::mat3<float>&);
    void _PaintModule(jet::video::Painter&, unsigned, const math::mat3<float>&);

    static SpriteBlending s_crtBlending;
    static float s_posScale;
    static float s_textureScale;
    float m_posScale;
    float m_textureScale;
    math::vec2<float> m_cachedInvTexSize;
    bool m_canLoadWithoutTGA;
    jet::video::Material *m_tempMaterial;
};

#endif

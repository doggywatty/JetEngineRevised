// source/game/interfaces/interfaceobjects/InterfaceButton.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEBUTTON_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEBUTTON_H

#include "InterfaceObject.h"
#include "sprites/spriteanim.h"
#include "text2/texter2.h"

// A pressable sprite with a text and an optional icon frame; its state follows the touches on it
class CostumeShopItem;
class ShopBonusUpgradeItemBox;

class InterfaceButton : public InterfaceObject {  // sizeof 0x254
    friend class ShopBonusUpgradeItemBox;  // moves m_frameTextOff of its price button
    friend class CostumeShopItem;  // keeps a mark in m_fromAurora and moves m_frameTextOff of its price button
public:
    enum State { Untouched = -1, Idle = 0, Pressed = 1, Released = 2 };
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceButton();
    InterfaceButton(AuroraLevelData *level, int id);
    virtual ~InterfaceButton();
    void Init(math::vec2<float> pos, int sprite, int idleAnim, int pressedAnim, int selectedAnim, const jet::String &text, int font, int palIdle,
              int palPressed, int palSelected, int iconPos, int iconIdleAnim, int iconPressedAnim, int iconSelectedAnim, int vAlign, int hAlign,
              int textOffset, bool wrap, bool scaleToWidth, int shadow, int outline, int strikeout, int underline, bool clipText);
    virtual void Init();
    void InitFrom(InterfaceObject *like);
    void InitFrom(InterfaceObject *parent, const math::vec2<float> &pos, const math::vec2<float> &size);
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset();
    int GetState() { return m_state; }
    bool IsPressed() { return m_state == Pressed; }
    bool IsReleased() { return m_state == Released; }
    void SetSelected(bool s) { m_selected = s; }
    bool IsSelected() { return m_selected; }
    void SetText(const jet::String&);
    void SetText(const char*);
    const jet::String &GetText() { return m_text; }
    void ResetTextToDefault();
    void SetIdleAnim(int);
    void SetPressedAnim(int);
    void SetSelectedAnim(int);
    void SetAnimLoop(bool);
    void SetColor(unsigned argb) { m_color = argb; }
    math::rect<float> GetInputRect() { return m_inputRect; }
    int GetSpriteIdx() { return m_spriteIdx; }
    int GetIdleAnim() { return m_idleAnimId; }
    int GetPressedAnim() { return m_pressedAnimId; }
    int GetSelectedAnim() { return m_selectedAnimId; }
    static const jet::String k_uiTmplName;
private:
    void UpdateTouches();
    void SetupFontStyle(int palette, int alpha);
    static jet::text2::Texter::Style s_fontStyle;
    bool m_fromAurora;  // 0x1A4
    int m_shadowThickness;  // 0x1A8
    int m_outlineThickness;  // 0x1AC
    int m_strikeoutThickness;  // 0x1B0
    int m_underlineThickness;  // 0x1B4
    int m_state;  // 0x1B8
    int m_currTouchID;  // 0x1BC
    int m_spriteIdx;  // 0x1C0
    int m_idleAnimId;  // 0x1C4
    int m_pressedAnimId;  // 0x1C8
    int m_selectedAnimId;  // 0x1CC
    int m_fontId;  // 0x1D0
    int m_palIdle;  // 0x1D4
    int m_palPressed;  // 0x1D8
    int m_palSelected;  // 0x1DC
    int m_iconPos;  // 0x1E0: -1 none, 0 left of the text
    int m_iconIdleAnim;  // 0x1E4
    int m_iconPressedAnim;  // 0x1E8
    int m_iconSelectedAnim;  // 0x1EC
    int m_vAlign;  // 0x1F0
    int m_hAlign;  // 0x1F4
    int m_textOffset;  // 0x1F8
    bool m_enableTextClip;  // 0x1FC
    math::vec2<float> m_frameTextOff;  // 0x200
    math::vec2<float> m_frameTextSize;  // 0x208
    SpriteAnim *m_idleAnim;  // 0x210
    SpriteAnim *m_pressedAnim;  // 0x214
    SpriteAnim *m_selectedAnim;  // 0x218
    bool m_selected;  // 0x21C
    math::vec2<float> m_iconSize;  // 0x220
    jet::String m_auroraText;  // 0x228
    jet::String m_text;  // 0x22C
    bool m_enableTextWrap;  // 0x230
    bool m_enableTextScaleToWidth;  // 0x231
    float m_autoScrollText_animScale;  // 0x234
    unsigned m_color;  // 0x238: 0xAARRGGBB
    math::rect<float> m_inputRect;  // 0x23C
    math::vec2<float> m_textScale;  // 0x24C
};

#endif

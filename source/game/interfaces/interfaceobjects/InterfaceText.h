// source/game/interfaces/interfaceobjects/InterfaceText.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACETEXT_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACETEXT_H

#include "InterfaceObject.h"
#include "text2/texter2.h"

// A text of the GUI: a localization key (resolved again on Reset) or a literal, in one of the GUI fonts
class InterfaceText : public InterfaceObject {  // sizeof 0x1F8
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceText();
    InterfaceText(AuroraLevelData *level, int id);
    virtual ~InterfaceText();
    void Init(math::vec2<float> pos, int w, int h, int font, const jet::String &text, int vAlign, int hAlign, int palette, float rotationRad,
              int shadow, int outline, int strikeout, int underline, bool wrap, bool scaleToWidth, bool clipToBox);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset();
    void SetText(const char*);
    void SetText(const jet::String&);
    const jet::String &GetText() { return m_text; }
    void SetAlpha(int a) { m_alpha = a; }
    void SetPal(int p) { m_palIdx = p; }
    // the text is drawn at this scale instead of the one that fits its box (when positive)
    void SetFixedScale(float s) { m_fixedScale = s; }
    int GetFontIdx() { return m_fontIdx; }
    int GetPalIdx() { return m_palIdx; }
    math::vec2<float> GetTextSize();
    void EnableAutoScroll(bool e) { m_enableAutoScroll = e; }
    static const jet::String k_uiTmplName;
private:
    void SetupFontStyle();
    static jet::text2::Texter::Style s_fontStyle;
    bool m_wrapAtDefaultWidth;  // 0x1A4
    int m_fontIdx;  // 0x1B8
    int m_vAlign;  // 0x1BC: 0 top, 1 middle, 2 bottom
    int m_hAlign;  // 0x1C0: 0 left, 1 center, 2 right
    int m_palIdx;  // 0x1C4
    int m_shadowThickness;  // 0x1C8
    int m_outlineThickness;  // 0x1CC
    int m_strikeoutThickness;  // 0x1D0
    int m_underlineThickness;  // 0x1D4
    bool m_enableTextClip;  // 0x1D8
    int m_alpha;  // 0x1DC
    bool m_enableWrap;  // 0x1E0
    bool m_enableScaleToWidth;  // 0x1E1
    jet::String m_auroraText;  // 0x1E4: the key
    jet::String m_text;  // 0x1E8: the shown text
    float m_autoScrollText_animScale;  // 0x1EC
    float m_fixedScale;  // 0x1F0: text scale instead of the computed one when positive
    bool m_enableAutoScroll;  // 0x1F4
};

#endif

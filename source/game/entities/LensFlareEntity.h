// source/game/entities/LensFlareEntity.h
#ifndef SOURCE_GAME_ENTITIES_LENSFLAREENTITY_H
#define SOURCE_GAME_ENTITIES_LENSFLAREENTITY_H

#include "GameEntity.h"

class LensFlareDef;

// A lens flare placed in the data: which LensFlareDef it shows and from how far (squared) it can be seen
class LensFlareEntity : public GameEntity {  // sizeof 0xB0
public:
    static jet::String k_tmplName;
    explicit LensFlareEntity(const clara::Template *t);
    virtual void Init();
    virtual void PostInit();
    virtual void NotifyParamChanged(const jet::String&);
    virtual void SetEnabled(bool b) { m_enabled = b; }
    virtual void SetVisible(bool b) { m_visible = b; }
    // @0x57183c: the flare manager sets it when the light is in sight
    void SetRenderable(bool b) { m_renderable = b; }
    // @0x571844
    bool IsVisible() const { return m_visible ? m_renderable : false; }
    bool IsEnabled() const { return m_enabled; }
    LensFlareDef *GetDef() const { return m_def; }
    float GetMinDistSq() const { return m_minDistSq; }
private:
    LensFlareDef *m_def;  // 0x8C
    bool m_enabled;       // 0x90
    bool m_renderable;    // 0x91
    bool m_visible;       // 0x92
    // 0x94 .. 0xA8: a fade (from, current, to, time 5000, elapsed, fading) set up like the flare manager's and never used
    float m_fadeFrom, m_fadeCurrent, m_fadeTo;  // 0x94 0x98 0x9C
    int m_fadeTime, m_fadeElapsed;              // 0xA0 0xA4
    bool m_fading;                              // 0xA8
    float m_minDistSq;                          // 0xAC
};

#endif

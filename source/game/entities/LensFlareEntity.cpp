// source/game/entities/LensFlareEntity.cpp
#include "LensFlareEntity.h"
#include "LensFlareDef.h"
#include "param.h"

jet::String LensFlareEntity::k_tmplName("LensFlare");

// @0x571508
LensFlareEntity::LensFlareEntity(const clara::Template *t)
    : GameEntity(t), m_def(0), m_enabled(true), m_renderable(false), m_visible(true), m_fadeFrom(0), m_fadeCurrent(0), m_fadeTo(0), m_fadeTime(5000),
      m_fadeElapsed(0), m_fading(false), m_minDistSq(10000.0f) {}

// @0x5715d8: "Enable", "MinDist" (kept squared) and the "FlareDef" it shows
void LensFlareEntity::Init() {
    GameEntity::Init();
    if (HasParam(jet::String("Enable"))) GetParam(jet::String("Enable"), m_enabled);
    if (HasParam(jet::String("MinDist"))) {
        float d;
        GetParam(jet::String("MinDist"), d);
        m_minDistSq = d * d;
    }
    const clara::Param *p = FindParamByName(jet::String("FlareDef"));
    m_def = static_cast<LensFlareDef*>(clara::Project::GetInstance()->FindEntityByPath(p ? p->GetAsPath(0) : clara::Path()));
}

// @0x571768
void LensFlareEntity::PostInit() { GameEntity::PostInit(); }

// @0x57176c
void LensFlareEntity::NotifyParamChanged(const jet::String &n) {
    if (n.Equals("Enable")) GetParam(n, m_enabled);
    if (n.Equals("MinDist")) {
        float d;
        GetParam(n, d);
        m_minDistSq = d * d;
    }
}

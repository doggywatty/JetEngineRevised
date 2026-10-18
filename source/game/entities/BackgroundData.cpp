// source/game/entities/BackgroundData.cpp
#include "BackgroundData.h"
#include "param.h"
#include "project.h"
#include "clarafile.h"
#include "MaterialDef.h"
#include "game/gameenums.h"

const jet::String BackgroundData::k_tmplName("BackgroundData"), BackgroundData::k_fogEnabled("FogEnabled"), BackgroundData::k_fogStartDepth("FogStartDepth"),
    BackgroundData::k_fogEndDepth("FogEndDepth"), BackgroundData::k_fogColor("FogColor"), BackgroundData::k_postFx("PostFX"),
    BackgroundData::k_ambienceSoundEnabledParam("AmbienceSoundEnabled"), BackgroundData::k_ambienceSoundParam("AmbienceSound"),
    BackgroundData::k_musicEnabledParam("MusicEnabled"), BackgroundData::k_musicLabelParam("MusicLabel"),
    BackgroundData::k_useSoftMusicParam("UseSoftMusic"), BackgroundData::k_gameplaySoundsGainParam("GameplaySoundsGain"),
    BackgroundData::k_locationParam("Location"), BackgroundData::k_backgroundTitleParam("LocationTitle"), BackgroundData::k_lensFlare("LensFlare");

// @0x53758c
void BackgroundData::Init() {
    clara::DataEntity::Init();
    GetParam(k_fogEnabled, m_fogEnabled);
    if (m_fogEnabled) {
        GetParam(k_fogStartDepth, m_fogStart);
        GetParam(k_fogEndDepth, m_fogEnd);
        math::vec4<float> c(0, 0, 0, 0);
        GetParam(k_fogColor, c);
        m_fogColor = jet::video::Color(c.x, c.y, c.z, c.w);
    }
    clara::Project *p = clara::Project::GetInstance();
    const clara::Param *fx = FindParamByName(k_postFx);
    m_postFx = static_cast<MaterialDef*>(p->FindEntityByPath(fx ? fx->GetAsPath(0) : clara::Path()));
    m_ambienceSound = jet::String();
    GetParam(k_ambienceSoundEnabledParam, m_ambienceEnabled);
    if (m_ambienceEnabled) GetParam(k_ambienceSoundParam, m_ambienceSound);
    m_musicLabel = jet::String();
    GetParam(k_musicEnabledParam, m_musicEnabled);
    if (m_musicEnabled) {
        GetParam(k_musicLabelParam, m_musicLabel);
        GetParam(k_useSoftMusicParam, m_useSoftMusic);
    }
    GetParam(k_gameplaySoundsGainParam, m_gameplaySoundsGain);
    m_fileName = GetClaraFile()->GetName();
    GetParam(k_locationParam, m_location);
    m_name = m_fileName.substr(11);
    m_locationName = LocationIdToString(ELocation(m_location));
    GetParam(k_backgroundTitleParam, m_title);
    m_lensFlare = 0;
    const clara::Param *lf = FindParamByName(k_lensFlare);
    if (lf) m_lensFlare = p->FindEntityByPath(lf->GetAsPath(0));
}

// @0x537880
bool BackgroundData::UseFog(jet::video::Color *color, float *start, float *end) const {
    if (m_fogEnabled) {
        *start = m_fogStart;
        *end = m_fogEnd;
        *color = m_fogColor;
    }
    return m_fogEnabled;
}

// @0x5378ac
bool BackgroundData::UseSound(jet::String *label) const {
    if (m_ambienceEnabled) *label = m_ambienceSound;
    return m_ambienceEnabled;
}

// @0x5378d4
bool BackgroundData::UseMusic(jet::String *label) const {
    if (m_musicEnabled) *label = m_musicLabel;
    return m_musicEnabled;
}

// @0x5378fc
bool BackgroundData::UsePostFx(MaterialDef **fx) const {
    if (m_postFx) *fx = m_postFx;
    return m_postFx != 0;
}

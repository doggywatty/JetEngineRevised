// source/game/entities/BackgroundData.h
#ifndef SOURCE_GAME_ENTITIES_BACKGROUNDDATA_H
#define SOURCE_GAME_ENTITIES_BACKGROUNDDATA_H

#include "entity.h"
#include "video/color.h"

class MaterialDef;

// What the data says of a background: fog, post effect, lens flare, sounds and its location
class BackgroundData : public clara::DataEntity {  // sizeof 0x64
public:
    static const jet::String k_tmplName, k_fogEnabled, k_fogStartDepth, k_fogEndDepth, k_fogColor, k_postFx, k_ambienceSoundEnabledParam,
        k_ambienceSoundParam, k_musicEnabledParam, k_musicLabelParam, k_useSoftMusicParam, k_gameplaySoundsGainParam, k_locationParam,
        k_backgroundTitleParam, k_lensFlare;
    explicit BackgroundData(const clara::Template *t) : clara::DataEntity(t), m_postFx(0), m_lensFlare(0), m_fogEnabled(false), m_fogStart(0), m_fogEnd(0),
        m_ambienceEnabled(false), m_musicEnabled(false), m_useSoftMusic(false), m_gameplaySoundsGain(0), m_location(-1) {}
    virtual void Init();
    // each says whether the background has the setting and gives it
    bool UseFog(jet::video::Color *color, float *start, float *end) const;
    bool UseSound(jet::String *label) const;
    bool UseMusic(jet::String *label) const;
    bool UsePostFx(MaterialDef **fx) const;
    bool UseSoftMusic() const { return m_useSoftMusic; }
    float GetGameplaySoundGain() const { return m_gameplaySoundsGain; }
    clara::DataEntity *GetLensFlare() const { return m_lensFlare; }
    // the name of the background library file, without "Background_"
    const jet::String &GetFileName() const { return m_fileName; }
    const jet::String &GetName() const { return m_name; }
    int GetLocation() const { return m_location; }
    const jet::String &GetLocationName() const { return m_locationName; }
    const jet::String &GetTitle() const { return m_title; }
private:
    MaterialDef *m_postFx;           // 0x20
    clara::DataEntity *m_lensFlare;  // 0x24
    bool m_fogEnabled;               // 0x28
    float m_fogStart, m_fogEnd;      // 0x2C
    jet::video::Color m_fogColor;    // 0x34
    bool m_ambienceEnabled;          // 0x38
    jet::String m_ambienceSound;     // 0x3C
    bool m_musicEnabled;             // 0x40
    jet::String m_musicLabel;        // 0x44
    bool m_useSoftMusic;             // 0x48
    float m_gameplaySoundsGain;      // 0x4C
    jet::String m_fileName;          // 0x50
    jet::String m_name;              // 0x54
    int m_location;                  // 0x58
    jet::String m_locationName;      // 0x5C
    jet::String m_title;             // 0x60
};

#endif

// source/game/entities/LocationConfig.h
#ifndef SOURCE_GAME_ENTITIES_LOCATIONCONFIG_H
#define SOURCE_GAME_ENTITIES_LOCATIONCONFIG_H

#include "entity.h"
#include "LargeMinionGameplay.h"

// What the data says of a location's gameplay: its menu music and ambient, the boss, the fluffy and cannon backgrounds
class LocationConfig : public clara::Entity {  // sizeof 0x8C
public:
    static const jet::String k_tmplName, k_param_LocationId, k_param_MainMenuMusic, k_param_MainMenuAmbient, k_param_RenderPlatforms,
        k_param_FluffyBackground, k_param_CannonBackgrounds, k_param_CannonTemplates, k_param_hasBoss, k_param_bossReference;
    explicit LocationConfig(const clara::Template *t) : clara::Entity(t), m_locationId(0), m_renderPlatforms(true), m_fluffyBackground(0),
        m_cannonBackgrounds(0), m_cannonTemplates(0), m_boss(0) {}
    virtual void PostInit();
    int GetLocationId() const { return m_locationId; }
    bool GetRenderPlatforms() const { return m_renderPlatforms; }
    clara::DataEntity *GetBoss() const { return m_boss; }
    clara::DataEntity *GetFluffyBackground() const { return m_fluffyBackground; }
    clara::DataEntity *GetCannonBackgrounds() const { return m_cannonBackgrounds; }
    clara::DataEntity *GetCannonTemplates() const { return m_cannonTemplates; }
    const jet::String &GetMainMenuMusic() const { return m_mainMenuMusic; }
    const jet::String &GetMainMenuAmbient() const { return m_mainMenuAmbient; }
    const LargeMinionGameplay::LargeMinionGameplayConfig &GetLargeMinionConfig() const { return m_largeMinionConfig; }
private:
    void LoadLargeMinionConfig();  // @0x57283c
    void LoadSequenceForLargeMinion(clara::DataEntity*, LargeMinionGameplay::LargeMinionGameplaySequence*);  // @0x572a00
    void LoadWave(clara::DataEntity*, LargeMinionGameplay::LargeMinionGameplayWave*);  // @0x572b08
    LargeMinionGameplay::LargeMinionGameplayConfig m_largeMinionConfig;  // 0x6C: the sequences and the runners of the mode
    int m_locationId;                    // 0x54
    clara::DataEntity *m_fluffyBackground;   // 0x58
    clara::DataEntity *m_cannonBackgrounds;  // 0x5C
    clara::DataEntity *m_cannonTemplates;    // 0x60
    bool m_renderPlatforms;              // 0x64
    clara::DataEntity *m_boss;           // 0x68
    jet::String m_mainMenuMusic;         // 0x84
    jet::String m_mainMenuAmbient;       // 0x88
};

#endif

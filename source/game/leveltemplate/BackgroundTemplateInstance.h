// source/game/leveltemplate/BackgroundTemplateInstance.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_BACKGROUNDTEMPLATEINSTANCE_H
#define SOURCE_GAME_LEVELTEMPLATE_BACKGROUNDTEMPLATEINSTANCE_H

#include "game/leveltemplate/LevelTemplateInstance.h"

class RocketPath;

// A background placed along the run: the decorations of its Background_ library and the settings (fog, sound, sky) it brings
class BackgroundTemplateInstance : public LevelTemplateInstance {  // sizeof 0x8C
public:
    static const jet::String k_skyDomeDeco, k_skyDomeObjectDeco;
    explicit BackgroundTemplateInstance(LevelTemplateDef *def) : LevelTemplateInstance(def), m_distanceToApplySettings(0), m_rocketPath(0) {}
    // the distance from which its settings apply
    void SetDistanceToApplySettings(unsigned d) { m_distanceToApplySettings = d; }
    unsigned GetDistanceToApplySettings() const { return m_distanceToApplySettings; }
    void ApplyPostFxValue() const;
    void ApplyFogValue() const;
    void ApplySoundSettings() const;
    void ApplyRocketPath() const;
    void ApplyLensFlare() const;
    void ApplySkyDome() const;
protected:
    virtual GameEntity *ProcessCreatedEntity(const GameEntity *tmpl, GameEntity *copy, GameplayTemplateInstance *gti);
private:
    unsigned m_distanceToApplySettings;  // 0x84
    GameEntity *m_rocketPath;            // 0x88
};

#endif

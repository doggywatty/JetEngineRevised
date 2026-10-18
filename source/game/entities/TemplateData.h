// source/game/entities/TemplateData.h
#ifndef SOURCE_GAME_ENTITIES_TEMPLATEDATA_H
#define SOURCE_GAME_ENTITIES_TEMPLATEDATA_H

#include "entity.h"

class PlatformInstance;

// What the data says of a level template as a whole: its kind and the platform it runs on
class TemplateData : public clara::Entity {  // sizeof 0x5D
public:
    static const jet::String k_tmplName, k_groundPlatform, k_IsShootingSection, k_isBonusSection, k_isLaunchSection, k_isGameplayTemplate,
        k_isTransitionSection;
    explicit TemplateData(const clara::Template *t) : clara::Entity(t), m_groundPlatform(0), m_isBonusSection(false), m_isShootingSection(false),
        m_isLaunchSection(false), m_isTransitionSection(false), m_isGameplayTemplate(false) {}
    virtual void Init();
    // the platform entity of the ground, found on first use
    PlatformInstance *GetGroundPlatform(bool);
    bool IsBonusSection() const { return m_isBonusSection; }
    bool IsShootingSection() const { return m_isShootingSection; }
    bool IsLaunchSection() const { return m_isLaunchSection; }
    bool IsTransitionSection() const { return m_isTransitionSection; }
    bool IsGameplayTemplate() const { return m_isGameplayTemplate; }
private:
    PlatformInstance *m_groundPlatform;  // 0x54
    bool m_isBonusSection;               // 0x58
    bool m_isShootingSection;            // 0x59
    bool m_isLaunchSection;              // 0x5A
    bool m_isTransitionSection;          // 0x5B
    bool m_isGameplayTemplate;           // 0x5C
};

#endif

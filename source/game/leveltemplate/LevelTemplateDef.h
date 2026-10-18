// source/game/leveltemplate/LevelTemplateDef.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_LEVELTEMPLATEDEF_H
#define SOURCE_GAME_LEVELTEMPLATE_LEVELTEMPLATEDEF_H

#include <map>
#include <vector>
#include "core/pstring.h"
#include "smath.h"
#include "video/color.h"
#include "game/entities/GameEntity.h"

class TemplateData;
class BackgroundData;
class PlatformInstance;
class MaterialDef;
namespace clara { class DataEntity; }

// A level template as the data defines it: the entities to place in it (sorted along the run), its size, its
// TemplateData and the background settings that go with it, and the decos each deco generation group may place
class LevelTemplateDef {  // sizeof 0x48
public:
    typedef GameEntity::GenerationGroupEntity GenerationGroupEntity;
    // the decos of a generation group, how many of them may be placed and how far apart
    struct EntityGenerationGroup {
        std::vector<const GenerationGroupEntity*> entities;  // 0x0
        unsigned maxDecos;                                   // 0xC
        float minDistance;                                   // 0x10
        EntityGenerationGroup() : maxDecos(3), minDistance(0) {}
        void AddEntity(const GenerationGroupEntity *e);
    };
    // the decos of a group a level template instance allows (indices into the group's entities)
    struct EntityGenerationGroupInstance { std::vector<unsigned> allowed; };
    typedef std::map<unsigned, EntityGenerationGroupInstance> GenerationMap;

    static const jet::String k_DecoGenerationGroupDefinition_tmplName, k_paramDecoGroupDefinitions, k_paramDecoGenerationGroup, k_paramMinDecos,
        k_paramMaxDecos;
    LevelTemplateDef();
    ~LevelTemplateDef();
    // an entity of the template (kept sorted by position), the template data or the background data
    void AddEntity(clara::DataEntity *e);
    void SetTemplateData(TemplateData *t) { m_templateData = t; }
    TemplateData *GetTemplateData() const { return m_templateData; }
    void SetBackgroundData(BackgroundData *b) { m_backgroundData = b; }
    BackgroundData *GetBackgroundData() const { return m_backgroundData; }
    unsigned GetEntityCount() const { return (unsigned)m_entities.size(); }
    GameEntity *GetEntity(unsigned i) const { return m_entities[i]; }
    void SetTemplateID(unsigned id) { m_templateId = id; }
    unsigned GetTemplateID() const { return m_templateId; }
    float GetSizeX() const { return m_size.x; }
    float GetSizeY() const { return m_size.y; }
    float GetOffsetX() const { return m_position.x; }
    void SetScaleMult(float m);
    // reads the position, size and ground platform of the template data, once
    void ProcessTemplateData();
    PlatformInstance *GetGroundPlatform() const;
    bool IsBonusTemplate() const;
    bool IsShootingArea() const;
    bool IsPauseTemplate() const;
    bool IsTransitionTemplate() const;
    bool IsGameplayTemplate() const;
    bool UsePostFx(MaterialDef **fx) const;
    bool UseFog(jet::video::Color *color, float *start, float *end) const;
    bool UseSound(jet::String *label) const;
    bool UseMusic(jet::String *label) const;
    bool UseSoftMusic() const;
    float GetGameplaySoundGain() const;
    math::vec3<float> GetTemplateDataPosition() const;
    const jet::String &GetName() const;
    EntityGenerationGroup &GetEntityGenerationGroup(unsigned id) { return m_groups[id]; }
    void InitEntityGenerationMap(GenerationMap &map) const;
    void RemoveEntityGenerationGroup(unsigned id) { m_groups.erase(id); }
    void LoadEntityGenerationGroupsData(clara::DataEntity *background);
private:
    unsigned m_templateId;                    // 0x4
    math::vec2<float> m_position;             // 0x8
    math::vec2<float> m_size;                 // 0x10
    bool m_processed;                         // 0x18
    TemplateData *m_templateData;             // 0x1C
    BackgroundData *m_backgroundData;         // 0x20
    std::vector<GameEntity*> m_entities;      // 0x24
    std::map<unsigned, EntityGenerationGroup> m_groups;  // 0x30
};

#endif

// source/game/leveltemplate/LevelTemplateInstance.h
#ifndef SOURCE_GAME_LEVELTEMPLATE_LEVELTEMPLATEINSTANCE_H
#define SOURCE_GAME_LEVELTEMPLATE_LEVELTEMPLATEINSTANCE_H

#include <list>
#include "smath.h"
#include "game/leveltemplate/LevelTemplateDef.h"

class GameplayTemplateInstance;

// A level template placed in the world: the entities made from its definition, where it starts and ends and the volume
// it covers. Entities are made a few at a time (SpawnEntities) as the level is built ahead of the player.
class LevelTemplateInstance {  // sizeof 0x80
public:
    // an entity made from the template with the transform it has in the template
    struct Instance {
        GameEntity *entity;
        math::vec3<float> position;
        math::quat<float> rotation;
        bool flag;
    };
    explicit LevelTemplateInstance(LevelTemplateDef *def);
    virtual ~LevelTemplateInstance() {}
    virtual void Render() {}
    // moves the instance so it starts there, the end anchor and the volume with it
    void SetStartAnchor(const math::vec3<float> &pos, float angle);
    void GetStartAnchor(math::vec3<float> &pos, float &angle) const { pos = m_startPos; angle = m_startAngle; }
    virtual void GetEndAnchor(math::vec3<float> &pos, float &angle) const { pos = m_endPos; angle = m_endAngle; }
    LevelTemplateDef *GetDefinition() const { return m_def; }
    bool IsBifurcationDecided() const { return m_bifurcationDecided; }
    void ResolveBifurcation(int b) { m_bifurcationType = b; m_bifurcationDecided = true; }
    int GetFinalBifurcation() const { return m_bifurcationType; }
    // makes the next entities of the template (all when count < 0) and where the instance ends; returns how many are made
    unsigned SpawnEntities(int count, GameplayTemplateInstance *gti);
    bool SpawningFinished() const { return m_def->GetEntityCount() <= m_lastEntityParsed; }
    virtual void Translate(const math::vec3<float> &d);
    // places the instance and its entities at another start
    void Reposition(const math::vec3<float> &pos, float angle);
    void SetPendingToDelete(unsigned distance);
    void EnableTouchInteracts(bool b);
    void DeleteEntities();
    virtual void SetEnabled(bool b) { m_isEnabled = b; }
    virtual bool IsEnabled() const { return m_isEnabled; }
    bool Contains(const math::vec3<float> &p) const;
    float PlaneDistance(const math::vec3<float> &p, float scale) const;
    const math::vec3<float> &GetDirection() const { return m_dir; }
    void SetDirection(const math::vec3<float> &d) { m_dir = d; }
    unsigned GetDistanceToRemove() const { return m_distanceToRemove; }
    void SetDistanceToRemove(unsigned d) { m_distanceToRemove = d; }
    std::list<Instance> &GetCreatedEntities() { return m_createdEntities; }
    // whether the entity's deco generation group lets it be placed
    bool IsEntityAllowed(const GameEntity::GenerationGroupEntity *e) const;
    // which decos of each group (of one group, id != 0) are placed in this instance
    virtual void BuildAllowedDecoList(unsigned group);
    static float s_lastAllowedDistancePosition;
protected:
    // the entity that stands for the made one in the created list, or none (@vtable 0x20)
    virtual GameEntity *ProcessCreatedEntity(const GameEntity *tmpl, GameEntity *copy, GameplayTemplateInstance *gti) = 0;
    void UpdateVolume(const math::vec3<float> &start, const math::quat<float> &rot);
    LevelTemplateDef *m_def;                    // 0x4
    std::list<Instance> m_createdEntities;      // 0x8
    LevelTemplateDef::GenerationMap m_entityGenerationGroups;  // 0x10
    bool m_isEnabled;                           // 0x28
    unsigned m_lastEntityParsed;                // 0x2C
    math::vec3<float> m_startPos;               // 0x30
    float m_startAngle;                         // 0x3C
    math::vec3<float> m_endPos;                 // 0x40
    float m_endAngle;                           // 0x4C
    bool m_bifurcationDecided;                  // 0x50
    int m_bifurcationType;                      // 0x54
    math::vec3<float> m_volumeMin, m_volumeMax; // 0x58
    math::vec3<float> m_dir;                    // 0x70
    unsigned m_distanceToRemove;                // 0x7C
};

#endif

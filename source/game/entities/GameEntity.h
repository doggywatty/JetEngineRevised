// source/game/entities/GameEntity.h
#ifndef SOURCE_GAME_ENTITIES_GAMEENTITY_H
#define SOURCE_GAME_ENTITIES_GAMEENTITY_H

#include <utility>
#include <vector>
#include "entity.h"
#include "project.h"
#include "core/pstring.h"

namespace jet::scene { class Model; class Dummy; }
namespace jet::video { class Material; }
class LevelTemplateInstance;
class MaterialDef;

// Base of the game's clara entities (GameEntity, 0x8C bytes in the 0.0.3 .so, 0x90 in the PDB layout).
// Registration with the GameLevel's update and render lists, persistence, deferred deletion, materials.
// Not ported yet: the attached sounds / particle systems (AttachedData) that follow the entity.
class GameEntity : public clara::Entity {
public:
    // the deco generation group an entity of a level template belongs to, and where it sits in it (@0x54)
    struct GenerationGroupEntity {
        unsigned group;
        float position;
        bool forced;
        GenerationGroupEntity() : group(0), position(0), forced(false) {}
    };
    typedef std::vector<std::pair<jet::String, MaterialDef*> > MaterialDefs;
    struct EntityTypeDef { enum type { Unknown = 0, Deco3d = 3, TouchInteract3d = 0x17 }; };
    struct PersistenceTypeDef { enum type { Default = 0, Volatile = 1 }; };
    // how a copy is made: Interface entities draw into the menu scene and get an effect layer; DontRegister
    // keeps the copy off the level's update and render lists
    struct EGEConstructorFlagsDef { enum { None = 0, Interface = 1, DontRegister = 2 }; };

    explicit GameEntity(const clara::Template*);
    virtual ~GameEntity();
    virtual void Init();
    virtual void PostInit();
    virtual void Update();
    virtual void Render() {}
    virtual void SetEnabled(bool);
    virtual void SetVisible(bool);
    virtual void Translate(const math::vec3<float>&);
    virtual void ConstructFrom(const clara::Entity*);
    virtual void ConstructFrom(const GameEntity*);
    virtual void SetPersistenceType(int);
    virtual void Destroy() { m_destroyed = true; }
    // not placed in a level template that does not allow it, and not a template description
    virtual bool CanSpawnNewEntity(LevelTemplateInstance*) const;
    virtual void SetSpawnLane(int) {}
    virtual void OnMinionRevive() {}
    virtual void OnLinkedHidden() {}
    virtual void OnPositionedInWorld() {}
    virtual bool IsCollidable() const { return false; }
    // pending deletion and behind the player
    virtual bool IsOutOfScene() const;
    virtual math::vec3<float> GetPositionOnTemplate() const { return GetPosition(); }
    using clara::Entity::ConstructFrom;

    void RegisterForUpdate(bool);
    void RegisterForRender(bool);
    bool IsRegisteredForUpdate() const { return m_isRegisteredForUpdate; }
    bool IsRegisteredForRender() const { return m_isRegisteredForRender; }
    const GenerationGroupEntity &GetGenerationGroup() const { return m_generation; }
    int GetType() const { return m_type; }
    float GetDistanceInRun() const { return m_distanceInRun; }
    void SetDistanceInRun(float d) { m_distanceInRun = d; }
    int GetPersistenceType() const { return m_persistenceType; }
    int GetAppearProbability() const { return m_appearProbability; }
    bool GetPendingToDelete() const { return m_pendingToDelete; }
    void SetPendingToDelete(unsigned distanceToDelete);
    clara::Entity *GetOriginalEntity() const { return m_originalEntity; }
    // a point is out of sight when it is behind the camera plane test or farther than the range
    static bool CheckVisibility(float rangeSq, float, const math::vec3<float>& pos, float);

    void ApplyMaterials(jet::scene::Model*, const jet::String&, MaterialDef*) const;
    void ApplyMaterials(jet::scene::Model*, MaterialDef*) const;
    void ApplyMaterials(jet::scene::Model*, const MaterialDefs&) const;
    void LoadMaterials(MaterialDefs&, const jet::String&, bool) const;
    // The dummies of a model named "Animated_Dummy<n>" (the list is indexed by n; a number without dummy is null) drive the shaders of its
    // materials: the scale of the dummy minus one and its rotation in turns go to the uniforms animdummyscale<n> and animdummyrot<n>
    // (BuildAnimatedDummyList @0x5657a0, UpdateShaderUniformByAnimatedDummy @0x565ab8: true when a uniform changed)
    void BuildAnimatedDummyList(jet::scene::Model*, std::vector<jet::scene::Dummy*>&) const;
    bool UpdateShaderUniformByAnimatedDummy(std::vector<jet::scene::Dummy*>&, jet::scene::Model*) const;
    bool UpdateShaderUniformByAnimatedDummy(std::vector<jet::scene::Dummy*>&, jet::video::Material&) const;

    // "X_<name>_<address>" with every non alphanumeric character a "_": the prefix of its Lua functions
    jet::String GetScriptPrefix() const;
    // Milliseconds of the frame being updated (the game's frame time)
    static int s_frameDt;

protected:
    void _RegisterForUpdate(bool);
    void _RegisterForRender(bool);

    GenerationGroupEntity m_generation;  // 0x54
    int m_type;                       // 0x60
    int m_persistenceType;            // 0x64
    bool m_destroyed;                 // 0x68
    int m_appearProbability;          // 0x6C: percent, 100
    bool m_isRegisteredForUpdate;     // 0x70
    bool m_isRegisteredForRender;     // 0x71
    float m_distanceInRun;            // 0x74: how far along the run it stands (set where objects are placed on the platforms)
    clara::Entity *m_originalEntity;  // 0x78: the entity this is a copy of
    bool m_pendingToDelete;           // 0x84
    unsigned m_distanceToDelete;      // 0x88
};

#endif

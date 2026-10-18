// source/game/entities/PlatformInstance.h
#ifndef SOURCE_GAME_ENTITIES_PLATFORMINSTANCE_H
#define SOURCE_GAME_ENTITIES_PLATFORMINSTANCE_H

#include <list>
#include <vector>
#include "GameEntity.h"
#include "ObjectHolder.h"

class Object;
class PlatformTemplate;
class GameplayTemplateInstance;
class LevelTemplateDef;
namespace jet::scene { class Model; class Dummy; }

// A platform placed in a level template: it takes the object of its PlatformTemplate and makes the gameplay objects its dummies hold
// (obstacles, runners, interactive objects), a few at a time. A template of several lengths (the randomizer) has copies that follow
// each other, the first holding the objects to make.
class PlatformInstance : public GameEntity, public ObjectHolder {  // sizeof 0xE4
public:
    // an object to make at a dummy of the platform's model
    struct GenerationSequence {
        GameEntity *entity;                    // 0x0: what to copy
        jet::String dummyName;                 // 0x4
        unsigned index;                        // 0x8: the dummy's index along the platform
        unsigned lane;                         // 0xC: 0 right, 1 center, 2 left
        GameplayTemplateInstance *gti;         // 0x10
        GameEntity *created;                   // 0x14
        GenerationGroupEntity *group;          // 0x18
    };
    static jet::String k_tmplName;
    static const jet::String k_DummyHelperInstance_tmplName, k_DummyHelper_tmplName, k_ChunkTemplateSelector_tmplName;
    explicit PlatformInstance(const clara::Template *t);
    virtual ~PlatformInstance();
    virtual const jet::String &GetBaseTemplateName() const { return k_tmplName; }
    virtual void Init();
    virtual void PostInit();
    virtual const math::vec3<float> &GetPosition() const;
    virtual void SetPosition(const math::vec3<float>&);
    virtual const math::quat<float> &GetRotation() const;
    virtual void SetRotation(const math::quat<float>&);
    virtual void Translate(const math::vec3<float>&);
    virtual void SetPersistenceType(int t);
    virtual void ConstructFrom(const GameEntity*);
    virtual void OnPositionedInWorld();
    using GameEntity::ConstructFrom;
    PlatformTemplate *GetDefinition() const { return m_platformTemplate; }
    virtual void SetKeepCreatedEntity(bool b) { m_keepCreatedEntity = b; }
    virtual Object *GetCreatedEntity() const { return m_createdObject; }
    bool UsesRandomizer() const { return m_useRandomizer; }
    float GetRandomizerSizeMult() const { return m_randomizerSizeMult; }
    void SetClonedFromRandomIndex(int i) { m_clonedFromRandomIndex = i; }
    // the copies of the next length of a randomized template: this one is the first
    void ProcessRandomizerData(LevelTemplateDef*);
    PlatformInstance *GetNextCloned() const { return m_nextCloned; }
    int GetSingleLane() const { return m_hasSingleLane ? (int)m_singleLane : -1; }
    // makes up to count objects of the sequence (all when not positive); true once none is left
    bool CreateObjectsFromSequence(GameplayTemplateInstance*, int count);
    void SetGameplayTemplateInstance(GameplayTemplateInstance *g) { m_gameplayTemplateInstance = g; }
    // what to make from the chunks and the dummies of this platform's data, and the copy of it to make from
    void AddSequencesFromParamConfig();
    void AddSequencesFromParamConfig(const std::vector<const clara::Param*> &params, jet::scene::Model*, unsigned offset);
    void AddSequencesFromRandomizer();
    void CopySequenceToCreate();
private:
    void InitRandomizer();
    unsigned GetChunkCount(clara::DataEntity*) const;
    void AddSequencesFromChunk(clara::DataEntity*, unsigned &offset);
    void AddToSequence(clara::DataEntity *config, const jet::String &dummyName, unsigned offset, unsigned lane, std::vector<GenerationSequence> &out);
    GameEntity *CreateLinkedObject(GameEntity *tmpl, jet::scene::Dummy *dummy, unsigned lane, unsigned index, GameplayTemplateInstance *gti);
    // the copy of a sequence element that the platform takes over (its generation group, or the linked template of an obstacle)
    void ProcessCopiedSequence(size_t index, std::vector<GenerationSequence> &list);
    // an object made from a sequence element: the elements an obstacle covers, and the object placed in the frame of the obstacle covering it
    void ProcessGeneratedLinkedObject(GameEntity *obj, size_t index, GameplayTemplateInstance *gti, std::vector<GenerationSequence> &list);
    Object *m_createdObject;                       // 0x8C
    bool m_keepCreatedEntity;                      // 0x90
    PlatformTemplate *m_platformTemplate;          // 0x94
    bool m_useRandomizer;                          // 0x98
    float m_randomizerSizeMult;                    // 0x9C
    unsigned m_numDummies;                         // 0xA0
    unsigned m_numChunks;                          // 0xA4
    int m_clonedFromRandomIndex;                   // 0xA8
    PlatformInstance *m_nextCloned;                // 0xAC
    PlatformInstance *m_baseInstance;              // 0xB0
    bool m_hasSingleLane;                          // 0xB4
    unsigned m_singleLane;                         // 0xB8
    bool m_addDefaultSafeMinionPoint;              // 0xBC
    std::list<GameEntity*> m_createdLinkedObjects; // 0xC0
    std::vector<PlatformInstance*> m_clonedList;   // 0xC8
    std::vector<GenerationSequence> m_generationSequence;  // 0xD4
    GameplayTemplateInstance *m_gameplayTemplateInstance;  // 0xE0
};

#endif

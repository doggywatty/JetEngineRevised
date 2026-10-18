// source/game/entities/ChunkTemplate.h
#ifndef SOURCE_GAME_ENTITIES_CHUNKTEMPLATE_H
#define SOURCE_GAME_ENTITIES_CHUNKTEMPLATE_H

#include "entity.h"

class PlatformInstance;

// A stretch of a platform's objects: the references of its Dummy_R, Dummy_C and Dummy_L, repeated along randomized platforms
class ChunkTemplate : public clara::DataEntity {  // sizeof 0x90
public:
    static jet::String k_tmplName;
    explicit ChunkTemplate(const clara::Template *t) : clara::DataEntity(t) {}
    virtual void Init() { if (!(m_flags & F_LIBRARY)) clara::DataEntity::Init(); }
    virtual void PostInit() { if (!(m_flags & F_LIBRARY)) clara::DataEntity::PostInit(); }
    // adds the objects of its dummies to the platform's sequence, the dummy indices starting at offset
    void AddObjectsToSequence(PlatformInstance*, unsigned offset) const;
};

#endif

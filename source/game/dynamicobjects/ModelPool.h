// source/game/dynamicobjects/ModelPool.h
#ifndef SOURCE_GAME_DYNAMICOBJECTS_MODELPOOL_H
#define SOURCE_GAME_DYNAMICOBJECTS_MODELPOOL_H

#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"

namespace jet::scene { class Model; class SceneMgr; }

// The models of the effects (ctor @0x59d0a4, 0x10 bytes). An effect that plays a model takes it from here and gives it back: the model is kept
// loaded for the next effect that asks for the same one, and the oldest free ones are reused for others.
class ModelPool : public Singleton<ModelPool> {
public:
    ModelPool();
    virtual ~ModelPool();
    // @0x59d23c: the models taken in that layer are added to the scene (and their shadows, which follow the model's animation, to the shadow tag)
    void Render(jet::scene::SceneMgr *sceneMgr, int layerId);
    // @0x59d18c: every model goes
    void DestroyAll();
    // @0x59d1f0: the model is free again
    void Release(jet::scene::Model *model);
    // @0x59d350: a free model for the layer; the shadow model of the entry goes to shadowOut when the game draws shadows
    jet::scene::Model *GetModel(int layerId, const jet::String &modelName, const jet::String &shadowName, jet::scene::Model **shadowOut);
private:
    // ModelPoolEntry (0x18 bytes)
    struct Entry {
        jet::scene::Model *model;        // 0x00
        jet::scene::Model *shadow;       // 0x04
        long long lastUsed;              // 0x08: when it was taken (ms)
        bool inUse;                      // 0x10
        int layerId;                     // 0x14
    };
    // @0x59d5b4: the model is loaded when there is none, or when it is another one than asked (the one that was there goes first)
    void LoadModelIfNeeded(jet::scene::Model **slot, const jet::String &name);
    // @0x59d670: out of the scene (under its tag, if it has one) and freed
    void FreeModel(jet::scene::Model **slot, const jet::String &tag);

    std::vector<Entry> m_pool;           // 0x4
};

#endif

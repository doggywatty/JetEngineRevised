// core/jet/include/scene/modelloader.h
#ifndef CORE_JET_INCLUDE_SCENE_MODELLOADER_H
#define CORE_JET_INCLUDE_SCENE_MODELLOADER_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "core/pstring.h"

namespace jet::stream { class MemoryStream; }

namespace jet::scene {
class Model;
class ModelBase;

// ModelLoader (0x3C): model bases cached by name, every created instance kept alive.
class ModelLoader {
public:
    // Load(name) @0x702b54 uses { false, true, true }
    struct Params {
        bool geometryCPUAccess;
        bool loadRenderTechniques;
        bool loadTextures;
        Params() : geometryCPUAccess(false), loadRenderTechniques(true), loadTextures(true) {}
    };
protected:
    ModelLoader();
    ~ModelLoader();
public:
    static ModelLoader *GetInstance();
    static void FreeInstance();
    void Free(Model*);
    void FreeAll();
    void FreeUnusedModelBases();
    void UnloadAll();
    void LoadAllModelGeometry();
    bool LoadAllModelGeometry(unsigned&, unsigned);
    bool CanLoad(const jet::String&);
    Model *Load(const jet::String&, const Params&);
    Model *Load(const jet::String&);
    void CleanUp();
    bool IsStreamingEnabled() const { return m_isStreamingEnabled; }
    void SetStreamingEnabled(bool on, unsigned frames) { m_isStreamingEnabled = on; m_framesBeforeUnloading = frames; }
    boost::shared_ptr<ModelBase> LoadModelBase(const jet::String&, const Params&);
private:
    static ModelLoader *s_instance;
    bool m_isStreamingEnabled;       // 0x0
    unsigned m_framesBeforeUnloading;  // 0x4
    int m_skipFrames;                // 0x8
    boost::unordered_map<jet::String, boost::shared_ptr<ModelBase> > m_map;  // 0xC
    std::vector<boost::shared_ptr<Model> > m_array;  // 0x24
};
}

#endif

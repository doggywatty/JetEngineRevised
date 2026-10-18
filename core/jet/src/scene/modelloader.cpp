// core/jet/src/scene/modelloader.cpp
#include "scene/modelloader.h"
#include "scene/modelbase.h"
#include "scene/model.h"
#include "stream/streammgr.h"
#include <cstdio>

namespace jet::scene {

ModelLoader *ModelLoader::s_instance = 0;

ModelLoader::ModelLoader() : m_isStreamingEnabled(false), m_framesBeforeUnloading(0), m_skipFrames(0) {}
ModelLoader::~ModelLoader() { FreeAll(); }

ModelLoader *ModelLoader::GetInstance() {
    if (!s_instance) s_instance = new ModelLoader();
    return s_instance;
}
void ModelLoader::FreeInstance() { delete s_instance; s_instance = 0; }

// Free @0x7028dc
void ModelLoader::Free(Model *m) {
    for (std::vector<boost::shared_ptr<Model> >::iterator it = m_array.begin(); it != m_array.end(); ++it)
        if (it->get() == m) { m_array.erase(it); return; }
}
void ModelLoader::FreeAll() { m_array.clear(); m_map.clear(); }
void ModelLoader::CleanUp() { FreeAll(); }

// FreeUnusedModelBases @0x7031d0: bases only the cache references
void ModelLoader::FreeUnusedModelBases() {
    for (boost::unordered_map<jet::String, boost::shared_ptr<ModelBase> >::iterator it = m_map.begin(); it != m_map.end();)
        if (!it->second || it->second.use_count() == 1) it = m_map.erase(it);
        else ++it;
}

void ModelLoader::UnloadAll() {
    for (boost::unordered_map<jet::String, boost::shared_ptr<ModelBase> >::iterator it = m_map.begin(); it != m_map.end(); ++it)
        if (it->second) it->second->Unload();
}

void ModelLoader::LoadAllModelGeometry() { unsigned i = 0; LoadAllModelGeometry(i, 0); }
bool ModelLoader::LoadAllModelGeometry(unsigned &idx, unsigned) {
    // geometry is created on first use (SubMesh::GetGeometry), so there is nothing to preload
    idx = (unsigned)m_array.size();
    return true;
}

// CanLoad @0x702a68
bool ModelLoader::CanLoad(const jet::String &name) {
    if (m_map.find(name) != m_map.end()) return m_map[name] != 0;
    return jet::stream::StreamMgr::GetInstance().CanCreateStreamByName(name);
}

Model *ModelLoader::Load(const jet::String &name) { return Load(name, Params()); }

// Load @0x702b78
Model *ModelLoader::Load(const jet::String &name, const Params &p) {
    if (name.empty()) {
        fprintf(stderr, "ERROR: empty model name\n");
        return 0;
    }
    boost::shared_ptr<ModelBase> base = LoadModelBase(name, p);
    if (!base) return 0;
    boost::shared_ptr<Model> m(base->CreateInstance());
    m_array.push_back(m);
    return m.get();
}

// LoadModelBase @0x702f84: a failed name is cached as empty so it is not retried
boost::shared_ptr<ModelBase> ModelLoader::LoadModelBase(const jet::String &name, const Params &p) {
    boost::unordered_map<jet::String, boost::shared_ptr<ModelBase> >::iterator it = m_map.find(name);
    if (it != m_map.end()) return it->second;
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
    if (!s) {
        fprintf(stderr, "ERROR: could not find model %s\n", name.c_str());
        m_map[name] = boost::shared_ptr<ModelBase>();
        return boost::shared_ptr<ModelBase>();
    }
    boost::shared_ptr<ModelBase> base(new ModelBase());
    base->SetResourceName(name);
    base->SetGeometryCPUAccess(p.geometryCPUAccess);
    base->SetLoadRenderTechniques(p.loadRenderTechniques);
    base->SetLoadTextures(p.loadTextures);
    base->Init(s);
    if (!base->Load()) {
        fprintf(stderr, "ERROR: could not load model %s\n", name.c_str());
        m_map[name] = boost::shared_ptr<ModelBase>();
        return boost::shared_ptr<ModelBase>();
    }
    m_map[name] = base;
    return base;
}

}

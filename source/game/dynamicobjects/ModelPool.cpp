// source/game/dynamicobjects/ModelPool.cpp
#include "ModelPool.h"
#include "game/Game.h"
#include "game/GameLevel.h"
#include "game/CGameConfig.h"
#include "scene/model.h"
#include "scene/modelloader.h"
#include "scene/scenemgr.h"
#include "system.h"

// @0x59d0a4
ModelPool::ModelPool() {}

// @0x59d0e8
ModelPool::~ModelPool() { DestroyAll(); }

// @0x59d18c: the models and their shadows (out of the scene under the shadow tag) go, the pool is emptied
void ModelPool::DestroyAll() {
    for (size_t i = 0; i < m_pool.size(); i++) {
        FreeModel(&m_pool[i].shadow, GameLevel::GetShadowTagName());
        FreeModel(&m_pool[i].model, jet::String::null);
    }
    m_pool.clear();
}

// @0x59d1f0: the model becomes free; it leaves the node it was attached to
void ModelPool::Release(jet::scene::Model *model) {
    if (!model) return;
    for (size_t i = 0; i < m_pool.size(); i++)
        if (m_pool[i].model == model) {
            model->GetRootNode().AttachTo(0);
            m_pool[i].inUse = false;
            return;
        }
}

// @0x59d23c
void ModelPool::Render(jet::scene::SceneMgr *sceneMgr, int layerId) {
    for (size_t i = 0; i < m_pool.size(); i++) {
        Entry &e = m_pool[i];
        if (!e.inUse || e.layerId != layerId) continue;
        sceneMgr->Add(e.model);
        if (!e.shadow || CGameConfig::Current().useShadows <= 0) continue;
        // the shadow plays the animation of the model, at the same time and place
        if (e.shadow->GetAnimation() != e.model->GetAnimation()) e.shadow->SetAnimation(e.model->GetAnimation(), 100);
        e.shadow->GetRootNode().SetTransform(e.model->GetRootNode().GetPosition(), e.model->GetRootNode().GetRotation());
        e.shadow->CC().SetTime(e.model->CC().GetTime());
        sceneMgr->Add(GameLevel::GetShadowTagName(), e.shadow);
    }
}

// @0x59d350
jet::scene::Model *ModelPool::GetModel(int layerId, const jet::String &modelName, const jet::String &shadowName, jet::scene::Model **shadowOut) {
    // the free entry that holds the same model and was used the longest ago, else the free entry that was used the latest
    Entry *sameModel = 0, *other = 0;
    long long sameModelTime = 99999999LL, otherTime = -99999999LL;
    for (size_t i = 0; i < m_pool.size(); i++) {
        Entry &e = m_pool[i];
        if (e.inUse) continue;
        bool same = e.model && const_cast<jet::scene::ModelBase*>(e.model->GetBase().get())->GetResourceName().Equals(modelName);
        if (same && e.lastUsed < sameModelTime) {
            sameModelTime = e.lastUsed;
            sameModel = &e;
        } else if (otherTime < e.lastUsed) {
            otherTime = e.lastUsed;
            other = &e;
        }
    }
    Entry *entry = sameModel ? sameModel : other;
    if (!entry) {
        Entry fresh = { 0, 0, 0, false, 0 };
        m_pool.push_back(fresh);
        entry = &m_pool.back();
    }
    entry->inUse = true;
    entry->lastUsed = jet::System::GetTime();
    entry->layerId = layerId;
    LoadModelIfNeeded(&entry->model, modelName);
    // the entry's shadow is made again (the .so just forgets the old one, which stays loaded)
    FreeModel(&entry->shadow, GameLevel::GetShadowTagName());
    if (CGameConfig::Current().useShadows > 0) {
        LoadModelIfNeeded(&entry->shadow, shadowName);
        if (shadowOut) *shadowOut = entry->shadow;
    }
    return entry->model;
}

// @0x59d5b4
void ModelPool::LoadModelIfNeeded(jet::scene::Model **slot, const jet::String &name) {
    if (*slot && !name.empty() && !const_cast<jet::scene::ModelBase*>((*slot)->GetBase().get())->GetResourceName().Equals(name)) FreeModel(slot, jet::String::null);
    if (!name.empty() && !*slot) *slot = jet::scene::ModelLoader::GetInstance()->Load(name);
}

// @0x59d670
void ModelPool::FreeModel(jet::scene::Model **slot, const jet::String &tag) {
    if (!*slot) return;
    jet::scene::SceneMgr *sceneMgr = GameLevel::GetSceneMgr(GameLevel::SceneMgr_World);
    if (!tag.empty()) sceneMgr->Remove(tag, *slot);
    else sceneMgr->Remove(*slot);
    jet::scene::ModelLoader::GetInstance()->Free(*slot);
    *slot = 0;
}

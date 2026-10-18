// source/game/interfaces/aurora/AuroraLevelMgr.cpp
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include "sprites/spritemgr.h"
#include "stream/streammgr.h"
#include <cstdio>

AuroraLevelMgr::~AuroraLevelMgr() { Unload(true); }

// @0x493dc0: every entry of the factory is a level
void AuroraLevelMgr::Load(boost::shared_ptr<jet::stream::IStreamFactory> layers, const jet::String &namesFile, bool unload, float gameW, float gameH, float scale, const std::vector<jet::String> &gfxSuffixes) {
    fprintf(stderr, "AuroraLevelMgr::Load %s\n", layers->GetPath().c_str());
    m_gfxSpecificSuffixes = gfxSuffixes;
    Unload(unload);
    for (unsigned i = 0; i < layers->GetFileCount(); i++) {
        if (i >= m_auroraLevelData.size()) m_auroraLevelData.push_back(new AuroraLevelData());
        AuroraLevelData *level = m_auroraLevelData[i];
        jet::stream::IStream *s = layers->CreateStream(jet::String::Format("level%d.layers", i + 1));
        if (!s) continue;
        s->Start();
        if (s->Size()) level->Load(*s);
        s->Finish();
        delete s;
    }
    _LoadLvlNames(namesFile);
    _ResetGUIToDisplayData(gameW, gameH, scale);
}

// @0x494054: unload empties the levels, otherwise they are deleted
void AuroraLevelMgr::Unload(bool destroy) {
    if (destroy) {
        for (AuroraLevelData *l : m_auroraLevelData) delete l;
        m_auroraLevelData.clear();
    } else
        for (AuroraLevelData *l : m_auroraLevelData) l->Unload();
}

// @0x4941cc: level count, then the element names of each level
void AuroraLevelMgr::_LoadLvlNames(const jet::String &file) {
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(file);
    if (!s) return;
    s->Start();
    if (s->IsStarted()) {
        int n = 0;
        *s >> n;
        for (int i = 0; i < n && i < (int)m_auroraLevelData.size(); i++) m_auroraLevelData[i]->LoadElementNames(*s);
    }
    s->Finish();
    delete s;
}

void AuroraLevelMgr::_ResetGUIToDisplayData(float w, float h, float scale) {
    for (AuroraLevelData *l : m_auroraLevelData) l->ResetGUIToDisplayData(w, h, scale);
}

int AuroraLevelMgr::_GetLevelId(const jet::String &name) const {
    for (unsigned i = 0; i < m_auroraLevelData.size(); i++)
        if (name.Equals(m_auroraLevelData[i]->GetLevelName())) return (int)i;
    return -1;
}

// @0x4942d4: a device specific suffix wins over the plain name
int AuroraLevelMgr::GetLevelId(const jet::String &name) const {
    for (const jet::String &suffix : m_gfxSpecificSuffixes) {
        int id = _GetLevelId(jet::String::Format("%s%s", name.c_str(), suffix.c_str()));
        if (id >= 0) return id;
    }
    return _GetLevelId(name);
}

// @0x4940dc
void AuroraLevelMgr::LoadGraphItemSpriteNames(const jet::String &file) {
    m_graphItemSpriteNames.clear();
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(file);
    if (!s) return;
    s->Start();
    if (s->IsStarted()) {
        int n = 0;
        *s >> n;
        for (int i = 0; i < n; i++) {
            jet::String name;
            *s >> name;
            m_graphItemSpriteNames.push_back(name);
        }
    }
    s->Finish();
    delete s;
}

// @0x4943e8
Sprite *AuroraLevelMgr::GetGraphItemSprite(unsigned i) const {
    return SpriteMgr::GetInstance()->GetSprite(m_graphItemSpriteNames[i], false);
}

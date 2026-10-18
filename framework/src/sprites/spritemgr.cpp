// framework/src/sprites/spritemgr.cpp
#include "sprites/spritemgr.h"
#include "stream/streammgr.h"

SpriteMgr::SpriteMgr() : m_hasPrefix(false) {}
SpriteMgr::~SpriteMgr() {}

jet::String SpriteMgr::MappedName(const jet::String &name) const {
    auto it = m_spritesNameMapping.find(name.c_str());
    return it == m_spritesNameMapping.end() ? name : it->second;
}

// @0x578410
bool SpriteMgr::CanLoadSprite(const jet::String &name) {
    if (m_sprites.find(name.c_str()) != m_sprites.end()) return true;
    jet::String mapped = MappedName(name);
    jet::stream::StreamMgr &sm = jet::stream::StreamMgr::GetInstance();
    if (m_hasPrefix && sm.CanCreateStream(m_prefix + mapped + ".bsprite", true)) return true;
    return sm.CanCreateStream(mapped + ".bsprite", true);
}

// @0x578550: the prefixed name is tried first
Sprite *SpriteMgr::LoadSprite(const jet::String &name, bool canLoadWithoutTGA) {
    auto it = m_sprites.find(name.c_str());
    if (it != m_sprites.end() && it->second) return it->second.get();
    jet::String mapped = MappedName(name);
    Sprite *s = 0;
    if (m_hasPrefix) s = Sprite::LoadSprite(m_prefix + mapped, canLoadWithoutTGA);
    if (!s) s = Sprite::LoadSprite(mapped, canLoadWithoutTGA);
    m_sprites[name.c_str()] = boost::shared_ptr<Sprite>(s);
    return s;
}

// @0x42487c
Sprite *SpriteMgr::GetSprite(const jet::String &name, bool canLoadWithoutTGA) {
    auto it = m_sprites.find(name.c_str());
    if (it != m_sprites.end()) return it->second.get();
    return LoadSprite(name, canLoadWithoutTGA);
}

void SpriteMgr::FreeSprite(const jet::String &name) { m_sprites.erase(name.c_str()); }
void SpriteMgr::FreeSprites() { m_sprites.clear(); }

// @0x578800: loads everything again from its stream
void SpriteMgr::ReloadSprites(bool canLoadWithoutTGA) {
    std::vector<jet::String> names = GetSpriteNames();
    m_sprites.clear();
    for (const jet::String &n : names) LoadSprite(n, canLoadWithoutTGA);
}

void SpriteMgr::UnloadTextureData() {
    for (auto &s : m_sprites) if (s.second) s.second->Unload();
}
void SpriteMgr::UnloadTextureData(const jet::String &name) {
    auto it = m_sprites.find(name.c_str());
    if (it != m_sprites.end() && it->second) it->second->Unload();
}

std::vector<jet::String> SpriteMgr::GetSpriteNames() const {
    std::vector<jet::String> out;
    for (auto &s : m_sprites) out.push_back(jet::String(s.first.c_str()));
    return out;
}

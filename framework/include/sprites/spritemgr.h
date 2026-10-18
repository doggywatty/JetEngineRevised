// framework/include/sprites/spritemgr.h
#ifndef FRAMEWORK_INCLUDE_SPRITES_SPRITEMGR_H
#define FRAMEWORK_INCLUDE_SPRITES_SPRITEMGR_H

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "jsingleton.h"
#include "core/pstring.h"
#include "sprites/sprite.h"

// owns the loaded Aurora sprites by name (SpriteMgr @0x578014)
class SpriteMgr : public Singleton<SpriteMgr> {  // sizeof 0xF0
public:
    SpriteMgr();
    virtual ~SpriteMgr();
    bool CanLoadSprite(const jet::String&);
    Sprite *LoadSprite(const jet::String&, bool canLoadWithoutTGA);
    Sprite *GetSprite(const jet::String&, bool canLoadWithoutTGA);
    void ReloadSprites(bool);
    void FreeSprite(const jet::String&);
    void FreeSprites();
    void UnloadTextureData();
    void UnloadTextureData(const jet::String&);
    void SetPrefix(const jet::String &p) { m_prefix = p; m_hasPrefix = true; }
    const jet::String &GetPrefix() const { return m_prefix; }
    bool HasPrefix() const { return m_hasPrefix; }
    std::vector<jet::String> GetSpriteNames() const;
    void SetSpritesNameMapping(const std::map<std::string, jet::String> &m) { m_spritesNameMapping = m; }
private:
    jet::String MappedName(const jet::String&) const;
    std::map<std::string, jet::String> m_spritesNameMapping;     // 0x4
    std::map<std::string, boost::shared_ptr<Sprite> > m_sprites; // 0x1c
    bool m_hasPrefix;                                            // 0x3c
    jet::String m_prefix;                                        // 0x40
};

#endif

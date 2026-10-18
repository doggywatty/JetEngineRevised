// source/game/interfaces/aurora/AuroraLevelMgr.h
#ifndef SOURCE_GAME_INTERFACES_AURORA_AURORALEVELMGR_H
#define SOURCE_GAME_INTERFACES_AURORA_AURORALEVELMGR_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include "jsingleton.h"
#include "core/pstring.h"
#include "stream/istreamfactory.h"
#include "game/interfaces/aurora/AuroraLevelData.h"

class Sprite;

// the GUI layers of the gui_1_1 datalib ("level<N>.layers") by name, and the sprite list Graph elements index
class AuroraLevelMgr : public Singleton<AuroraLevelMgr> {  // sizeof 0x40
public:
    virtual ~AuroraLevelMgr();
    void Load(boost::shared_ptr<jet::stream::IStreamFactory> layers, const jet::String &namesFile, bool unload, float gameW, float gameH, float scale, const std::vector<jet::String> &gfxSuffixes);
    void Unload(bool destroy);
    unsigned GetLevelCount() const { return (unsigned)m_auroraLevelData.size(); }
    int GetLevelId(const jet::String&) const;
    AuroraLevelData *GetLevel(int i) const { return m_auroraLevelData[i]; }
    AuroraLevelData *GetLevel(const jet::String &name) const { int i = GetLevelId(name); return i < 0 ? 0 : m_auroraLevelData[i]; }
    void LoadGraphItemSpriteNames(const jet::String &file);
    const jet::String &GetGraphItemSpriteName(unsigned i) const { return m_graphItemSpriteNames[i]; }
    Sprite *GetGraphItemSprite(unsigned i) const;
private:
    void _LoadLvlNames(const jet::String&);
    void _ResetGUIToDisplayData(float, float, float);
    int _GetLevelId(const jet::String&) const;
    std::vector<AuroraLevelData*> m_auroraLevelData;         // 0x4
    std::vector<jet::String> m_graphItemSpriteNames;         // 0x10
    std::vector<jet::String> m_gfxSpecificSuffixes;          // 0x1c
};

#endif

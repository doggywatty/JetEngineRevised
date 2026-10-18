// source/game/gamemanagers/LocationsMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_LOCATIONSMGR_H
#define SOURCE_GAME_GAMEMANAGERS_LOCATIONSMGR_H

#include <map>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"

class LocationInfoEntity;
class LocationConfig;

// The location entities of the loaded data by location id
class LocationsMgr : public Singleton<LocationsMgr> {  // sizeof 0x40
public:
    LocationsMgr() {}
    virtual ~LocationsMgr() { ClearData(); }
    // looks the entities up in the project
    void Reset() { ResetLocationInfo(); ResetLocationConfig(); }
    void ClearData() { ClearLocationInfo(); ClearLocationConfig(); }
    LocationInfoEntity *GetLocationInfo(int location) const;
    LocationConfig *GetLocationConfig(int location);
    // the location and the one it falls back to
    const std::map<int, LocationInfoEntity*> &GetAllLocationInfo() const { return m_info; }
    const std::vector<int> &GetAssociatedLocationsFor(int location) const;
    const jet::String &GetMainMenuMusicLabel(int location);
    const jet::String &GetMainMenuAmbientLabel(int location);
    void ResetLocationInfo();
    void ClearLocationInfo() { m_info.clear(); }
    void ResetLocationConfig();
    void ClearLocationConfig() { m_config.clear(); }
private:
    std::map<int, LocationInfoEntity*> m_info;  // 0x4
    std::map<int, LocationConfig*> m_config;    // 0x1C
    static const std::vector<int> s_emptyLocationArray;
};

#endif

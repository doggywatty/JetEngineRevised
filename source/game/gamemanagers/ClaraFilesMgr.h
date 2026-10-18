// source/game/gamemanagers/ClaraFilesMgr.h
#ifndef SOURCE_GAME_GAMEMANAGERS_CLARAFILESMGR_H
#define SOURCE_GAME_GAMEMANAGERS_CLARAFILESMGR_H

#include <map>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "game/gameenums.h"

// Which clara files the game loads: the level, the libraries every location uses and, per location,
// the dynamic library file it needs. Locations in use are loaded, those no longer in use unloaded.
class ClaraFilesMgr : public Singleton<ClaraFilesMgr> {  // sizeof 0x78
public:
    // A dynamic library file: the libraries it brought, the location names it serves and its location id
    struct DynamicLibData {
        std::vector<jet::String> libraries;      // 0x0
        std::vector<jet::String> locationNames;  // 0xC
        int locationId;                          // 0x18
        DynamicLibData() : locationId(0) {}
    };
    ClaraFilesMgr();
    virtual ~ClaraFilesMgr() {}
    // the level file and the always loaded library files
    const std::vector<jet::String> &GetLevelFiles() const { return m_levelFiles; }
    const std::vector<jet::String> &GetLibFiles() const { return m_libFiles; }
    // the dynamic library file of a location
    jet::String LocationToDynamicLibFile(const jet::String &name);
    jet::String LocationIdToDynamicLibFile(const ELocation &id);
    // starts a new list of the locations in use
    void ClearUsedLocations() { m_usedFiles.clear(); m_filesToLoad.clear(); m_filesToUnload.clear(); }
    void AddUsedLocation(const jet::String &name);
    void AddUsedLocation(const ELocation &id);
    // what to load and unload to go from the locations in use before to the ones now
    void EndUpdateUsedLocations();
    // loads the files marked to load; false when none were
    bool LoadLibraries();
    // moves the libraries of the files marked to unload to the ones to unload one by one
    bool UnloadLibraries();
    // unloads up to maxLibs libraries every every ms; 0 nothing to unload, 1 working, 2 done
    int UpdateUnloadingLibraries(int dt, int maxLibs, int every);
    void Reset();
    void StartLoadingDynamicLibFile(const jet::String &file);
    void EndLoadingDynamicLibFile(const jet::String &file);
    void OnDynamicLibFileUnloaded(const jet::String &file);
private:
    std::vector<jet::String> m_levelFiles;         // 0x4
    std::vector<jet::String> m_libFiles;           // 0x10
    std::map<jet::String, DynamicLibData> m_dynamicLibs;  // 0x1C
    unsigned m_libCountBefore;                     // 0x34
    std::vector<jet::String> m_loadedFiles;        // 0x38
    std::vector<jet::String> m_usedFiles;          // 0x44
    std::vector<jet::String> m_filesToLoad;        // 0x50
    std::vector<jet::String> m_filesToUnload;      // 0x5C
    std::vector<jet::String> m_libsToUnload;       // 0x68
    int m_unloadTimer;                             // 0x74
};

#endif

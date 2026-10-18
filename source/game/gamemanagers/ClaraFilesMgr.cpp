// source/game/gamemanagers/ClaraFilesMgr.cpp
#include "ClaraFilesMgr.h"
#include "LocationsMgr.h"
#include "../leveltemplate/LevelTemplateMgr.h"
#include "game/GameLevel.h"
#include "game/entities/GameEntityCreator.h"
#include "project.h"
#include "stream/streammgr.h"
#include <algorithm>

typedef ClaraFilesMgr::DynamicLibData DynLib;

static bool Has(const std::vector<jet::String> &v, const jet::String &s) {
    for (const jet::String &x : v)
        if (x.Equals(s)) return true;
    return false;
}

static void Erase(std::vector<jet::String> &v, const jet::String &s) {
    for (size_t i = 0; i < v.size(); i++)
        if (v[i].Equals(s)) {
            v.erase(v.begin() + i);
            return;
        }
}

// @0x4c421c
ClaraFilesMgr::ClaraFilesMgr() : m_libCountBefore(0), m_unloadTimer(0) {
    m_levelFiles.push_back(jet::String("level.bclara"));
    m_libFiles.push_back(jet::String("designlib.blibclara"));
    struct { const char *file, *name; ELocationDef::type id; } k[] = {
        { "lablib.blibclara", "labo", ELocationDef::LABO_ROOM },        { "reslib.blibclara", "resi", ELocationDef::RESIDENTIAL },
        { "macholib.blibclara", "macho", ELocationDef::MACHO_LAB },     { "beachlib.blibclara", "beach", ELocationDef::MINIONS_BEACH },
        { "rntlib.blibclara", "rnt", ELocationDef::RESI_NIGHT },        { "malllib.blibclara", "mall", ELocationDef::MALL } };
    for (auto &e : k) {
        DynLib &d = m_dynamicLibs[jet::String(e.file)];
        d.locationNames.push_back(jet::String(e.name));
        if (e.id == ELocationDef::RESIDENTIAL) d.locationNames.push_back(jet::String("house"));
        d.locationId = e.id;
    }
}

// @0x4c4740: the file of the location whose name the given text contains
jet::String ClaraFilesMgr::LocationToDynamicLibFile(const jet::String &name) {
    for (auto &f : m_dynamicLibs)
        for (const jet::String &n : f.second.locationNames)
            if (name.find(n, 0) != (unsigned)-1) return f.first;
    return jet::String::null;
}

// @0x4c47cc
jet::String ClaraFilesMgr::LocationIdToDynamicLibFile(const ELocation &id) {
    for (auto &f : m_dynamicLibs)
        if (f.second.locationId == (int)id.underlying()) return f.first;
    return jet::String::null;
}

// @0x4c48ac: the libraries of a file are told by how many there were before it
void ClaraFilesMgr::StartLoadingDynamicLibFile(const jet::String &file) {
    DynLib &d = m_dynamicLibs[file];
    if (d.locationNames.empty()) return;
    d.libraries.clear();
    m_libCountBefore = clara::Project::GetInstance()->GetLibraryCount();
}

// @0x4c4958
void ClaraFilesMgr::EndLoadingDynamicLibFile(const jet::String &file) {
    DynLib &d = m_dynamicLibs[file];
    if (d.locationNames.empty()) return;
    clara::Project *p = clara::Project::GetInstance();
    for (unsigned i = m_libCountBefore; i < p->GetLibraryCount(); i++) {
        const jet::String &name = p->GetLibrary(i)->GetRoot().GetName();
        d.libraries.push_back(name);
        Erase(m_libsToUnload, name);
    }
    m_loadedFiles.push_back(file);
}

// @0x4c4a5c
void ClaraFilesMgr::OnDynamicLibFileUnloaded(const jet::String &file) {
    if (m_dynamicLibs[file].locationNames.empty()) return;
    Erase(m_loadedFiles, file);
}

// @0x4c4ac8: every `every` ms up to maxLibs libraries of the files to unload are unloaded
int ClaraFilesMgr::UpdateUnloadingLibraries(int dt, int maxLibs, int every) {
    if (m_libsToUnload.empty()) return 0;
    int left = m_unloadTimer - dt;
    if (left >= 1 && every >= 0) {
        m_unloadTimer = left;
        return 1;
    }
    m_unloadTimer = every;
    int n = (int)m_libsToUnload.size();
    if (maxLibs > 0 && maxLibs <= n) n = maxLibs;
    std::vector<jet::String> done;
    for (int i = 0; i < n; i++) {
        jet::String name = m_libsToUnload.front();
        clara::Project::GetInstance()->UnloadLibrary(name);
        done.push_back(name);
        m_libsToUnload.erase(m_libsToUnload.begin());
    }
    return m_libsToUnload.empty() ? 2 : 1;
}

// @0x4c4c04
bool ClaraFilesMgr::LoadLibraries() {
    if (m_filesToLoad.empty()) return false;
    GameLevel::GetInstance()->SetLoadingLibs(true);
    clara::Project *p = clara::Project::GetInstance();
    static GameEntityCreator creator;
    for (const jet::String &f : m_filesToLoad) {
        jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(f);
        StartLoadingDynamicLibFile(f);
        p->AddLibraries(*s, creator);
        EndLoadingDynamicLibFile(f);
    }
    GameLevel::GetInstance()->SetLoadingLibs(false);
    p->Init();
    p->PostInit();
    LevelTemplateMgr::BuildLevelTemplates(true);
    LocationsMgr::GetInstance()->Reset();
    m_filesToLoad.clear();
    return true;
}

// @0x4c4d0c
bool ClaraFilesMgr::UnloadLibraries() {
    if (m_filesToUnload.empty()) return false;
    for (const jet::String &f : m_filesToUnload) {
        std::vector<jet::String> libs = m_dynamicLibs[f].libraries;
        for (const jet::String &l : libs)
            if (!Has(m_libsToUnload, l)) m_libsToUnload.push_back(l);
        OnDynamicLibFileUnloaded(f);
    }
    m_filesToUnload.clear();
    return true;
}

// @0x4c4dcc
void ClaraFilesMgr::Reset() {
    m_loadedFiles.clear();
    m_usedFiles.clear();
    m_filesToLoad.clear();
    m_filesToUnload.clear();
    m_libsToUnload.clear();
    m_unloadTimer = 0;
}

// @0x5018d4
void ClaraFilesMgr::AddUsedLocation(const jet::String &name) {
    jet::String f = LocationToDynamicLibFile(name);
    if (!Has(m_usedFiles, f)) m_usedFiles.push_back(f);
}

// @0x501964
void ClaraFilesMgr::AddUsedLocation(const ELocation &id) {
    jet::String f = LocationIdToDynamicLibFile(id);
    if (!Has(m_usedFiles, f)) m_usedFiles.push_back(f);
}

// @0x5019f4: the used files not loaded yet are loaded, the loaded ones no longer used unloaded
void ClaraFilesMgr::EndUpdateUsedLocations() {
    for (const jet::String &f : m_usedFiles)
        if (!Has(m_loadedFiles, f)) m_filesToLoad.push_back(f);
    for (const jet::String &f : m_loadedFiles)
        if (!Has(m_usedFiles, f)) m_filesToUnload.push_back(f);
    m_usedFiles.clear();
}

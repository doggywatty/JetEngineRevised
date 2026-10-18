// core/claraloader/include/project.h
#ifndef CORE_CLARALOADER_INCLUDE_PROJECT_H
#define CORE_CLARALOADER_INCLUDE_PROJECT_H

#include <memory>
#include <unordered_map>
#include "clarafile.h"
#include "entity.h"
#include "jsingleton.h"

namespace clara {

// Owns the loaded libraries (types, templates, library files) and the current level file.
class Project : public Singleton<Project> {
public:
    Project();
    virtual ~Project();

    // Library stream: u32 version, u16 types, u16 templates, u16 files.
    void LoadLibraries(jet::stream::IStream& s, const Creator& c);
    // AddLibraries @0x698b10: another library pack; types and templates already known by name
    // are kept, the file's own are only added when new.
    void AddLibraries(jet::stream::IStream& s, const Creator& c);
    void UnloadLibraries();
    // The library whose root folder is called name goes (@0x61c3a4)
    void UnloadLibrary(const jet::String& name);
    // The level, the libraries, the types and the templates (@0x61c53c)
    void UnloadAll();
    void LoadClaraFile(jet::stream::IStream& s, const Creator& c);
    void UnloadClaraFile();

    void Init();
    bool Init(unsigned& cursor, unsigned budgetMs);
    void PostInit();
    bool PostInit(unsigned& cursor, unsigned budgetMs);
    void Update(int dtMs);

    const Creator* GetCreator() const { return m_creator; }
    unsigned GetNewId() const { return ++m_lastId; }
    // While a pack loads, type indices are that file's own (m_fileTypes)
    const Type* GetType(unsigned i) const
    {
        if (!m_fileTypes.empty()) return i < m_fileTypes.size() ? m_fileTypes[i] : nullptr;
        return i < m_types.size() ? m_types[i].get() : nullptr;
    }
    unsigned GetTypeCount() const { return unsigned(m_types.size()); }
    const Type* FindTypeByName(const jet::String& n) const;
    const Template* FindTemplateByName(const jet::String& n) const;
    unsigned GetTemplateCount() const { return unsigned(m_templates.size()); }
    const Template* GetTemplate(unsigned i) const { return m_templates[i].get(); }

    DataEntity* FindEntityByName(const jet::String& n) const;
    DataEntity* FindEntityById(unsigned id) const;
    DataEntity* FindEntityByPath(const Path& p) const;
    DataEntity* FindEntityByPath(ClaraFile& f, const Path& p) const;
    Folder* FindFolderByName(const jet::String& n) const;
    Group* FindGroupByName(const jet::String& n) const;
    Group* FindGroupById(unsigned id) const;
    Movie* FindMovieByName(const jet::String& n) const;
    Movie* FindMovieByPath(const Path& p) const;
    Movie* FindMovieByPath(ClaraFile& f, const Path& p) const;
    unsigned GetEntityCount() const { return unsigned(m_entities.size()); }
    DataEntity* GetEntity(unsigned i) const { return m_entities[i]; }
    ClaraFile* GetClaraFile() const { return m_claraFile.get(); }
    unsigned GetLibraryCount() const { return unsigned(m_libraries.size()); }
    ClaraFile* GetLibrary(unsigned i) const { return m_libraries[i].get(); }

private:
    void AddNodes(ClaraFile& f);
    void AddNodes(ClaraFile& f, Folder& folder);
    void AddNodes(ClaraFile& f, DataEntity& e);
    void RemoveNodes(ClaraFile& f);
    void RemoveNodes(Folder& folder);
    void RemoveNodes(DataEntity& e);
    template <class T, class F>
    T* FindInFolders(const ClaraFile& f, const Path& p, F pick) const;

    std::unordered_map<jet::String, unsigned> m_typeNames;
    std::unordered_map<jet::String, unsigned> m_templateNames;
    std::vector<std::unique_ptr<Type>> m_types;
    std::vector<std::unique_ptr<Template>> m_templates;
    std::vector<std::unique_ptr<ClaraFile>> m_libraries;
    std::unique_ptr<ClaraFile> m_claraFile;
    std::vector<DataEntity*> m_entities;
    std::unordered_multimap<jet::String, DataEntity*> m_entityNames;
    std::unordered_map<unsigned, DataEntity*> m_entityIds;
    std::vector<Folder*> m_folders;
    std::vector<Group*> m_groups;
    std::vector<Movie*> m_movies;
    const Creator* m_creator;
    mutable unsigned m_lastId;
    std::vector<const Type*> m_fileTypes;
};

}

#endif

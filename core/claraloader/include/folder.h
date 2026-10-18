// core/claraloader/include/folder.h
#ifndef CORE_CLARALOADER_INCLUDE_FOLDER_H
#define CORE_CLARALOADER_INCLUDE_FOLDER_H

#include <memory>
#include "group.h"
#include "movie.h"
#include "multilayer.h"

namespace clara {

class Folder {
public:
    Folder() {}
    Folder(const Folder&) = delete;
    Folder& operator=(const Folder&) = delete;
    ~Folder() { Unload(); }

    void Load(jet::stream::IStream& s, Project& p);
    void Unload();
    void Init();
    void PostInit();

    const jet::String& GetName() const { return m_name; }
    unsigned GetFolderCount() const { return unsigned(m_folders.size()); }
    Folder& GetFolder(unsigned i) const { return *m_folders[i]; }
    unsigned GetEntityCount() const { return unsigned(m_entities.size()); }
    DataEntity* GetEntity(unsigned i) const { return m_entities[i]; }
    unsigned GetGroupCount() const { return unsigned(m_groups.size()); }
    Group& GetGroup(unsigned i) const { return *m_groups[i]; }
    unsigned GetMovieCount() const { return unsigned(m_movies.size()); }
    Movie& GetMovie(unsigned i) const { return *m_movies[i]; }
    unsigned GetMultiLayerCount() const { return unsigned(m_multiLayers.size()); }
    MultiLayer& GetMultiLayer(unsigned i) const { return *m_multiLayers[i]; }
    DataEntity* FindEntityByName(const jet::String& n) const;
    Folder* FindFolderByName(const jet::String& n) const;
    Movie* FindMovieByName(const jet::String& n) const;

private:
    jet::String m_name;
    std::vector<std::unique_ptr<Folder>> m_folders;
    std::vector<std::unique_ptr<Group>> m_groups;
    std::vector<DataEntity*> m_entities;
    std::vector<std::unique_ptr<Movie>> m_movies;
    std::vector<std::unique_ptr<MultiLayer>> m_multiLayers;
};

}

#endif

// core/claraloader/include/group.h
#ifndef CORE_CLARALOADER_INCLUDE_GROUP_H
#define CORE_CLARALOADER_INCLUDE_GROUP_H

#include "clara.h"

namespace clara {

// A named list of entity paths, resolved by Init.
class Group {
public:
    Group() : m_id(0) {}
    void Load(jet::stream::IStream& s);
    void Init(Project& p, ClaraFile& f);
    void Unload() { m_paths.clear(); m_entities.clear(); }
    const jet::String& GetName() const { return m_name; }
    unsigned GetId() const { return m_id; }
    unsigned GetPathCount() const { return unsigned(m_paths.size()); }
    const Path& GetPath(unsigned i) const { return m_paths[i]; }
    unsigned GetEntityCount() const { return unsigned(m_entities.size()); }
    DataEntity* GetEntity(unsigned i) const { return m_entities[i]; }

private:
    jet::String m_name;
    unsigned m_id;
    std::vector<Path> m_paths;
    std::vector<DataEntity*> m_entities;
};

}

#endif

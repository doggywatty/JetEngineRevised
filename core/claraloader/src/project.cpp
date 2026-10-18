// core/claraloader/src/project.cpp
#include "project.h"

namespace clara {

Project::Project() : m_creator(nullptr), m_lastId(0) {}

Project::~Project()
{
    UnloadClaraFile();
    UnloadLibraries();
}

void Project::LoadLibraries(jet::stream::IStream& s, const Creator& c)
{
    s.Start();
    m_types.clear();
    m_templates.clear();
    m_typeNames.clear();
    m_templateNames.clear();
    u32 version = 0;
    u16 nt = 0, np = 0, nf = 0;
    s >> version >> nt;
    for (unsigned i = 0; i < nt; ++i) {
        m_types.emplace_back(new Type);
        m_types.back()->Load(s);
        m_typeNames[m_types.back()->GetName()] = i;
    }
    s >> np;
    for (unsigned i = 0; i < np; ++i) {
        m_templates.emplace_back(new Template);
        m_templates.back()->Load(s);
        m_templateNames[m_templates.back()->GetName()] = i;
    }
    m_creator = &c;
    s >> nf;
    for (unsigned i = 0; i < nf; ++i) {
        m_libraries.emplace_back(new ClaraFile(true));
        m_libraries.back()->Load(s, *this);
        AddNodes(*m_libraries.back());
    }
    s.Finish();
}

void Project::AddLibraries(jet::stream::IStream& s, const Creator& c)
{
    if (m_types.empty()) {
        LoadLibraries(s, c);
        return;
    }
    s.Start();
    u32 version = 0;
    u16 nt = 0, np = 0, nf = 0;
    s >> version >> nt;
    m_fileTypes.clear();
    for (unsigned i = 0; i < nt; ++i) {
        std::unique_ptr<Type> t(new Type);
        t->Load(s);
        auto it = m_typeNames.find(t->GetName());
        if (it != m_typeNames.end()) {
            m_fileTypes.push_back(m_types[it->second].get());
            continue;
        }
        m_typeNames[t->GetName()] = unsigned(m_types.size());
        m_fileTypes.push_back(t.get());
        m_types.push_back(std::move(t));
    }
    s >> np;
    for (unsigned i = 0; i < np; ++i) {
        std::unique_ptr<Template> t(new Template);
        t->Load(s);
        if (m_templateNames.find(t->GetName()) != m_templateNames.end()) continue;
        m_templateNames[t->GetName()] = unsigned(m_templates.size());
        m_templates.push_back(std::move(t));
    }
    m_creator = &c;
    s >> nf;
    for (unsigned i = 0; i < nf; ++i) {
        m_libraries.emplace_back(new ClaraFile(true));
        m_libraries.back()->Load(s, *this);
        AddNodes(*m_libraries.back());
    }
    m_fileTypes.clear();
    s.Finish();
}

void Project::UnloadLibraries()
{
    for (auto& f : m_libraries) {
        RemoveNodes(*f);
        f->Unload();
    }
    m_libraries.clear();
}

void Project::UnloadLibrary(const jet::String& name)
{
    for (size_t i = 0; i < m_libraries.size(); ++i)
        if (m_libraries[i]->GetRoot().GetName().Equals(name)) {
            RemoveNodes(*m_libraries[i]);
            m_libraries[i]->Unload();
            m_libraries.erase(m_libraries.begin() + i);
            return;
        }
}

void Project::UnloadAll()
{
    UnloadClaraFile();
    UnloadLibraries();
    m_types.clear();
    m_templates.clear();
    m_typeNames.clear();
    m_templateNames.clear();
}

void Project::LoadClaraFile(jet::stream::IStream& s, const Creator& c)
{
    UnloadClaraFile();
    s.Start();
    m_creator = &c;
    m_claraFile.reset(new ClaraFile(false));
    m_claraFile->Load(s, *this);
    AddNodes(*m_claraFile);
    s.Finish();
}

void Project::UnloadClaraFile()
{
    if (!m_claraFile)
        return;
    RemoveNodes(*m_claraFile);
    m_claraFile->Unload();
    m_claraFile.reset();
}

void Project::AddNodes(ClaraFile& f) { AddNodes(f, f.GetRoot()); }

void Project::AddNodes(ClaraFile& f, Folder& folder)
{
    m_folders.push_back(&folder);
    for (unsigned i = 0; i < folder.GetGroupCount(); ++i)
        m_groups.push_back(&folder.GetGroup(i));
    for (unsigned i = 0; i < folder.GetMovieCount(); ++i)
        m_movies.push_back(&folder.GetMovie(i));
    for (unsigned i = 0; i < folder.GetEntityCount(); ++i) {
        DataEntity* e = folder.GetEntity(i);
        e->SetClaraFile(&f);
        m_entities.push_back(e);
        if (e->IsSearchable())
            m_entityNames.emplace(e->GetName(), e);
        m_entityIds[e->GetId()] = e;
        AddNodes(f, *e);
    }
    for (unsigned i = 0; i < folder.GetFolderCount(); ++i)
        AddNodes(f, folder.GetFolder(i));
}

// Entities nested in ENTITY parameters are registered too.
void Project::AddNodes(ClaraFile& f, DataEntity& e)
{
    for (unsigned i = 0; i < e.GetParamCount(); ++i) {
        Param* p = e.GetParam(i);
        if (!p->GetType() || p->GetType()->GetValueType() != Type::ENTITY)
            continue;
        for (unsigned c = 0; c < p->GetComponentCount(); ++c) {
            DataEntity* n = p->GetAsEntity(c);
            if (!n)
                continue;
            n->SetClaraFile(&f);
            if (n->IsSearchable())
                m_entityNames.emplace(n->GetName(), n);
            m_entityIds[n->GetId()] = n;
            AddNodes(f, *n);
        }
    }
}

template <class V, class T>
static void Erase(V& v, T* x)
{
    for (auto it = v.begin(); it != v.end(); ++it)
        if (*it == x) {
            v.erase(it);
            return;
        }
}

void Project::RemoveNodes(ClaraFile& f) { RemoveNodes(f.GetRoot()); }

void Project::RemoveNodes(Folder& folder)
{
    Erase(m_folders, &folder);
    for (unsigned i = 0; i < folder.GetGroupCount(); ++i)
        Erase(m_groups, &folder.GetGroup(i));
    for (unsigned i = 0; i < folder.GetMovieCount(); ++i)
        Erase(m_movies, &folder.GetMovie(i));
    for (unsigned i = 0; i < folder.GetEntityCount(); ++i) {
        DataEntity* e = folder.GetEntity(i);
        Erase(m_entities, e);
        RemoveNodes(*e);
    }
    for (unsigned i = 0; i < folder.GetFolderCount(); ++i)
        RemoveNodes(folder.GetFolder(i));
}

void Project::RemoveNodes(DataEntity& e)
{
    auto r = m_entityNames.equal_range(e.GetName());
    for (auto it = r.first; it != r.second; ++it)
        if (it->second == &e) {
            m_entityNames.erase(it);
            break;
        }
    m_entityIds.erase(e.GetId());
    for (unsigned i = 0; i < e.GetParamCount(); ++i) {
        Param* p = e.GetParam(i);
        if (p->GetType() && p->GetType()->GetValueType() == Type::ENTITY)
            for (unsigned c = 0; c < p->GetComponentCount(); ++c)
                if (DataEntity* n = p->GetAsEntity(c))
                    RemoveNodes(*n);
    }
}

void Project::Init()
{
    unsigned c = 0;
    while (!Init(c, 0)) {
    }
}

// Resumable: returns true when done. A budget of 0 runs to completion.
bool Project::Init(unsigned& cursor, unsigned)
{
    for (; cursor < m_entities.size(); ++cursor)
        m_entities[cursor]->Init();
    for (Group* g : m_groups)
        if (m_claraFile)
            g->Init(*this, *m_claraFile);
    return true;
}

void Project::PostInit()
{
    unsigned c = 0;
    while (!PostInit(c, 0)) {
    }
}

bool Project::PostInit(unsigned& cursor, unsigned)
{
    if (cursor == 0)
        for (Movie* m : m_movies)
            m->PostInit(*this);
    for (; cursor < m_entities.size(); ++cursor)
        if (!(m_entities[cursor]->m_flags & DataEntity::F_POSTINIT))
            m_entities[cursor]->PostInit();
    return true;
}

void Project::Update(int dtMs)
{
    for (Movie* m : m_movies)
        m->Update(dtMs);
}

const Type* Project::FindTypeByName(const jet::String& n) const
{
    auto it = m_typeNames.find(n);
    return it == m_typeNames.end() ? nullptr : m_types[it->second].get();
}

const Template* Project::FindTemplateByName(const jet::String& n) const
{
    auto it = m_templateNames.find(n);
    return it == m_templateNames.end() ? nullptr : m_templates[it->second].get();
}

DataEntity* Project::FindEntityByName(const jet::String& n) const
{
    auto it = m_entityNames.find(n);
    return it == m_entityNames.end() ? nullptr : it->second;
}

DataEntity* Project::FindEntityById(unsigned id) const
{
    auto it = m_entityIds.find(id);
    return it == m_entityIds.end() ? nullptr : it->second;
}

// Walks folders from the file root; the rest of the path names an entity and then nested components.
DataEntity* Project::FindEntityByPath(ClaraFile& f, const Path& p) const
{
    const Folder* folder = &f.GetRoot();
    unsigned i = 0;
    if (p.GetCount() && folder->GetName() == p.Get(0) && !folder->FindFolderByName(p.Get(0)) && !folder->FindEntityByName(p.Get(0)))
        i = 1;
    for (; i < p.GetCount(); ++i) {
        if (DataEntity* e = folder->FindEntityByName(p.Get(int(i))))
            return i + 1 < p.GetCount() ? e->FindComponentByRelativePath(p, i + 1) : e;
        folder = folder->FindFolderByName(p.Get(int(i)));
        if (!folder)
            return nullptr;
    }
    return nullptr;
}

DataEntity* Project::FindEntityByPath(const Path& p) const
{
    if (m_claraFile)
        if (DataEntity* e = FindEntityByPath(*m_claraFile, p))
            return e;
    for (auto& l : m_libraries)
        if (DataEntity* e = FindEntityByPath(*l, p))
            return e;
    return p.GetCount() == 1 ? FindEntityByName(p.Get(0)) : nullptr;
}

Folder* Project::FindFolderByName(const jet::String& n) const
{
    for (Folder* f : m_folders)
        if (f->GetName() == n)
            return f;
    return nullptr;
}

Group* Project::FindGroupByName(const jet::String& n) const
{
    for (Group* g : m_groups)
        if (g->GetName() == n)
            return g;
    return nullptr;
}

Group* Project::FindGroupById(unsigned id) const
{
    for (Group* g : m_groups)
        if (g->GetId() == id)
            return g;
    return nullptr;
}

Movie* Project::FindMovieByName(const jet::String& n) const
{
    for (Movie* m : m_movies)
        if (m->GetName() == n)
            return m;
    return nullptr;
}

Movie* Project::FindMovieByPath(ClaraFile& f, const Path& p) const
{
    const Folder* folder = &f.GetRoot();
    for (unsigned i = 0; folder && i < p.GetCount(); ++i) {
        if (i + 1 == p.GetCount())
            return folder->FindMovieByName(p.Get(int(i)));
        const Folder* next = folder->FindFolderByName(p.Get(int(i)));
        if (!next && i == 0 && folder->GetName() == p.Get(0))
            continue;
        folder = next;
    }
    return nullptr;
}

Movie* Project::FindMovieByPath(const Path& p) const
{
    if (m_claraFile)
        if (Movie* m = FindMovieByPath(*m_claraFile, p))
            return m;
    for (auto& l : m_libraries)
        if (Movie* m = FindMovieByPath(*l, p))
            return m;
    return nullptr;
}

}

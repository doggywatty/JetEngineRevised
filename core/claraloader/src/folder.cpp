// core/claraloader/src/folder.cpp
#include "folder.h"
#include <cstdio>
#include "entity.h"
#include "project.h"

namespace clara {

// name, u16 folders, u16 entities, u16 groups, u16 movies, u16 multilayers, u16 (unused),
// then that many nodes, each a tag byte: 'f' folder, 'e' entity, 'g' group, 'm' movie, 'u' multilayer.
void Folder::Load(jet::stream::IStream& s, Project& p)
{
    jet::mem::Tag tag("clara");
    u16 nf = 0, ne = 0, ng = 0, nm = 0, nu = 0, unused = 0;
    s >> m_name >> nf >> ne >> ng >> nm >> nu >> unused;
    m_folders.reserve(nf);
    m_entities.reserve(ne);
    m_groups.reserve(ng);
    m_movies.reserve(nm);
    m_multiLayers.reserve(nu);
    unsigned total = unsigned(nf) + ne + ng + nm + nu;
    for (unsigned i = 0; i < total; ++i) {
        u8 t = 0;
        s >> t;
        switch (t) {
        case 'f':
            m_folders.emplace_back(new Folder);
            m_folders.back()->Load(s, p);
            break;
        case 'e': {
            jet::String tname;
            u32 size = 0;
            s >> tname >> size;
            const Template* tp = p.FindTemplateByName(tname);
            DataEntity* e = tp && p.GetCreator() ? p.GetCreator()->CreateEntity(tp) : nullptr;
            if (!e) {
                s.SeekCur(int(size));
                break;
            }
            e->Load(s, p);
            m_entities.push_back(e);
            break;
        }
        case 'm':
            m_movies.emplace_back(new Movie);
            m_movies.back()->Load(s);
            break;
        case 'g':
            m_groups.emplace_back(new Group);
            m_groups.back()->Load(s);
            break;
        case 'u':
            m_multiLayers.emplace_back(new MultiLayer);
            m_multiLayers.back()->Load(s);
            break;
        default:
            printf("Devs: Illegal node type found in folder '%s' - '%c'\n", m_name.c_str(), t);
        }
    }
}

void Folder::Unload()
{
    for (DataEntity* e : m_entities) {
        e->Unload();
        delete e;
    }
    m_entities.clear();
    m_folders.clear();
    m_groups.clear();
    m_movies.clear();
    m_multiLayers.clear();
}

void Folder::Init()
{
    for (auto& f : m_folders)
        f->Init();
    for (DataEntity* e : m_entities)
        e->Init();
}

void Folder::PostInit()
{
    for (auto& f : m_folders)
        f->PostInit();
    for (DataEntity* e : m_entities)
        e->PostInit();
}

DataEntity* Folder::FindEntityByName(const jet::String& n) const
{
    for (DataEntity* e : m_entities)
        if (e->GetName() == n)
            return e;
    return nullptr;
}

Folder* Folder::FindFolderByName(const jet::String& n) const
{
    for (auto& f : m_folders)
        if (f->GetName() == n)
            return f.get();
    return nullptr;
}

Movie* Folder::FindMovieByName(const jet::String& n) const
{
    for (auto& m : m_movies)
        if (m->GetName() == n)
            return m.get();
    return nullptr;
}

}

// core/claraloader/src/group.cpp
#include "group.h"
#include "mem/memorymanager.h"
#include "project.h"

namespace clara {

// name, u32 count, count entity paths.
void Group::Load(jet::stream::IStream& s)
{
    jet::mem::Tag tag("clara");
    u32 n = 0;
    s >> m_name >> n;
    m_paths.resize(n);
    for (Path& p : m_paths)
        s >> p;
    if (Project::GetInstance())
        m_id = Project::GetInstance()->GetNewId();
}

void Group::Init(Project& p, ClaraFile& f)
{
    m_entities.clear();
    for (const Path& path : m_paths)
        if (DataEntity* e = p.FindEntityByPath(f, path))
            m_entities.push_back(e);
}

}

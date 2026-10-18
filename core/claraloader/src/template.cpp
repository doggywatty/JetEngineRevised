// core/claraloader/src/template.cpp
#include "template.h"
#include "project.h"

namespace clara {

Template::Template() : m_id(Project::GetInstance() ? Project::GetInstance()->GetNewId() : 0), m_isInternal(false) {}

// name, u8 internal, u32 count, count x (name, u32 type index).
void Template::Load(jet::stream::IStream& s)
{
    u8 internal = 0;
    u32 n = 0;
    s >> m_name >> internal >> n;
    m_isInternal = internal != 0;
    m_params.resize(n);
    for (Param& p : m_params) {
        u32 t = 0;
        s >> p.name >> t;
        p.type = Project::GetInstance()->GetType(t);
    }
}

int Template::FindParam(const jet::String& name) const
{
    for (size_t i = 0; i < m_params.size(); ++i)
        if (m_params[i].name == name)
            return int(i);
    return -1;
}

}

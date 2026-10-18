// core/claraloader/src/clarafile.cpp
#include "clarafile.h"

namespace clara {

void ClaraFile::Load(jet::stream::IStream& s, Project& p)
{
    u8 unused = 0;
    s >> m_version >> m_name >> unused;
    m_root.Load(s, p);
}

}

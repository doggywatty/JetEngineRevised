// core/claraloader/src/multilayer.cpp
#include "multilayer.h"
#include "mem/memorymanager.h"

namespace clara {

// name, u32 layers, layer paths, u32 entries, entry paths, then layers x entries u32 values.
void MultiLayer::Load(jet::stream::IStream& s)
{
    jet::mem::Tag tag("clara");
    u32 nl = 0, ne = 0;
    s >> m_name >> nl;
    m_layers.resize(nl);
    for (Layer& l : m_layers)
        s >> l.path;
    s >> ne;
    m_entries.resize(ne);
    for (Path& p : m_entries)
        s >> p;
    for (Layer& l : m_layers) {
        l.values.resize(ne);
        for (u32& v : l.values)
            s >> v;
    }
}

}

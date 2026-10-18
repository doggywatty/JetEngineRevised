// core/claraloader/src/type.cpp
#include "type.h"

namespace clara {

// u16 value type, name, u8 scalar storage, u16 count, count enum names.
void Type::Load(jet::stream::IStream& s)
{
    u16 vt = 0, n = 0;
    u8 st = 0;
    s >> vt >> m_name >> st >> n;
    m_valueType = vt;
    m_scalarType = st;
    for (u16 i = 0; i < n; ++i) {
        jet::String e;
        s >> e;
        m_enumValues[e] = i;
    }
}

}

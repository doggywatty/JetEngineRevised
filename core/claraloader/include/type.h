// core/claraloader/include/type.h
#ifndef CORE_CLARALOADER_INCLUDE_TYPE_H
#define CORE_CLARALOADER_INCLUDE_TYPE_H

#include <unordered_map>
#include "clara.h"

namespace clara {

// A parameter type from a library: its value kind, and for enums the value names.
class Type {
public:
    enum ValueType {
        SCALAR = 2, ENUM = 4, NODE_REF = 8, SCRIPT = 16, ENTITY = 32, STATE = 64,
        VALUE = 128, VALUE_REF = 256, SPRITE = 512, STRING = 1024, ID_ENUM = 2048
    };
    // Storage of SCALAR values.
    enum ScalarType { S_U8, S_U16, S_S32, S_F32, S_F64 };

    Type() : m_valueType(0), m_scalarType(0) {}
    Type(const Type&) = delete;
    Type& operator=(const Type&) = delete;

    void Load(jet::stream::IStream& s);
    unsigned GetValueType() const { return m_valueType; }
    unsigned GetScalarType() const { return m_scalarType; }
    const jet::String& GetName() const { return m_name; }
    unsigned GetEnumCount() const { return unsigned(m_enumValues.size()); }
    const std::unordered_map<jet::String, unsigned short>& GetEnumValues() const { return m_enumValues; }
    int GetEnumValue(const jet::String& name) const
    {
        auto it = m_enumValues.find(name);
        return it == m_enumValues.end() ? -1 : it->second;
    }

private:
    unsigned m_valueType;
    jet::String m_name;
    std::unordered_map<jet::String, unsigned short> m_enumValues;
    unsigned m_scalarType;
};

}

#endif

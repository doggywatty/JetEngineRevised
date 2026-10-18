// core/claraloader/src/param.cpp
#include "param.h"
#include "entity.h"
#include "project.h"

namespace clara {

namespace {
const jet::String& Empty() { return jet::String::null; }

template <class T>
T* NewArray(unsigned n) { return new T[n](); }

template <class T>
void DeleteArray(void* p) { delete[] static_cast<T*>(p); }
}

const jet::String& Param::GetName() const { return m_def ? m_def->name : Empty(); }

const jet::String& Param::GetComponentName(unsigned i) const { return i < m_names.size() ? m_names[i] : Empty(); }

void Param::Allocate(unsigned n, bool named)
{
    if (!GetType())
        return;
    if (!n)
        n = 1;
    Destroy();
    m_count = u16(n);
    if (named)
        m_names.assign(n, jet::String());
    switch (GetType()->GetValueType()) {
    case Type::SCALAR:
        switch (GetType()->GetScalarType()) {
        case Type::S_U8: m_data = NewArray<u8>(n); break;
        case Type::S_U16: m_data = NewArray<u16>(n); break;
        case Type::S_S32: m_data = NewArray<s32>(n); break;
        case Type::S_F32: m_data = NewArray<float>(n); break;
        case Type::S_F64: m_data = NewArray<double>(n); break;
        }
        break;
    case Type::ENUM: m_data = NewArray<EnumValue>(n); break;
    case Type::NODE_REF: m_data = NewArray<Path>(n); break;
    case Type::SCRIPT: m_data = NewArray<std::string>(n); break;
    case Type::ENTITY: m_data = NewArray<DataEntity*>(n); break;
    case Type::STATE: m_data = NewArray<StateValue>(n); break;
    case Type::VALUE: m_data = NewArray<Vec>(n); break;
    case Type::VALUE_REF: m_data = NewArray<ValueRef>(n); break;
    case Type::SPRITE: m_data = NewArray<SpriteValue>(n); break;
    case Type::STRING: m_data = NewArray<jet::String>(n); break;
    case Type::ID_ENUM: m_data = NewArray<IdEnumValue>(n); break;
    }
}

void Param::Destroy()
{
    if (m_data && GetType()) {
        switch (GetType()->GetValueType()) {
        case Type::SCALAR:
            switch (GetType()->GetScalarType()) {
            case Type::S_U8: DeleteArray<u8>(m_data); break;
            case Type::S_U16: DeleteArray<u16>(m_data); break;
            case Type::S_S32: DeleteArray<s32>(m_data); break;
            case Type::S_F32: DeleteArray<float>(m_data); break;
            case Type::S_F64: DeleteArray<double>(m_data); break;
            }
            break;
        case Type::ENUM: DeleteArray<EnumValue>(m_data); break;
        case Type::NODE_REF: DeleteArray<Path>(m_data); break;
        case Type::SCRIPT: DeleteArray<std::string>(m_data); break;
        case Type::ENTITY:
            for (unsigned i = 0; i < m_count; ++i)
                delete As<DataEntity*>()[i];
            DeleteArray<DataEntity*>(m_data);
            break;
        case Type::STATE: DeleteArray<StateValue>(m_data); break;
        case Type::VALUE: DeleteArray<Vec>(m_data); break;
        case Type::VALUE_REF: DeleteArray<ValueRef>(m_data); break;
        case Type::SPRITE: DeleteArray<SpriteValue>(m_data); break;
        case Type::STRING: DeleteArray<jet::String>(m_data); break;
        case Type::ID_ENUM: DeleteArray<IdEnumValue>(m_data); break;
        }
    }
    m_data = nullptr;
    m_count = 0;
    m_names.clear();
}

// u8 header: bit7 = u16 count follows (else low 6 bits are the count), bit6 = components are named.
void Param::Load(jet::stream::IStream& s, Project& p)
{
    u8 h = 0;
    u16 n = 0;
    s >> h;
    if (h & 0x80)
        s >> n;
    else
        n = h & 0x3f;
    Allocate(n, (h & 0x40) != 0);
    for (unsigned i = 0; i < n; ++i)
        LoadComponent(i, s, p);
}

void Param::LoadComponent(unsigned i, jet::stream::IStream& s, Project& p)
{
    if (!m_names.empty())
        s >> m_names[i];
    if (!GetType())
        return;
    switch (GetType()->GetValueType()) {
    case Type::SCALAR:
        switch (GetType()->GetScalarType()) {
        case Type::S_U8: s.Read1(As<u8>() + i); break;
        case Type::S_U16: s.Read2(As<u16>() + i); break;
        case Type::S_S32: s >> As<s32>()[i]; break;
        case Type::S_F32: s.Read4(As<float>() + i); break;
        case Type::S_F64: s >> As<double>()[i]; break;
        }
        break;
    case Type::ENUM: s >> As<EnumValue>()[i].value >> As<EnumValue>()[i].name; break;
    case Type::NODE_REF: s >> As<Path>()[i]; break;
    case Type::SCRIPT: s >> As<std::string>()[i]; break;
    case Type::ENTITY: {
        u8 present = 0, tag = 0;
        s >> present;
        if (!present)
            return;
        s >> tag;
        if (tag != 'e')
            return;
        jet::String tname;
        u32 size = 0;
        s >> tname >> size;
        const Template* t = p.FindTemplateByName(tname);
        DataEntity* e = t && p.GetCreator() ? p.GetCreator()->CreateEntity(t) : nullptr;
        As<DataEntity*>()[i] = e;
        if (!e) {
            s.SeekCur(int(size));
            return;
        }
        e->SetName(GetComponentName(i));
        e->Load(s, p);
        break;
    }
    case Type::STATE: s >> As<StateValue>()[i].a >> As<StateValue>()[i].b; break;
    case Type::VALUE: {
        u8 n = 0;
        s >> n;
        Vec& v = As<Vec>()[i];
        v.size = n > 4 ? 4 : n;
        s.Read(v.v, v.size * 4);
        if (n > 4)
            s.SeekCur((n - 4) * 4);
        break;
    }
    case Type::VALUE_REF: s >> As<ValueRef>()[i].path >> As<ValueRef>()[i].index; break;
    case Type::SPRITE: s >> As<SpriteValue>()[i].file >> As<SpriteValue>()[i].anim >> As<SpriteValue>()[i].frame; break;
    case Type::STRING: s >> As<jet::String>()[i]; break;
    case Type::ID_ENUM: s >> As<IdEnumValue>()[i].value >> As<IdEnumValue>()[i].name; break;
    }
}

void Param::Init()
{
    if (GetType() && GetType()->GetValueType() == Type::ENTITY)
        for (unsigned i = 0; i < m_count; ++i)
            if (DataEntity* e = As<DataEntity*>()[i])
                e->Init();
}

void Param::PostInit()
{
    if (GetType() && GetType()->GetValueType() == Type::ENTITY)
        for (unsigned i = 0; i < m_count; ++i)
            if (DataEntity* e = As<DataEntity*>()[i])
                e->PostInit();
}

double Param::GetAsDouble(unsigned i) const
{
    if (!m_data || i >= m_count || !GetType())
        return 0;
    switch (GetType()->GetValueType()) {
    case Type::SCALAR:
        switch (GetType()->GetScalarType()) {
        case Type::S_U8: return As<u8>()[i];
        case Type::S_U16: return As<u16>()[i];
        case Type::S_S32: return As<s32>()[i];
        case Type::S_F32: return As<float>()[i];
        case Type::S_F64: return As<double>()[i];
        }
        return 0;
    case Type::ENUM: return As<EnumValue>()[i].value;
    case Type::ID_ENUM: return As<IdEnumValue>()[i].value;
    case Type::VALUE: return As<Vec>()[i].v[0];
    }
    return 0;
}

#define JET_PARAM_GET(ret, name, vt, T, expr, dflt)                                        \
    ret Param::name(unsigned i) const                                                    \
    {                                                                                    \
        static const std::remove_const_t<std::remove_reference_t<ret>> d = dflt;         \
        if (!m_data || i >= m_count || !GetType() || GetType()->GetValueType() != vt)    \
            return d;                                                                    \
        const T& v = As<T>()[i];                                                         \
        return expr;                                                                     \
    }

JET_PARAM_GET(unsigned, GetAsEnum, Type::ENUM, EnumValue, v.value, 0u)
JET_PARAM_GET(const jet::String&, GetAsEnumName, Type::ENUM, EnumValue, v.name, jet::String())
JET_PARAM_GET(const Path&, GetAsPath, Type::NODE_REF, Path, v, Path())
JET_PARAM_GET(const std::string&, GetAsScript, Type::SCRIPT, std::string, v, std::string())
JET_PARAM_GET(const Param::StateValue&, GetAsState, Type::STATE, StateValue, v, StateValue())
JET_PARAM_GET(const Param::SpriteValue&, GetAsSprite, Type::SPRITE, SpriteValue, v, SpriteValue())
JET_PARAM_GET(const jet::String&, GetAsString, Type::STRING, jet::String, v, jet::String())
JET_PARAM_GET(const Param::ValueRef&, GetAsValueRef, Type::VALUE_REF, ValueRef, v, ValueRef())
JET_PARAM_GET(const Param::IdEnumValue&, GetAsIdEnum, Type::ID_ENUM, IdEnumValue, v, IdEnumValue())
#undef JET_PARAM_GET

DataEntity* Param::GetAsEntity(unsigned i) const
{
    if (!m_data || i >= m_count || !GetType() || GetType()->GetValueType() != Type::ENTITY)
        return nullptr;
    return As<DataEntity*>()[i];
}

unsigned Param::GetValueSize(unsigned i) const
{
    if (!m_data || i >= m_count || !GetType() || GetType()->GetValueType() != Type::VALUE)
        return 0;
    return As<Vec>()[i].size;
}

const float* Param::GetAsFloats(unsigned i) const
{
    static const float z[4] = {};
    if (!m_data || i >= m_count || !GetType() || GetType()->GetValueType() != Type::VALUE)
        return z;
    return As<Vec>()[i].v;
}

math::vec2<float> Param::GetAsVector2D(unsigned i) const { const float* f = GetAsFloats(i); return math::vec2<float>(f[0], f[1]); }
math::vec3<float> Param::GetAsVector3D(unsigned i) const { const float* f = GetAsFloats(i); return math::vec3<float>(f[0], f[1], f[2]); }
math::vec4<float> Param::GetAsVector4D(unsigned i) const
{
    const float* f = GetAsFloats(i);
    return math::vec4<float>(f[0], f[1], f[2], f[3]);
}

}

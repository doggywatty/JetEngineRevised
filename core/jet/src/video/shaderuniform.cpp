// core/jet/src/video/shaderuniform.cpp
#include "video/shaderuniform.h"
#include "video/driver.h"
#include <cstdio>
#include <cstring>

namespace jet::video {

unsigned ShaderUniform::ElementSize(safe_enum<TypeDef,unsigned char> t) {
    switch ((TypeDef::type)t) {
    case TypeDef::Int: case TypeDef::Float: return 4;
    case TypeDef::Vec2: return sizeof(math::vec2<float>);
    case TypeDef::Vec3: return sizeof(math::vec3<float>);
    case TypeDef::Vec4: return sizeof(math::vec4<float>);
    case TypeDef::Mat3: return sizeof(math::mat3<float>);
    case TypeDef::Mat4: return sizeof(math::mat4<float>);
    case TypeDef::Mat43: return sizeof(mat43f);
    case TypeDef::Mat32: return sizeof(mat32f);
    default: return 0;
    }
}

void ShaderUniform::CommonConstructor() {
    m_serverData = 0;
    m_clientData = 0;
    m_hasClientServerData = 0;
    m_type = TypeDef::None;
    m_dataVersion = 0;
    m_count = 0;
    m_data = 0;
}

ShaderUniform::ShaderUniform() { CommonConstructor(); }

void ShaderUniform::Allocate(safe_enum<TypeDef,unsigned char> t, unsigned short count) {
    m_type = t;
    m_count = count;
    m_storage.assign((size_t)ElementSize(t) * count, 0);
    m_data = m_storage.empty() ? 0 : &m_storage[0];
}

ShaderUniform::ShaderUniform(safe_enum<TypeDef,unsigned char> t, unsigned short count, const unsigned char *data) {
    CommonConstructor();
    Allocate(t, count);
    if (data && m_data) memcpy(m_data, data, m_storage.size());
}

ShaderUniform::ShaderUniform(const ShaderUniform &o) { CommonConstructor(); *this = o; }

ShaderUniform::ShaderUniform(int v) { CommonConstructor(); Allocate(TypeDef::Int, 1); memcpy(m_data, &v, 4); }
ShaderUniform::ShaderUniform(float v) { CommonConstructor(); Allocate(TypeDef::Float, 1); memcpy(m_data, &v, 4); }
ShaderUniform::ShaderUniform(const math::vec2<float> &v) { CommonConstructor(); Allocate(TypeDef::Vec2, 1); memcpy(m_data, &v, sizeof(v)); }
ShaderUniform::ShaderUniform(const math::vec3<float> &v) { CommonConstructor(); Allocate(TypeDef::Vec3, 1); memcpy(m_data, &v, sizeof(v)); }
ShaderUniform::ShaderUniform(const math::vec4<float> &v) { CommonConstructor(); Allocate(TypeDef::Vec4, 1); memcpy(m_data, &v, sizeof(v)); }
ShaderUniform::ShaderUniform(const math::mat3<float> &v) { CommonConstructor(); Allocate(TypeDef::Mat3, 1); memcpy(m_data, &v, sizeof(v)); }
ShaderUniform::ShaderUniform(const math::mat4<float> &v) { CommonConstructor(); Allocate(TypeDef::Mat4, 1); memcpy(m_data, &v, sizeof(v)); }
ShaderUniform::ShaderUniform(const mat43f &v) { CommonConstructor(); Allocate(TypeDef::Mat43, 1); memcpy(m_data, &v, sizeof(v)); }
ShaderUniform::ShaderUniform(const mat32f &v) { CommonConstructor(); Allocate(TypeDef::Mat32, 1); memcpy(m_data, &v, sizeof(v)); }

ShaderUniform::~ShaderUniform() {}

void ShaderUniform::Touch() { m_dataVersion++; }

bool ShaderUniform::operator==(const ShaderUniform &o) const {
    return m_type == o.m_type && m_count == o.m_count && m_storage == o.m_storage;
}
bool ShaderUniform::operator!=(const ShaderUniform &o) const { return !(*this == o); }

ShaderUniform &ShaderUniform::operator=(const ShaderUniform &o) {
    m_type = o.m_type;
    m_count = o.m_count;
    m_storage = o.m_storage;
    m_data = m_storage.empty() ? 0 : &m_storage[0];
    m_serverData = o.m_serverData;
    m_clientData = o.m_clientData;
    m_hasClientServerData = o.m_hasClientServerData;
    Touch();
    return *this;
}
ShaderUniform &ShaderUniform::operator=(int v) { Allocate(TypeDef::Int, 1); memcpy(m_data, &v, 4); Touch(); return *this; }
ShaderUniform &ShaderUniform::operator=(float v) { Allocate(TypeDef::Float, 1); memcpy(m_data, &v, 4); Touch(); return *this; }
ShaderUniform &ShaderUniform::operator=(const math::vec2<float> &v) { Allocate(TypeDef::Vec2, 1); memcpy(m_data, &v, sizeof(v)); Touch(); return *this; }
ShaderUniform &ShaderUniform::operator=(const math::vec3<float> &v) { Allocate(TypeDef::Vec3, 1); memcpy(m_data, &v, sizeof(v)); Touch(); return *this; }
ShaderUniform &ShaderUniform::operator=(const math::vec4<float> &v) { Allocate(TypeDef::Vec4, 1); memcpy(m_data, &v, sizeof(v)); Touch(); return *this; }
ShaderUniform &ShaderUniform::operator=(const math::mat3<float> &v) { Allocate(TypeDef::Mat3, 1); memcpy(m_data, &v, sizeof(v)); Touch(); return *this; }
ShaderUniform &ShaderUniform::operator=(const math::mat4<float> &v) { Allocate(TypeDef::Mat4, 1); memcpy(m_data, &v, sizeof(v)); Touch(); return *this; }
ShaderUniform &ShaderUniform::operator=(const mat43f &v) { Allocate(TypeDef::Mat43, 1); memcpy(m_data, &v, sizeof(v)); Touch(); return *this; }
ShaderUniform &ShaderUniform::operator=(const mat32f &v) { Allocate(TypeDef::Mat32, 1); memcpy(m_data, &v, sizeof(v)); Touch(); return *this; }

void ShaderUniform::SetAndClamp(const ShaderUniform &value, const ShaderUniform &lo, const ShaderUniform &hi) {
    *this = value;
    if (m_type != TypeDef::Float && m_type != TypeDef::Int) return;
    if (m_type == TypeDef::Float) {
        float v, l, h;
        memcpy(&v, m_data, 4); memcpy(&l, lo.m_data, 4); memcpy(&h, hi.m_data, 4);
        if (v < l) v = l;
        if (v > h) v = h;
        memcpy(m_data, &v, 4);
    } else {
        int v, l, h;
        memcpy(&v, m_data, 4); memcpy(&l, lo.m_data, 4); memcpy(&h, hi.m_data, 4);
        if (v < l) v = l;
        if (v > h) v = h;
        memcpy(m_data, &v, 4);
    }
    Touch();
}

void ShaderUniform::SetServerData(unsigned d) { m_serverData = d; m_hasClientServerData |= 2; }
unsigned ShaderUniform::GetServerData() const { return m_serverData; }
bool ShaderUniform::HasServerData() const { return (m_hasClientServerData & 2) != 0; }
void ShaderUniform::RemoveServerData() { m_hasClientServerData &= ~2; m_serverData = 0; }
void ShaderUniform::SetClientData(unsigned d) { m_clientData = d; m_hasClientServerData |= 1; }
unsigned ShaderUniform::GetClientData() const { return m_clientData; }
bool ShaderUniform::HasClientData() const { return (m_hasClientServerData & 1) != 0; }
void ShaderUniform::RemoveClientData() { m_hasClientServerData &= ~1; m_clientData = 0; }

unsigned ShaderUniform::GetDataVersion() const { return m_dataVersion; }
void ShaderUniform::SetDataVersion(unsigned v) { m_dataVersion = (unsigned short)v; }
safe_enum<ShaderUniform::TypeDef,unsigned char> ShaderUniform::GetType() const { return m_type; }
bool ShaderUniform::IsValid() const { return m_type != TypeDef::None; }

void ShaderUniform::ParseType(const jet::String &s) {
    unsigned bracket = s.find('[');
    jet::String base = (bracket == jet::String::npos) ? s : s.substr(0, bracket);
    TypeDef::type t;
    if (base.Equals("int")) t = TypeDef::Int;
    else if (base.Equals("float")) t = TypeDef::Float;
    else if (base.Equals("vec2")) t = TypeDef::Vec2;
    else if (base.Equals("vec3")) t = TypeDef::Vec3;
    else if (base.Equals("vec4")) t = TypeDef::Vec4;
    else if (base.Equals("mat3")) t = TypeDef::Mat3;
    else if (base.Equals("mat4")) t = TypeDef::Mat4;
    else if (base.Equals("mat43") || base.Equals("3vec4")) t = TypeDef::Mat43;
    else if (base.Equals("mat32") || base.Equals("2vec3")) t = TypeDef::Mat32;
    else t = TypeDef::None;

    unsigned count = 1;
    if (bracket != jet::String::npos) {
        jet::String num = s.substr(bracket + 1, s.size() - bracket - 2);
        count = atoi(num.c_str());
        if (count == 0) count = 1;
    }
    Allocate(safe_enum<TypeDef,unsigned char>(t), (unsigned short)count);
}

bool ShaderUniform::ParseInt(const char *s, int &v) { return s && *s && sscanf(s, "%d", &v) == 1; }
bool ShaderUniform::ParseFloat(const char *s, float &v) { return s && *s && sscanf(s, "%f", &v) == 1; }
bool ShaderUniform::ParseVec2(const char *s, math::vec2<float> &v) { return s && *s && sscanf(s, "%f,%f", &v.x, &v.y) == 2; }
bool ShaderUniform::ParseVec3(const char *s, math::vec3<float> &v) { return s && *s && sscanf(s, "%f,%f,%f", &v.x, &v.y, &v.z) == 3; }
bool ShaderUniform::ParseVec4(const char *s, math::vec4<float> &v) { return s && *s && sscanf(s, "%f,%f,%f,%f", &v.x, &v.y, &v.z, &v.w) == 4; }

// isDefault selects whether the shader's own default-value string is parsed (single value) or a
// per-material override; the original only self-parses scalars/vectors here, not matrices/arrays.
// ParseValue @0x6a8d5c: an engine value the driver can link first, else a literal of the type
bool ShaderUniform::ParseValue(const jet::String &s, bool allowLink) {
    if (allowLink) {
        Driver *d = Driver::GetCurrent();
        if (d && d->LinkUniform(*this, s)) return true;
    }
    if (m_count >= 2) return true;
    const char *c = s.c_str();
    bool empty = !c || !*c;
    switch ((TypeDef::type)m_type) {
    case TypeDef::Int: { int v = 0; if (empty) return true; bool ok = ParseInt(c, v); if (ok) memcpy(m_data, &v, 4); return ok; }
    case TypeDef::Float: { float v = 0; if (empty) return true; bool ok = ParseFloat(c, v); if (ok) memcpy(m_data, &v, 4); return ok; }
    case TypeDef::Vec2: { math::vec2<float> v; if (empty) return true; bool ok = ParseVec2(c, v); if (ok) memcpy(m_data, &v, sizeof(v)); return ok; }
    case TypeDef::Vec3: { math::vec3<float> v; if (empty) return true; bool ok = ParseVec3(c, v); if (ok) memcpy(m_data, &v, sizeof(v)); return ok; }
    case TypeDef::Vec4: { math::vec4<float> v; if (empty) return true; bool ok = ParseVec4(c, v); if (ok) memcpy(m_data, &v, sizeof(v)); return ok; }
    default: return true;  // Mat3/Mat4/Mat43/Mat32 aren't self-parsed from a plain string.
    }
}

// "v0;v1;v2;..." per-element values for array uniforms.
void ShaderUniform::ParseArrayValues(const jet::String &s) {
    unsigned start = 0, idx = 0;
    unsigned elemSize = ElementSize(m_type);
    while (idx < m_count) {
        unsigned sep = s.find(';', start);
        jet::String tok = (sep == jet::String::npos) ? s.substr(start) : s.substr(start, sep - start);
        unsigned char *dst = m_data + idx * elemSize;
        const char *c = tok.c_str();
        switch ((TypeDef::type)m_type) {
        case TypeDef::Int: { int v = 0; if (ParseInt(c, v)) memcpy(dst, &v, 4); break; }
        case TypeDef::Float: { float v = 0; if (ParseFloat(c, v)) memcpy(dst, &v, 4); break; }
        case TypeDef::Vec2: { math::vec2<float> v; if (ParseVec2(c, v)) memcpy(dst, &v, sizeof(v)); break; }
        case TypeDef::Vec3: { math::vec3<float> v; if (ParseVec3(c, v)) memcpy(dst, &v, sizeof(v)); break; }
        case TypeDef::Vec4: { math::vec4<float> v; if (ParseVec4(c, v)) memcpy(dst, &v, sizeof(v)); break; }
        default: break;
        }
        idx++;
        if (sep == jet::String::npos) break;
        start = sep + 1;
    }
    Touch();
}

unsigned ShaderUniform::GetCount() const { return m_count; }

int ShaderUniform::GetInt() const { int v = 0; memcpy(&v, m_data, 4); return v; }
float ShaderUniform::GetFloat() const { float v = 0; memcpy(&v, m_data, 4); return v; }
const math::mat3<float> &ShaderUniform::GetMat3() const { return *reinterpret_cast<const math::mat3<float>*>(m_data); }
const math::mat4<float> &ShaderUniform::GetMat4() const { return *reinterpret_cast<const math::mat4<float>*>(m_data); }
const math::vec2<float> &ShaderUniform::GetVec2() const { return *reinterpret_cast<const math::vec2<float>*>(m_data); }
const math::vec3<float> &ShaderUniform::GetVec3() const { return *reinterpret_cast<const math::vec3<float>*>(m_data); }
const math::vec4<float> &ShaderUniform::GetVec4() const { return *reinterpret_cast<const math::vec4<float>*>(m_data); }
const mat32f &ShaderUniform::GetMat32() const { return *reinterpret_cast<const mat32f*>(m_data); }
const mat43f &ShaderUniform::GetMat43() const { return *reinterpret_cast<const mat43f*>(m_data); }

bool ShaderUniform::EqualsData(const unsigned char *d) const { return memcmp(m_data, d, m_storage.size()) == 0; }
void ShaderUniform::SetData(const unsigned char *d) { memcpy(m_data, d, m_storage.size()); Touch(); }
const unsigned char *ShaderUniform::GetData() const { return m_data; }
unsigned ShaderUniform::GetDataSize() const { return (unsigned)m_storage.size(); }

#define JET_SU_GETSET(T) \
    void ShaderUniform::Get(unsigned idx, T *dst, unsigned n) const { memcpy(dst, m_data + idx * sizeof(T), n * sizeof(T)); } \
    void ShaderUniform::Set(unsigned idx, const T *src, unsigned n) { \
        if (idx + n > m_count) return; \
        memcpy(m_data + idx * sizeof(T), src, n * sizeof(T)); \
        Touch(); \
    }

JET_SU_GETSET(int)
JET_SU_GETSET(float)
JET_SU_GETSET(math::vec2<float>)
JET_SU_GETSET(math::vec3<float>)
JET_SU_GETSET(math::vec4<float>)
JET_SU_GETSET(math::mat3<float>)
JET_SU_GETSET(math::mat4<float>)
JET_SU_GETSET(mat32f)
JET_SU_GETSET(mat43f)

#undef JET_SU_GETSET

}

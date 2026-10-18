// core/claraloader/include/param.h
#ifndef CORE_CLARALOADER_INCLUDE_PARAM_H
#define CORE_CLARALOADER_INCLUDE_PARAM_H

#include <string>
#include "smath.h"
#include "template.h"

namespace clara {

// One parameter value of an entity: an array of components of the parameter's type,
// optionally each with a name.
class Param {
public:
    struct SpriteValue {
        jet::String file, anim, frame;
    };
    struct StateValue {
        jet::String a, b;
    };
    struct EnumValue {
        unsigned value = 0;
        jet::String name;
    };
    struct IdEnumValue {
        int value = 0;
        jet::String name;
    };
    struct ValueRef {
        Path path;
        unsigned index = 0;
    };

    Param() : m_def(nullptr), m_count(0), m_data(nullptr) {}
    ~Param() { Destroy(); }
    Param(const Param&) = delete;
    Param& operator=(const Param&) = delete;

    void SetParamDef(const Template::Param& d) { m_def = &d; }
    const Template::Param* GetParamDef() const { return m_def; }
    const Type* GetType() const { return m_def ? m_def->type : nullptr; }
    const jet::String& GetName() const;
    void Load(jet::stream::IStream& s, Project& p);
    void Init();
    void PostInit();

    unsigned GetComponentCount() const { return m_count; }
    const jet::String& GetComponentName(unsigned i) const;
    double GetAsDouble(unsigned i = 0) const;
    float GetAsFloat(unsigned i = 0) const { return float(GetAsDouble(i)); }
    int GetAsS32(unsigned i = 0) const { return int(GetAsDouble(i)); }
    unsigned GetAsU32(unsigned i = 0) const { return unsigned(GetAsDouble(i)); }
    unsigned GetAsEnum(unsigned i = 0) const;
    const jet::String& GetAsEnumName(unsigned i = 0) const;
    const Path& GetAsPath(unsigned i = 0) const;
    const std::string& GetAsScript(unsigned i = 0) const;
    DataEntity* GetAsEntity(unsigned i = 0) const;
    const StateValue& GetAsState(unsigned i = 0) const;
    const SpriteValue& GetAsSprite(unsigned i = 0) const;
    const jet::String& GetAsString(unsigned i = 0) const;
    unsigned GetValueSize(unsigned i = 0) const;
    const float* GetAsFloats(unsigned i = 0) const;
    math::vec2<float> GetAsVector2D(unsigned i = 0) const;
    math::vec3<float> GetAsVector3D(unsigned i = 0) const;
    math::vec4<float> GetAsVector4D(unsigned i = 0) const;
    const ValueRef& GetAsValueRef(unsigned i = 0) const;
    const IdEnumValue& GetAsIdEnum(unsigned i = 0) const;

private:
    struct Vec {
        unsigned size = 0;
        float v[4] = {};
    };

    void Allocate(unsigned n, bool named);
    void Destroy();
    void LoadComponent(unsigned i, jet::stream::IStream& s, Project& p);
    template <class T> T* As() const { return static_cast<T*>(m_data); }

    const Template::Param* m_def;
    unsigned short m_count;
    std::vector<jet::String> m_names;
    void* m_data;
};

}

#endif

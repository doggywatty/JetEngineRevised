// core/jet/include/video/shaderuniform.h
#ifndef CORE_JET_INCLUDE_VIDEO_SHADERUNIFORM_H
#define CORE_JET_INCLUDE_VIDEO_SHADERUNIFORM_H

#include <vector>
#include "jenum.h"
#include "smath.h"
#include "core/pstring.h"

namespace jet::video {

// GPU-friendly compressed matrices: a 4x3 (mat43f) or 3x2 (mat32f) affine transform stored as
// column vec4s/vec3s with the last row ([0,0,0,1]) implicit, used for skin/bone uniforms. Verified
// against the 0.0.3 .so: ShaderUniform::ParseType accepts "mat43"/"3vec4" as synonyms (3 vec4
// columns, 12 floats) and "mat32"/"2vec3" (2 vec3 columns, 6 floats).
struct mat43f { math::vec4<float> col[3]; };
struct mat32f { math::vec3<float> col[2]; };

class ShaderUniform {  // sizeof 0x60
public:
    // Verified against libdespicablemefree.so 0.0.3, ShaderUniform::ParseType's string table
    // (0x61f064: int/float/mat3/mat4/vec2/vec3/vec4/mat43(or "3vec4")/mat32(or "2vec3")).
    struct TypeDef {
        enum type { None, Int, Float, Mat3, Mat4, Vec2, Vec3, Vec4, Mat43, Mat32 };
    };

    ShaderUniform(const mat43f&);
    ShaderUniform(const mat32f&);
    ShaderUniform(const math::vec4<float>&);
    ShaderUniform(const math::vec3<float>&);
    ShaderUniform(const math::vec2<float>&);
    ShaderUniform(const math::mat4<float>&);
    ShaderUniform(const math::mat3<float>&);
    ShaderUniform(float);
    ShaderUniform(int);
    ShaderUniform(const ShaderUniform&);
    ShaderUniform(safe_enum<TypeDef,unsigned char>, unsigned short, const unsigned char* = 0);
    ShaderUniform();
    ~ShaderUniform();
    bool operator==(const ShaderUniform&) const;
    bool operator!=(const ShaderUniform&) const;
    ShaderUniform &operator=(const mat43f&);
    ShaderUniform &operator=(const mat32f&);
    ShaderUniform &operator=(const math::vec4<float>&);
    ShaderUniform &operator=(const math::vec3<float>&);
    ShaderUniform &operator=(const math::vec2<float>&);
    ShaderUniform &operator=(const math::mat4<float>&);
    ShaderUniform &operator=(const math::mat3<float>&);
    ShaderUniform &operator=(float);
    ShaderUniform &operator=(int);
    ShaderUniform &operator=(const ShaderUniform&);
    void SetAndClamp(const ShaderUniform&, const ShaderUniform&, const ShaderUniform&);
    void SetServerData(unsigned);
    unsigned GetServerData() const;
    bool HasServerData() const;
    void RemoveServerData();
    void SetClientData(unsigned);
    unsigned GetClientData() const;
    bool HasClientData() const;
    void RemoveClientData();
    unsigned GetDataVersion() const;
    void SetDataVersion(unsigned);
    safe_enum<TypeDef,unsigned char> GetType() const;
    bool IsValid() const;
    void ParseType(const jet::String&);
    bool ParseValue(const jet::String&, bool isDefault);
    void ParseArrayValues(const jet::String&);
    unsigned GetCount() const;
    int GetInt() const;
    float GetFloat() const;
    const math::mat3<float> &GetMat3() const;
    const math::mat4<float> &GetMat4() const;
    const math::vec2<float> &GetVec2() const;
    const math::vec3<float> &GetVec3() const;
    const math::vec4<float> &GetVec4() const;
    const mat32f &GetMat32() const;
    const mat43f &GetMat43() const;
    bool EqualsData(const unsigned char*) const;
    void SetData(const unsigned char*);
    const unsigned char *GetData() const;
    unsigned GetDataSize() const;
    void Get(unsigned, math::vec4<float>*, unsigned) const;
    void Get(unsigned, math::vec3<float>*, unsigned) const;
    void Get(unsigned, math::vec2<float>*, unsigned) const;
    void Get(unsigned, mat32f*, unsigned) const;
    void Get(unsigned, mat43f*, unsigned) const;
    void Get(unsigned, math::mat4<float>*, unsigned) const;
    void Get(unsigned, math::mat3<float>*, unsigned) const;
    void Get(unsigned, float*, unsigned) const;
    void Get(unsigned, int*, unsigned) const;
    void Set(unsigned, const math::vec4<float>*, unsigned);
    void Set(unsigned, const math::vec3<float>*, unsigned);
    void Set(unsigned, const math::vec2<float>*, unsigned);
    void Set(unsigned, const mat32f*, unsigned);
    void Set(unsigned, const mat43f*, unsigned);
    void Set(unsigned, const math::mat4<float>*, unsigned);
    void Set(unsigned, const math::mat3<float>*, unsigned);
    void Set(unsigned, const float*, unsigned);
    void Set(unsigned, const int*, unsigned);
protected:
    void CommonConstructor();
    void Allocate(safe_enum<TypeDef,unsigned char>, unsigned short);
    void Touch();
    static unsigned ElementSize(safe_enum<TypeDef,unsigned char>);
    bool ParseInt(const char*, int&);
    bool ParseFloat(const char*, float&);
    bool ParseVec2(const char*, math::vec2<float>&);
    bool ParseVec3(const char*, math::vec3<float>&);
    bool ParseVec4(const char*, math::vec4<float>&);
    unsigned m_serverData;
    unsigned m_clientData;
    unsigned char m_hasClientServerData;  // bit0=client, bit1=server
    safe_enum<TypeDef,unsigned char> m_type;
    unsigned short m_dataVersion;
    unsigned m_count;
    std::vector<unsigned char> m_storage;
    unsigned char* m_data;
};
}

#endif

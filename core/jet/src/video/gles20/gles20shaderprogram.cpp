// core/jet/src/video/gles20/gles20shaderprogram.cpp
#include "video/gles20/gles20shaderprogram.h"
#include "video/rendertechnique.h"
#include "pugixml.hpp"
#include "video/driver.h"
#include "core/util.h"
#include <cstdio>
#include <set>
#include <algorithm>
#include <cctype>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// GLSL ES sources are written against "#version 100" (GLES2's shading language). Real GLES2
// contexts (Android, iOS, Windows via ANGLE-less desktop GL2 extensions) accept that pragma
// verbatim; a desktop-GL context (this rebuild's macOS path, since there's no real GLES2/EGL on
// the Mac) does not understand it and rejects the shader outright, so on that one platform the
// pragma is dropped and GLSL uses 1.20 (sub-matrix constructors), compatible enough for
// the plain vertex-color/texture shaders this engine's auto-uniform naming expects.
#if defined(__APPLE__) && !TARGET_OS_IPHONE
#define JET_GLSL_VERSION_PRAGMA "#version 120\n"
#else
#define JET_GLSL_VERSION_PRAGMA "#version 100\n"
#endif

namespace jet::video {

GLES20ShaderProgram::GLES20ShaderProgram(RenderTechnique *tech)
    : ShaderProgram(tech), m_shaderData(0), m_autoShaderData(0) {
}
GLES20ShaderProgram::~GLES20ShaderProgram() {
    gles::Interface gl;
    for (std::map<int, GLES20ShaderProgramFlavor*>::iterator it = m_flavors.begin(); it != m_flavors.end(); ++it) {
        if (it->second->GetGLId()) gl.iglDeleteProgram(it->second->GetGLId());
        delete it->second;
    }
}

void GLES20ShaderProgram::SetShaderData(ShaderData *sd, ShaderData *autoSd) { m_shaderData = sd; m_autoShaderData = autoSd; }

void GLES20ShaderProgram::SetVertexSource(const std::string &s) { m_vshaderSrc = s; }
const std::string &GLES20ShaderProgram::GetVertexSource() { return m_vshaderSrc; }
void GLES20ShaderProgram::SetFragmentSource(const std::string &s) { m_fshaderSrc = s; }
const std::string &GLES20ShaderProgram::GetFragmentSource() { return m_fshaderSrc; }
const std::string &GLES20ShaderProgram::GetFlavorVertexSource(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>) { return m_vshaderSrc; }
const std::string &GLES20ShaderProgram::GetFlavorFragmentSource(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>) { return m_fshaderSrc; }

const char *GLES20ShaderProgram::FlavorDefine(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> t) {
    static const char *defs[] = { "", "#define HAS_SKINNING 1\n", "#define HAS_SKINNING 2\n", "#define HAS_SKINNING 3\n", "#define HAS_SKINNING 4\n" };
    unsigned i = (unsigned)(ShaderProgramFlavor::TypeDef::type)t;
    return i < 5 ? defs[i] : "";
}

// Hardware skinning node palette size (ModelBase::s_hwSkinningBoneLimit, 30 in Game::PostInit)
static const unsigned k_maxSkinNodes = 30;

static bool IsIdentChar(char c) { return isalnum((unsigned char)c) || c == '_'; }

// FindAttributeType @0x6c35c0: the type in "attribute [precision] <type> <name>;"
static std::string FindAttributeType(const std::string &src, const std::string &name) {
    size_t pos = 0;
    while ((pos = src.find("attribute", pos)) != std::string::npos) {
        size_t end = src.find(';', pos);
        if (end == std::string::npos) break;
        std::string decl = src.substr(pos, end - pos);
        std::vector<std::string> tok;
        for (size_t i = 0; i < decl.size();) {
            while (i < decl.size() && !IsIdentChar(decl[i])) i++;
            size_t j = i;
            while (j < decl.size() && IsIdentChar(decl[j])) j++;
            if (j > i) tok.push_back(decl.substr(i, j - i));
            i = j;
        }
        if (tok.size() >= 3 && tok.back() == name) return tok[tok.size() - 2];
        pos = end;
    }
    return std::string();
}

// PreprocessShaderFlavorType @0x6c3904: the node palette uniform, mul4x3/mul3x3 and the node
// attributes go before main(); main() starts by blending the node transforms and skinning the
// position (and normal / tangent) attributes into _jet_<name>_ locals, which replace the
// attributes in main's body.
std::string GLES20ShaderProgram::PreprocessShaderFlavorType(const std::string &vs, unsigned weights) const {
    std::string posName, normalName, tangentName;
    if (m_shaderData)
        for (unsigned i = 0; i < m_shaderData->GetAttributeCount(); i++) {
            unsigned sem = (unsigned char)(Geometry::SemanticDef::type)m_shaderData->GetAttribute(i).GetType();
            const std::string n = m_shaderData->GetAttributeName(i).c_str();
            if (sem == Geometry::SemanticDef::Position) posName = n;
            else if (sem == Geometry::SemanticDef::Normal) normalName = n;
            else if (sem == Geometry::SemanticDef::Tangent) tangentName = n;
        }
    if (posName.empty()) {
        fprintf(stderr, "(%s) Cannot find position attribute for skinning.\n", m_debugName.c_str());
        return vs;
    }
    // "void main ( ) {"
    size_t mainPos = std::string::npos, bodyPos = std::string::npos;
    for (size_t p = vs.find("void"); p != std::string::npos; p = vs.find("void", p + 4)) {
        size_t q = p + 4;
        while (q < vs.size() && isspace((unsigned char)vs[q])) q++;
        if (vs.compare(q, 4, "main")) continue;
        q += 4;
        while (q < vs.size() && isspace((unsigned char)vs[q])) q++;
        if (q >= vs.size() || vs[q] != '(') continue;
        q++;
        while (q < vs.size() && isspace((unsigned char)vs[q])) q++;
        if (q >= vs.size() || vs[q] != ')') continue;
        q++;
        while (q < vs.size() && isspace((unsigned char)vs[q])) q++;
        if (q >= vs.size() || vs[q] != '{') continue;
        mainPos = p;
        bodyPos = q + 1;
        break;
    }
    if (mainPos == std::string::npos) {
        fprintf(stderr, "(%s) Cannot find main() function.\n", m_debugName.c_str());
        return vs;
    }
    struct Attr { std::string name, type, local; } attrs[3] = {
        { posName, FindAttributeType(vs, posName), "_jet_inPos_" },
        { normalName, normalName.empty() ? std::string() : FindAttributeType(vs, normalName), "_jet_inNormal_" },
        { tangentName, tangentName.empty() ? std::string() : FindAttributeType(vs, tangentName), "_jet_inTangent_" } };
    for (unsigned i = 0; i < 3; i++)
        if (!attrs[i].name.empty() && attrs[i].type.empty())
            fprintf(stderr, "(%s) Cannot find %s attribute type.\n", m_debugName.c_str(), attrs[i].name.c_str());

    char buf[64];
    snprintf(buf, sizeof(buf), "uniform highp vec4 _u_node_transforms_[%d];\n", k_maxSkinNodes * 3);
    std::string decl = buf;
    decl += "highp vec4 mul4x3(highp vec4 trans[3], highp vec4 v)\n{\n\thighp vec4 ret;\n\tret.x = dot(trans[0], v);\n\tret.y = dot(trans[1], v);\n\tret.z = dot(trans[2], v);\n\tret.w = 1.0;\n\treturn ret;\n}\nhighp vec3 mul3x3(highp vec4 trans[3], highp vec3 v)\n{\n\thighp vec3 ret;\n\tret.x = dot(trans[0].xyz, v);\n\tret.y = dot(trans[1].xyz, v);\n\tret.z = dot(trans[2].xyz, v);\n\treturn ret;\n}\n";
    std::string body;
    static const char *blend[] = {
        "\nint _jet_indices_ = int(3.0*_a_node_indices_.x);\nhighp vec4 _jet_skin_trans_[3];\n_jet_skin_trans_[0] = _u_node_transforms_[_jet_indices_];\n_jet_skin_trans_[1] = _u_node_transforms_[_jet_indices_+1];\n_jet_skin_trans_[2] = _u_node_transforms_[_jet_indices_+2];\n",
        "\nivec2 _jet_indices_ = ivec2(3.0*_a_node_indices_.xy);\nhighp vec4 _jet_skin_trans_[3];\n_jet_skin_trans_[0] = _u_node_transforms_[_jet_indices_.x ]*_a_node_weights_.x;\n_jet_skin_trans_[1] = _u_node_transforms_[_jet_indices_.x+1]*_a_node_weights_.x;\n_jet_skin_trans_[2] = _u_node_transforms_[_jet_indices_.x+2]*_a_node_weights_.x;\n_jet_skin_trans_[0] += _u_node_transforms_[_jet_indices_.y ]*_a_node_weights_.y;\n_jet_skin_trans_[1] += _u_node_transforms_[_jet_indices_.y+1]*_a_node_weights_.y;\n_jet_skin_trans_[2] += _u_node_transforms_[_jet_indices_.y+2]*_a_node_weights_.y;\n",
        "\nivec3 _jet_indices_ = ivec3(3.0*_a_node_indices_.xyz);\nhighp vec4 _jet_skin_trans_[3];\n_jet_skin_trans_[0] = _u_node_transforms_[_jet_indices_.x ]*_a_node_weights_.x;\n_jet_skin_trans_[1] = _u_node_transforms_[_jet_indices_.x+1]*_a_node_weights_.x;\n_jet_skin_trans_[2] = _u_node_transforms_[_jet_indices_.x+2]*_a_node_weights_.x;\n_jet_skin_trans_[0] += _u_node_transforms_[_jet_indices_.y ]*_a_node_weights_.y;\n_jet_skin_trans_[1] += _u_node_transforms_[_jet_indices_.y+1]*_a_node_weights_.y;\n_jet_skin_trans_[2] += _u_node_transforms_[_jet_indices_.y+2]*_a_node_weights_.y;\n_jet_skin_trans_[0] += _u_node_transforms_[_jet_indices_.z ]*_a_node_weights_.z;\n_jet_skin_trans_[1] += _u_node_transforms_[_jet_indices_.z+1]*_a_node_weights_.z;\n_jet_skin_trans_[2] += _u_node_transforms_[_jet_indices_.z+2]*_a_node_weights_.z;\n",
        "\nivec4 _jet_indices_ = ivec4(3.0 * _a_node_indices_);\nhighp vec4 _jet_skin_trans_[3];\n_jet_skin_trans_[0] = _u_node_transforms_[_jet_indices_.x ]*_a_node_weights_.x;\n_jet_skin_trans_[1] = _u_node_transforms_[_jet_indices_.x+1]*_a_node_weights_.x;\n_jet_skin_trans_[2] = _u_node_transforms_[_jet_indices_.x+2]*_a_node_weights_.x;\n_jet_skin_trans_[0] += _u_node_transforms_[_jet_indices_.y ]*_a_node_weights_.y;\n_jet_skin_trans_[1] += _u_node_transforms_[_jet_indices_.y+1]*_a_node_weights_.y;\n_jet_skin_trans_[2] += _u_node_transforms_[_jet_indices_.y+2]*_a_node_weights_.y;\n_jet_skin_trans_[0] += _u_node_transforms_[_jet_indices_.z ]*_a_node_weights_.z;\n_jet_skin_trans_[1] += _u_node_transforms_[_jet_indices_.z+1]*_a_node_weights_.z;\n_jet_skin_trans_[2] += _u_node_transforms_[_jet_indices_.z+2]*_a_node_weights_.z;\n_jet_skin_trans_[0] += _u_node_transforms_[_jet_indices_.w ]*_a_node_weights_.w;\n_jet_skin_trans_[1] += _u_node_transforms_[_jet_indices_.w+1]*_a_node_weights_.w;\n_jet_skin_trans_[2] += _u_node_transforms_[_jet_indices_.w+2]*_a_node_weights_.w;\n" };
    if (weights < 1) weights = 1;
    if (weights > 4) weights = 4;
    decl += "attribute highp vec4 _a_node_indices_;\n";
    if (weights > 1) decl += "attribute highp vec4 _a_node_weights_;\n";
    body += blend[weights - 1];
    // the position; normals and tangents only rotate
    const Attr &pa = attrs[0];
    body += "\nhighp " + pa.type + " " + pa.local + " = " + pa.type + "(mul4x3(_jet_skin_trans_, vec4(" + pa.name + ".xyz, 1.0)));";
    if (!attrs[1].name.empty() && !attrs[1].type.empty()) {
        if (attrs[1].type == "vec3") body += "\nhighp vec3 " + attrs[1].local + " = mul3x3(_jet_skin_trans_, " + attrs[1].name + ".xyz);";
        else body += "\nhighp vec4 " + attrs[1].local + " = vec4(mul3x3(_jet_skin_trans_, " + attrs[1].name + ".xyz), 0.0);";
    }
    if (!attrs[2].name.empty() && !attrs[2].type.empty()) {
        if (attrs[2].type == "vec3") body += "\nhighp " + attrs[2].type + " " + attrs[2].local + " = " + attrs[2].type + "(mul3x3(_jet_skin_trans_, " + attrs[2].name + ".xyz));";
        else body += "\nhighp " + attrs[2].type + " " + attrs[2].local + " = " + attrs[2].type + "(mul3x3(_jet_skin_trans_, " + attrs[2].name + ".xyz).xyz, " + attrs[2].name + ".w);";
    }
    body += "\n";
    // main's body uses the skinned locals
    std::string rest = vs.substr(bodyPos);
    for (unsigned i = 0; i < 3; i++) {
        const Attr &a = attrs[i];
        if (a.name.empty() || a.type.empty()) continue;
        std::string out;
        size_t p = 0;
        for (size_t f; (f = rest.find(a.name, p)) != std::string::npos; p = f + a.name.size()) {
            bool whole = (f == 0 || !IsIdentChar(rest[f - 1])) && (f + a.name.size() >= rest.size() || !IsIdentChar(rest[f + a.name.size()]));
            out += rest.substr(p, f - p);
            out += whole ? a.local : a.name;
        }
        out += rest.substr(p);
        rest = out;
    }
    return vs.substr(0, mainPos) + decl + vs.substr(mainPos, bodyPos - mainPos) + body + rest;
}

// Source as LinkFlavor @0x6c5720 builds it, then GLES20Driver::LoadShader @0x6b695c prepends one
// "#define NAME [VALUE]" line per driver define (Game::PostInit adds HAS_CURVATURE, HAS_NDIS, ...).
unsigned GLES20ShaderProgram::CompileStage(gles::Interface &gl, unsigned stage, const std::string &src, const char *define) {
    unsigned id = gl.iglCreateShader(stage);
    std::string defines;
    if (Driver *d = Driver::GetCurrent()) {
        const std::map<jet::String, jet::String> &m = d->GetShaderDefinesMap();
        for (std::map<jet::String, jet::String>::const_iterator it = m.begin(); it != m.end(); ++it) {
            defines += "#define ";
            defines += it->first.c_str();
            if (!it->second.empty()) { defines += " "; defines += it->second.c_str(); }
            defines += "\n";
        }
    }
    std::string header = "#define BASE_FLAVOR\t1\n#define IS_FLAVOR 0\n\n\n\n";
    header += define;
    // GLSL ES has no default float precision in fragment shaders; the game's shaders rely on the
    // (lenient) mobile drivers accepting that, Mesa does not.
    std::string precision = std::string("#ifndef GL_ES\n#define highp\n#define mediump\n#define lowp\n#endif\n") + (stage == 0x8B30 ? "#ifdef GL_ES\nprecision mediump float;\n#endif\n" : "");
    // On a GLES 3.0 context the shader is GLSL ES 3.00. The sources stay GLSL ES 1.00 (attribute, varying,
    // texture2D, gl_FragColor), so a prelude maps those names onto in/out/texture and an explicit fragment
    // output jet_FragColor. The GLSL 1.20 (desktop) and ES 1.00 pragmas are unchanged.
    const bool es3 = gles::ClientVersion() >= 3;
    std::string prelude;
    if (es3 && stage == 0x8B31 /*GL_VERTEX_SHADER*/) prelude = "#define attribute in\n#define varying out\n#define texture2D texture\n";
    if (es3 && stage == 0x8B30 /*GL_FRAGMENT_SHADER*/) prelude = "#define varying in\n#define texture2D texture\n#define gl_FragColor jet_FragColor\nout highp vec4 jet_FragColor;\n";
    std::string full = std::string(es3 ? "#version 300 es\n" : JET_GLSL_VERSION_PRAGMA) + defines + precision + prelude + header + src;
    const char *srcPtr = full.c_str();
    int len = (int)full.size();
    gl.iglShaderSource(id, 1, &srcPtr, &len);
    gl.iglCompileShader(id);
    int status = 0;
    gl.iglGetShaderiv(id, 0x8B81 /*GL_COMPILE_STATUS*/, &status);
    if (!status) {
        int logLen = 0;
        gl.iglGetShaderiv(id, 0x8B84 /*GL_INFO_LOG_LENGTH*/, &logLen);
        std::string log(logLen, '\0');
        if (logLen) gl.iglGetShaderInfoLog(id, logLen, 0, &log[0]);
        fprintf(stderr, "(%s) Error loading %s shader: %s\n", m_debugName.c_str(), stage == 0x8B30 ? "fragment" : "vertex", log.c_str());
        gl.iglDeleteShader(id);
        return 0;
    }
    return id;
}

bool GLES20ShaderProgram::CompileFlavor(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> type) {
    gles::Interface gl;
    const char *define = FlavorDefine(type);
    const unsigned weights = (unsigned)(ShaderProgramFlavor::TypeDef::type)type;
    const std::string vsrc = weights ? PreprocessShaderFlavorType(m_vshaderSrc, weights) : m_vshaderSrc;
    unsigned vs = CompileStage(gl, 0x8B31 /*GL_VERTEX_SHADER*/, vsrc, define);
    unsigned fs = vs ? CompileStage(gl, 0x8B30 /*GL_FRAGMENT_SHADER*/, m_fshaderSrc, define) : 0;
    if (!vs || !fs) { if (vs) gl.iglDeleteShader(vs); return false; }

    unsigned program = gl.iglCreateProgram();
    gl.iglAttachShader(program, vs);
    gl.iglAttachShader(program, fs);
    if (m_shaderData) {
        for (unsigned i = 0; i < m_shaderData->GetAttributeCount(); i++) {
            safe_enum<Geometry::SemanticDef,unsigned char> sem8 = m_shaderData->GetAttribute(i).GetType();
            (void)sem8;
            // location i <-> the pass's i-th attribute, by its GLSL name (LinkFlavor @0x6c5f18)
            gl.iglBindAttribLocation(program, i, m_shaderData->GetAttributeName(i).c_str());
        }
        // the skinning attributes follow the pass's own
        if (weights) {
            gl.iglBindAttribLocation(program, m_shaderData->GetAttributeCount(), "_a_node_indices_");
            if (weights > 1) gl.iglBindAttribLocation(program, m_shaderData->GetAttributeCount() + 1, "_a_node_weights_");
        }
    }
    gl.iglLinkProgram(program);
    int linkStatus = 0;
    gl.iglGetProgramiv(program, 0x8B82 /*GL_LINK_STATUS*/, &linkStatus);
    gl.iglDeleteShader(vs);
    gl.iglDeleteShader(fs);
    if (!linkStatus) {
        int logLen = 0;
        gl.iglGetProgramiv(program, 0x8B84 /*GL_INFO_LOG_LENGTH*/, &logLen);
        std::string log(logLen, '\0');
        if (logLen) gl.iglGetProgramInfoLog(program, logLen, 0, &log[0]);
        fprintf(stderr, "[jet] program link failed: %s\n", log.c_str());
        gl.iglDeleteProgram(program);
        return false;
    }

    GLES20ShaderProgramFlavor *flavor = new GLES20ShaderProgramFlavor(type);
    flavor->SetProgramId(vs, fs, program);
    if (m_shaderData) {
        for (unsigned i = 0; i < m_shaderData->GetAttributeCount(); i++) flavor->AddAttribute(m_shaderData->GetAttribute(i));
        if (weights) {
            flavor->AddAttribute(ShaderAttribute(Geometry::SemanticDef::NodeIndices));
            if (weights > 1) flavor->AddAttribute(ShaderAttribute(Geometry::SemanticDef::NodeWeights));
            flavor->SetMaxSkinNodeCount(k_maxSkinNodes);
        }
        for (unsigned i = 0; i < m_shaderData->GetUniformCount(); i++) flavor->AddUniform(m_shaderData->GetUniform(i));
    }
    m_flavors[(int)(ShaderProgramFlavor::TypeDef::type)type] = flavor;
    m_isReady = true;
    return true;
}

void GLES20ShaderProgram::Link() { CompileFlavor(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type>(ShaderProgramFlavor::TypeDef::Normal)); }
void GLES20ShaderProgram::Relink() {
    gles::Interface gl;
    for (std::map<int, GLES20ShaderProgramFlavor*>::iterator it = m_flavors.begin(); it != m_flavors.end(); ++it) {
        if (it->second->GetGLId()) gl.iglDeleteProgram(it->second->GetGLId());
        delete it->second;
    }
    m_flavors.clear();
    Link();
}

ShaderProgramFlavor *GLES20ShaderProgram::GetFlavor(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> type,
                                                     const Material&, unsigned, const std::vector<ShaderUniform>&) {
    int key = (int)(ShaderProgramFlavor::TypeDef::type)type;
    std::map<int, GLES20ShaderProgramFlavor*>::iterator it = m_flavors.find(key);
    if (it != m_flavors.end()) return it->second;
    if (!CompileFlavor(type)) return 0;
    return m_flavors[key];
}

bool GLES20ShaderProgram::IsFlavorCompiled(safe_enum<ShaderProgramFlavor::TypeDef,ShaderProgramFlavor::TypeDef::type> type) {
    return m_flavors.find((int)(ShaderProgramFlavor::TypeDef::type)type) != m_flavors.end();
}

void GLES20ShaderProgram::UploadUniform(gles::Interface &gl, int location, const ShaderUniform &u) {
    using T = ShaderUniform::TypeDef;
    unsigned count = u.GetCount();
    switch ((T::type)u.GetType()) {
    case T::Int: gl.iglUniform1i(location, u.GetInt()); break;
    case T::Float: gl.iglUniform1f(location, u.GetFloat()); break;
    case T::Vec2: gl.iglUniform2fv(location, count, reinterpret_cast<const float*>(u.GetData())); break;
    case T::Vec3: gl.iglUniform3fv(location, count, reinterpret_cast<const float*>(u.GetData())); break;
    case T::Vec4: gl.iglUniform4fv(location, count, reinterpret_cast<const float*>(u.GetData())); break;
    case T::Mat3: gl.iglUniformMatrix3fv(location, count, 0, reinterpret_cast<const float*>(u.GetData())); break;
    case T::Mat4: gl.iglUniformMatrix4fv(location, count, 0, reinterpret_cast<const float*>(u.GetData())); break;
    default: break;  // Mat43/Mat32 (packed skin matrices) are uploaded as plain vec4/vec3 arrays by the caller.
    }
}

// PreprocessAlphaSamplers @0x6c2bd4: jetAlphaTexture2D(s, uv) samples s, times the red channel
// of a separate "s_a_l_p_h_a" sampler when the driver keeps alpha in separate textures.
static std::string PreprocessAlphaSamplers(const std::string &src, bool separateAlpha, std::vector<std::string> &alphaSamplers) {
    static const char tag[] = "jetAlphaTexture2D";
    std::string out;
    size_t pos = 0;
    for (;;) {
        size_t t = src.find(tag, pos);
        if (t == std::string::npos) { out += src.substr(pos); break; }
        size_t open = src.find('(', t);
        size_t comma = src.find(',', open);
        if (open == std::string::npos || comma == std::string::npos) { out += src.substr(pos); break; }
        // matching ')' of the call
        size_t close = open; int depth = 0;
        for (; close < src.size(); close++) {
            if (src[close] == '(') depth++;
            else if (src[close] == ')' && --depth == 0) break;
        }
        std::string sampler = src.substr(open + 1, comma - open - 1);
        while (!sampler.empty() && isspace((unsigned char)sampler[0])) sampler.erase(0, 1);
        while (!sampler.empty() && isspace((unsigned char)sampler[sampler.size() - 1])) sampler.erase(sampler.size() - 1);
        std::string uv = src.substr(comma + 1, close - comma - 1);
        out += src.substr(pos, t - pos);
        if (separateAlpha) {
            out += "(texture2D(" + sampler + "," + uv + ") * vec4(vec3(1.0), texture2D(" + sampler + "_a_l_p_h_a, " + uv + ").r))";
            if (std::find(alphaSamplers.begin(), alphaSamplers.end(), sampler) == alphaSamplers.end()) alphaSamplers.push_back(sampler);
        } else {
            out += "texture2D(" + sampler + "," + uv + ")";
        }
        pos = close + 1;
    }
    if (separateAlpha && !alphaSamplers.empty()) {
        std::string decl;
        for (size_t i = 0; i < alphaSamplers.size(); i++) decl += "uniform sampler2D " + alphaSamplers[i] + "_a_l_p_h_a;\n";
        out = decl + out;
    }
    return out;
}

// LoadV100 @0x6c29ac: <vshader> and <fshader> text with #includes resolved; the fragment shader
// gets the lod debug uniforms declared in front and the debug tint before main's closing brace.
void GLES20ShaderProgram::LoadV100(const pugi::xml_node &node) {
    std::set<std::string> seen;
    if (pugi::xml_node vs = node.child("vshader")) {
        std::string resolved;
        const std::string vtext = std::string(m_technique ? m_technique->GetVertexPatch().c_str() : "") + vs.child_value();  // the technique's patch goes in front
        jet::core::ResolveIncludes(vtext, resolved, seen);
        m_vshaderSrc = resolved;
    }
    if (pugi::xml_node fs = node.child("fshader")) {
        std::string src = std::string("uniform lowp vec4 _jet_debugColor;\n uniform mediump int _jet_debugLod;\n") + (m_technique ? m_technique->GetFragmentPatch().c_str() : "") + fs.child_value();
        std::string resolved;
        seen.clear();
        jet::core::ResolveIncludes(src, resolved, seen);
        size_t end = resolved.rfind('}');
        if (end != std::string::npos)
            resolved.insert(end, "\n if (_jet_debugLod != 0) {  gl_FragColor = vec4(gl_FragColor.rgb * 0.5 + _jet_debugColor.rgb * 0.5, gl_FragColor.a) ; } \n");
        Driver *d = Driver::GetCurrent();
        m_alphaSamplers.clear();
        m_fshaderSrc = PreprocessAlphaSamplers(resolved, d && d->GetSeparateAlphaTextures(), m_alphaSamplers);
    }
}

}

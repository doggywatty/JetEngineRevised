// source/editor/formats/pig_reader.cpp
#include "editor/formats/pig_reader.h"

#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace jeteditor::pig {
namespace {

// Every versioned record in a .pig starts with this int (ModelBase::LoadV100 and the node/mesh/submesh loaders)
constexpr int32_t kVersion = 100;

struct ParseError {
    std::string message;
};

// Little-endian cursor over the file bytes. Each read checks the remaining length first.
class Cursor {
public:
    explicit Cursor(const std::vector<uint8_t> &bytes) : m_bytes(bytes) {}

    size_t Remaining() const { return m_bytes.size() - m_offset; }

    const uint8_t *Take(size_t n, const char *what) {
        if (n > Remaining())
            throw ParseError{std::string("truncated while reading ") + what + " at offset " + std::to_string(m_offset)};
        const uint8_t *p = m_bytes.data() + m_offset;
        m_offset += n;
        return p;
    }

    uint8_t U8(const char *what) { return *Take(1, what); }

    uint16_t U16(const char *what) {
        const uint8_t *p = Take(2, what);
        return static_cast<uint16_t>(p[0] | (p[1] << 8));
    }

    uint32_t U32(const char *what) {
        const uint8_t *p = Take(4, what);
        return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) |
               (static_cast<uint32_t>(p[3]) << 24);
    }

    int16_t S16(const char *what) { return static_cast<int16_t>(U16(what)); }

    int32_t I32(const char *what) { return static_cast<int32_t>(U32(what)); }

    float F32(const char *what) {
        uint32_t bits = U32(what);
        float f;
        std::memcpy(&f, &bits, 4);
        return f;
    }

    std::string Str(const char *what) {
        uint16_t n = U16(what);
        const uint8_t *p = Take(n, what);
        return std::string(reinterpret_cast<const char *>(p), n);
    }

    void Version(const char *what) {
        int32_t v = I32(what);
        if (v != kVersion)
            throw ParseError{std::string("bad version ") + std::to_string(v) + " for " + what + " (want 100)"};
    }

    Vec3 Vec(const char *what) {
        Vec3 v;
        v.x = F32(what);
        v.y = F32(what);
        v.z = F32(what);
        return v;
    }

    std::vector<Property> Properties(const char *what) {
        uint16_t n = U16(what);
        std::vector<Property> out(n);
        for (Property &p : out) {
            p.key = Str(what);
            p.value = Str(what);
        }
        return out;
    }

private:
    const std::vector<uint8_t> &m_bytes;
    size_t m_offset = 0;
};

// NodeV100: int 100, name, flags, parent, position, rotation (x, y, z, w), scale, radius, properties
Node ReadNode(Cursor &c) {
    c.Version("node");
    Node n;
    n.name = c.Str("node name");
    n.flags = c.U8("node flags");
    n.parent = c.S16("node parent");
    n.position = c.Vec("node position");
    n.rotation.x = c.F32("node rotation");
    n.rotation.y = c.F32("node rotation");
    n.rotation.z = c.F32("node rotation");
    n.rotation.w = c.F32("node rotation");
    n.scale = c.Vec("node scale");
    n.radius = c.F32("node radius");
    n.properties = c.Properties("node property");
    return n;
}

// CameraV100: node, target, horizontal fov (degrees), aspect, near, far, ortho byte
Camera ReadCamera(Cursor &c) {
    Camera cam;
    cam.node = ReadNode(c);
    cam.target = c.Str("camera target");
    cam.hfov = c.F32("camera fov");
    cam.aspect = c.F32("camera aspect");
    cam.nearDist = c.F32("camera near");
    cam.farDist = c.F32("camera far");
    cam.ortho = c.U8("camera ortho");
    return cam;
}

// SubMeshV100: int 100, format, attribute mask, center, [quantization if format & 3], vertex count, index count,
// material, texture count and names, unused byte, blob size and blob, [skin block if the mesh is skinned]
SubMesh ReadSubMesh(Cursor &c, bool skinned) {
    c.Version("submesh");
    SubMesh sm;
    sm.format = c.U32("submesh format");
    sm.u0c = c.U32("submesh mask");
    sm.center = c.Vec("submesh center");
    if (sm.format & 3) {
        sm.quant.offset = c.Vec("submesh quant offset");
        sm.quant.scale = c.Vec("submesh quant scale");
    }
    sm.u44 = c.U16("submesh u44");
    sm.u48 = c.U32("submesh u48");
    sm.material = c.Str("submesh material");
    uint16_t textureCount = c.U16("submesh texture count");
    sm.textures.resize(textureCount);
    for (std::string &t : sm.textures) t = c.Str("submesh texture");
    sm.unused = c.U8("submesh unused");
    uint32_t blobSize = c.U32("submesh blob size");
    const uint8_t *blob = c.Take(blobSize, "submesh blob");
    sm.vertexBlob.assign(blob, blob + blobSize);
    if (skinned) {
        Skin skin;
        skin.weightsPerVertex = c.U16("skin weights");
        skin.boneCount = c.U16("skin bones");
        uint32_t size = c.U32("skin size");
        const uint8_t *data = c.Take(size, "skin data");
        skin.data.assign(data, data + size);
        sm.skin = std::move(skin);
    }
    return sm;
}

// MeshV100: int 100, skinned byte, unused byte, aabb (6 floats), submesh count, submeshes
Mesh ReadMesh(Cursor &c) {
    c.Version("mesh");
    Mesh m;
    m.skinned = c.U8("mesh skinned");
    m.unused = c.U8("mesh unused");
    for (float &f : m.aabb) f = c.F32("mesh aabb");
    uint16_t count = c.U16("mesh submesh count");
    m.submeshes.reserve(count);
    for (uint16_t i = 0; i < count; i++) m.submeshes.push_back(ReadSubMesh(c, m.skinned != 0));
    return m;
}

// MultiResMesh: int 100, node index, LOD count, then per LOD: slot byte and mesh
MultiResMesh ReadMultiRes(Cursor &c) {
    c.Version("multires");
    MultiResMesh mr;
    mr.nodeIndex = c.U32("multires node");
    uint16_t lodCount = c.U16("multires lod count");
    mr.lods.reserve(lodCount);
    for (uint16_t i = 0; i < lodCount; i++) {
        Lod lod;
        lod.slot = c.U8("lod slot");
        lod.mesh = ReadMesh(c);
        mr.lods.push_back(std::move(lod));
    }
    return mr;
}

// The whole file: int 100, nodes, a0 byte, multires meshes, cameras, then the optional source-path table
Model ReadModel(Cursor &c) {
    c.Version("model");
    Model m;
    uint16_t nodeCount = c.U16("node count");
    m.nodes.reserve(nodeCount);
    for (uint16_t i = 0; i < nodeCount; i++) m.nodes.push_back(ReadNode(c));
    m.a0 = c.U8("a0");
    uint16_t mrCount = c.U16("multires count");
    m.multires.reserve(mrCount);
    for (uint16_t i = 0; i < mrCount; i++) m.multires.push_back(ReadMultiRes(c));
    uint16_t camCount = c.U16("camera count");
    m.cameras.reserve(camCount);
    for (uint16_t i = 0; i < camCount; i++) m.cameras.push_back(ReadCamera(c));
    if (c.Remaining() > 0) {
        m.hasSourcePathTable = true;
        m.sourcePathTable = c.Properties("source path entry");
    }
    if (c.Remaining() != 0) throw ParseError{std::to_string(c.Remaining()) + " trailing bytes after the model"};
    return m;
}

}  // namespace

bool ReadPig(const std::vector<uint8_t> &bytes, Model &out, std::string &error) {
    try {
        Cursor c(bytes);
        out = ReadModel(c);
        return true;
    } catch (const ParseError &e) {
        out = Model();
        error = e.message;
        return false;
    }
}

bool ReadPigFile(const std::string &path, Model &out, std::string &error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        out = Model();
        error = "cannot open " + path;
        return false;
    }
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return ReadPig(bytes, out, error);
}

}  // namespace jeteditor::pig

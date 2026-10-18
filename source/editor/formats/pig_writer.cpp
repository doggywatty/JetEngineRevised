// source/editor/formats/pig_writer.cpp
#include "editor/formats/pig_writer.h"

#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <utility>

namespace jeteditor::pig {
namespace {

constexpr int32_t kVersion = 100;

struct WriteError {
    std::string message;
};

// Little-endian sink for the file bytes
class Sink {
public:
    explicit Sink(std::vector<uint8_t> &out) : m_out(out) {}

    void U8(uint8_t v) { m_out.push_back(v); }

    void U16(uint16_t v) {
        m_out.push_back(static_cast<uint8_t>(v));
        m_out.push_back(static_cast<uint8_t>(v >> 8));
    }

    void U32(uint32_t v) {
        for (int i = 0; i < 4; i++) m_out.push_back(static_cast<uint8_t>(v >> (8 * i)));
    }

    void I32(int32_t v) { U32(static_cast<uint32_t>(v)); }

    void S16(int16_t v) { U16(static_cast<uint16_t>(v)); }

    void F32(float f) {
        uint32_t bits;
        std::memcpy(&bits, &f, 4);
        U32(bits);
    }

    void Bytes(const std::vector<uint8_t> &b) { m_out.insert(m_out.end(), b.begin(), b.end()); }

    // A count that is written as u16
    void Count(size_t n, const char *what) {
        if (n > std::numeric_limits<uint16_t>::max())
            throw WriteError{std::string(what) + " has " + std::to_string(n) + " entries (max 65535)"};
        U16(static_cast<uint16_t>(n));
    }

    void Str(const std::string &s, const char *what) {
        if (s.size() > std::numeric_limits<uint16_t>::max())
            throw WriteError{std::string(what) + " is " + std::to_string(s.size()) + " bytes (max 65535)"};
        U16(static_cast<uint16_t>(s.size()));
        m_out.insert(m_out.end(), s.begin(), s.end());
    }

    void Vec(const Vec3 &v) {
        F32(v.x);
        F32(v.y);
        F32(v.z);
    }

    void Properties(const std::vector<Property> &props, const char *what) {
        Count(props.size(), what);
        for (const Property &p : props) {
            Str(p.key, what);
            Str(p.value, what);
        }
    }

    // A size written as u32, followed by the bytes it counts
    void Blob(const std::vector<uint8_t> &b, const char *what) {
        if (b.size() > std::numeric_limits<uint32_t>::max())
            throw WriteError{std::string(what) + " does not fit a u32 size"};
        U32(static_cast<uint32_t>(b.size()));
        Bytes(b);
    }

private:
    std::vector<uint8_t> &m_out;
};

void WriteNode(Sink &s, const Node &n) {
    s.I32(kVersion);
    s.Str(n.name, "node name");
    s.U8(n.flags);
    s.S16(n.parent);
    s.Vec(n.position);
    s.F32(n.rotation.x);
    s.F32(n.rotation.y);
    s.F32(n.rotation.z);
    s.F32(n.rotation.w);
    s.Vec(n.scale);
    s.F32(n.radius);
    s.Properties(n.properties, "node properties");
}

void WriteCamera(Sink &s, const Camera &c) {
    WriteNode(s, c.node);
    s.Str(c.target, "camera target");
    s.F32(c.hfov);
    s.F32(c.aspect);
    s.F32(c.nearDist);
    s.F32(c.farDist);
    s.U8(c.ortho);
}

void WriteSubMesh(Sink &s, const SubMesh &sm, bool meshSkinned) {
    if (meshSkinned != sm.skin.has_value())
        throw WriteError{meshSkinned ? "skinned mesh has a submesh without a skin block"
                                     : "unskinned mesh has a submesh with a skin block"};
    s.I32(kVersion);
    s.U32(sm.format);
    s.U32(sm.u0c);
    s.Vec(sm.center);
    if (sm.format & 3) {
        s.Vec(sm.quant.offset);
        s.Vec(sm.quant.scale);
    }
    s.U16(sm.u44);
    s.U32(sm.u48);
    s.Str(sm.material, "submesh material");
    s.Count(sm.textures.size(), "submesh textures");
    for (const std::string &t : sm.textures) s.Str(t, "submesh texture");
    s.U8(sm.unused);
    s.Blob(sm.vertexBlob, "submesh vertex blob");
    if (sm.skin) {
        s.U16(sm.skin->weightsPerVertex);
        s.U16(sm.skin->boneCount);
        s.Blob(sm.skin->data, "skin data");
    }
}

void WriteMesh(Sink &s, const Mesh &m) {
    s.I32(kVersion);
    s.U8(m.skinned);
    s.U8(m.unused);
    for (float f : m.aabb) s.F32(f);
    s.Count(m.submeshes.size(), "mesh submeshes");
    for (const SubMesh &sm : m.submeshes) WriteSubMesh(s, sm, m.skinned != 0);
}

void WriteMultiRes(Sink &s, const MultiResMesh &mr) {
    s.I32(kVersion);
    s.U32(mr.nodeIndex);
    s.Count(mr.lods.size(), "multires lods");
    for (const Lod &lod : mr.lods) {
        s.U8(lod.slot);
        WriteMesh(s, lod.mesh);
    }
}

void WriteModel(Sink &s, const Model &m) {
    s.I32(kVersion);
    s.Count(m.nodes.size(), "nodes");
    for (const Node &n : m.nodes) WriteNode(s, n);
    s.U8(m.a0);
    s.Count(m.multires.size(), "multires meshes");
    for (const MultiResMesh &mr : m.multires) WriteMultiRes(s, mr);
    s.Count(m.cameras.size(), "cameras");
    for (const Camera &c : m.cameras) WriteCamera(s, c);
    if (m.hasSourcePathTable) {
        s.Properties(m.sourcePathTable, "source path table");
    } else if (!m.sourcePathTable.empty()) {
        throw WriteError{"source path entries are set but hasSourcePathTable is false"};
    }
}

}  // namespace

bool WritePig(const Model &model, std::vector<uint8_t> &out, std::string &error) {
    out.clear();
    try {
        Sink s(out);
        WriteModel(s, model);
        return true;
    } catch (const WriteError &e) {
        out.clear();
        error = e.message;
        return false;
    }
}

bool WritePigFile(const std::string &path, const Model &model, std::string &error) {
    std::vector<uint8_t> bytes;
    if (!WritePig(model, bytes, error)) return false;
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        error = "cannot create " + path;
        return false;
    }
    out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!out) {
        error = "write failed for " + path;
        return false;
    }
    return true;
}

}  // namespace jeteditor::pig

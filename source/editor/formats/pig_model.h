// source/editor/formats/pig_model.h
//
// In-memory form of a .pig model (ModelBase::LoadV100 @0x701a70, layout in re/NOTES.md). Every field the
// engine reads is kept. Fields whose meaning is not established are kept as the raw value read from the file
// and are never interpreted; the writer emits exactly what is stored, so read -> write is byte-exact.
//
// Depends on the standard library only.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace jeteditor::pig {

struct Vec3 {
    float x = 0, y = 0, z = 0;
};

struct Quat {
    float x = 0, y = 0, z = 0, w = 1;  // stored as x, y, z, w
};

// A (key, value) pair of strings: node properties, or the trailing table of the file
struct Property {
    std::string key;
    std::string value;
};

// A scene node. flags is the raw byte. The engine sets 0x80 on nodes named "camera" at load time; that is
// derived from the name and is not stored, so the byte round-trips as read.
struct Node {
    std::string name;
    uint8_t flags = 0;
    int16_t parent = -1;
    Vec3 position;
    Quat rotation;
    Vec3 scale;
    float radius = 0;
    std::vector<Property> properties;
};

struct Camera {
    Node node;
    std::string target;
    float hfov = 0;     // horizontal fov in degrees, as stored
    float aspect = 0;
    float nearDist = 0;
    float farDist = 0;
    uint8_t ortho = 0;  // raw byte
};

// Skin block of a submesh of a skinned mesh (after the vertex blob)
struct Skin {
    uint16_t weightsPerVertex = 0;  // u16 read as m_weightCount in SubMesh::LoadV100 (raw)
    uint16_t boneCount = 0;         // u16 read as m_skinNodeCount in SubMesh::LoadV100 (raw)
    std::vector<uint8_t> data;      // boneCount node records (0x1c each), then boneCount mat4 inverse binds (raw)
};

// Position quantization: present in the file iff (format & 3) != 0. Zero otherwise.
struct Quantization {
    Vec3 offset;
    Vec3 scale;
};

struct SubMesh {
    uint32_t format = 0;     // raw u32, bit 31 included (the engine clears it on load)
    uint32_t u0c = 0;        // raw attribute mask
    Vec3 center;
    Quantization quant;
    uint16_t u44 = 0;        // raw u16
    uint32_t u48 = 0;        // raw u32
    std::string material;
    std::vector<std::string> textures;  // 3ds Max map slots; some are empty
    uint8_t unused = 0;      // raw byte before the blob
    std::vector<uint8_t> vertexBlob;    // raw vertex/index blob (SubMesh::LoadAttributes @0x707130)
    std::optional<Skin> skin;           // present iff the owning mesh is skinned
};

struct Mesh {
    uint8_t skinned = 0;     // raw byte; nonzero means the submeshes carry skin blocks
    uint8_t unused = 0;      // raw second byte after skinned
    float aabb[6] = {0, 0, 0, 0, 0, 0};  // min xyz, max xyz, as stored
    std::vector<SubMesh> submeshes;
};

struct Lod {
    uint8_t slot = 0;  // raw byte (the engine maps 1..5 to a slot and anything else to 0)
    Mesh mesh;
};

struct MultiResMesh {
    uint32_t nodeIndex = 0;  // raw u32 (the engine stores it as u16)
    std::vector<Lod> lods;
};

struct Model {
    std::vector<Node> nodes;
    uint8_t a0 = 0;                    // raw byte after the nodes
    std::vector<MultiResMesh> multires;
    std::vector<Camera> cameras;
    bool hasSourcePathTable = false;   // the trailing table is present (it can be present with 0 entries)
    std::vector<Property> sourcePathTable;  // raw (key, value) pairs of the trailing table
};

}  // namespace jeteditor::pig

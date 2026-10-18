// source/editor/formats/pvr3.cpp
#include "editor/formats/pvr3.h"

namespace jeteditor::pvr3 {
namespace {

constexpr size_t kHeaderSize = 52;
constexpr uint8_t kMagic[4] = {'P', 'V', 'R', 0x03};

uint32_t Le32(const uint8_t *p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

uint64_t Le64(const uint8_t *p) { return static_cast<uint64_t>(Le32(p)) | (static_cast<uint64_t>(Le32(p + 4)) << 32); }

}  // namespace

bool IsPvr3(const std::vector<uint8_t> &bytes) {
    if (bytes.size() < 4) return false;
    for (int i = 0; i < 4; i++)
        if (bytes[i] != kMagic[i]) return false;
    return true;
}

bool ReadPvr3(const std::vector<uint8_t> &bytes, Info &out, std::string &error) {
    out = Info();
    if (!IsPvr3(bytes)) {
        error = "not a PVR3 file (magic is not PVR\\x03)";
        return false;
    }
    if (bytes.size() < kHeaderSize) {
        error = "PVR3 is shorter than its 52-byte header";
        return false;
    }
    const uint8_t *h = bytes.data();
    out.flags = Le32(h + 4);
    out.pixelFormat = Le64(h + 8);
    out.colourSpace = Le32(h + 16);
    out.channelType = Le32(h + 20);
    out.height = Le32(h + 24);
    out.width = Le32(h + 28);
    out.depth = Le32(h + 32);
    out.surfaces = Le32(h + 36);
    out.faces = Le32(h + 40);
    out.mipLevels = Le32(h + 44);
    out.metaDataSize = Le32(h + 48);

    if (out.pixelFormat > 0xffffffffu) {
        error = "PVR3 custom pixel format is not supported";
        return false;
    }
    if (out.pixelFormat != kPixelFormatEtc1) {
        error = "PVR3 pixel format " + std::to_string(out.pixelFormat) + " is not supported (ETC1 = 6 only)";
        return false;
    }
    if (out.width == 0 || out.height == 0 || out.depth == 0 || out.surfaces == 0 || out.faces == 0 ||
        out.mipLevels == 0) {
        error = "PVR3 header has a zero width, height, depth, surface, face or mip count";
        return false;
    }

    out.dataOffset = kHeaderSize + static_cast<size_t>(out.metaDataSize);
    if (bytes.size() < out.dataOffset) {
        error = "PVR3 metadata runs past the end of the file";
        return false;
    }

    uint64_t blocksTotal = 0;
    for (uint32_t level = 0; level < out.mipLevels; level++) {
        uint64_t w = out.width >> level;
        uint64_t hgt = out.height >> level;
        if (w == 0) w = 1;
        if (hgt == 0) hgt = 1;
        blocksTotal += ((w + 3) / 4) * ((hgt + 3) / 4) * 8;
    }
    out.dataSize = blocksTotal * out.depth * out.surfaces * out.faces;

    const uint64_t expected = static_cast<uint64_t>(out.dataOffset) + out.dataSize;
    if (bytes.size() < expected) {
        error = "PVR3 is truncated: " + std::to_string(bytes.size()) + " bytes, expected " + std::to_string(expected);
        return false;
    }
    if (bytes.size() > expected) {
        error = "PVR3 has " + std::to_string(bytes.size() - expected) + " bytes after the mip data";
        return false;
    }
    return true;
}

}  // namespace jeteditor::pvr3

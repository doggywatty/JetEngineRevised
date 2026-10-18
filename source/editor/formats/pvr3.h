// source/editor/formats/pvr3.h
//
// PVR v3 texture container, read only. A PVR3 file starts with the bytes 'P' 'V' 'R' 0x03 (the files in
// data/textures_android carry a .tga name but are PVR3). The reader checks the 52-byte header and that the file
// length equals header + metadata + mip data exactly. Only pixel format 6 (ETC1, 4x4 blocks of 8 bytes) has a
// block size here; other formats are refused with an error. Writing PVR3 is not supported.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace jeteditor::pvr3 {

constexpr uint64_t kPixelFormatEtc1 = 6;

struct Info {
    uint64_t pixelFormat = 0;
    uint32_t flags = 0;
    uint32_t colourSpace = 0;
    uint32_t channelType = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    uint32_t surfaces = 0;
    uint32_t faces = 0;
    uint32_t mipLevels = 0;
    uint32_t metaDataSize = 0;
    size_t dataOffset = 0;  // 52 + metaDataSize
    uint64_t dataSize = 0;  // all surfaces, faces, depth slices and mip levels
};

bool IsPvr3(const std::vector<uint8_t> &bytes);
bool ReadPvr3(const std::vector<uint8_t> &bytes, Info &out, std::string &error);

}  // namespace jeteditor::pvr3

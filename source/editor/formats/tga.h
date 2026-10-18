// source/editor/formats/tga.h
//
// Uncompressed truecolor TGA: image type 2, 24 or 32 bits per pixel, no colour map. Reading keeps every header
// field and the pixel rows in file order, so a file read by ReadTga and written by WriteTga is byte-identical.
// Refused with an error: RLE images (types 9 to 11), colour-mapped images, other pixel depths, right-to-left or
// interleaved images, and any bytes after the pixel data (the TGA 2.0 footer is one of them).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace jeteditor::tga {

struct Image {
    std::vector<uint8_t> id;             // image ID field, as stored (0 to 255 bytes)
    uint16_t xOrigin = 0;
    uint16_t yOrigin = 0;
    uint8_t colorMapSpec[5] = {0, 0, 0, 0, 0};  // stored as read; a colour map type of 0 makes them unused
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t depth = 0;                   // 24 or 32
    uint8_t alphaBits = 0;               // descriptor bits 0 to 3: 0 for 24-bit; 0 or 8 for 32-bit
    bool topLeft = false;                // descriptor bit 5: rows are stored top to bottom
    std::vector<uint8_t> pixels;         // width * height * depth / 8 bytes, B G R [A] per pixel, rows in file order
};

bool ReadTga(const std::vector<uint8_t> &bytes, Image &out, std::string &error);
bool WriteTga(const Image &image, std::vector<uint8_t> &out, std::string &error);

}  // namespace jeteditor::tga

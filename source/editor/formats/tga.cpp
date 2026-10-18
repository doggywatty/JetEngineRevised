// source/editor/formats/tga.cpp
#include "editor/formats/tga.h"

#include <cstddef>

namespace jeteditor::tga {
namespace {

constexpr size_t kHeaderSize = 18;
constexpr uint8_t kImageTypeUncompressedTrueColor = 2;

uint16_t Le16(const uint8_t *p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }

void Put16(std::vector<uint8_t> &out, uint16_t v) {
    out.push_back(static_cast<uint8_t>(v));
    out.push_back(static_cast<uint8_t>(v >> 8));
}

size_t PixelBytes(uint16_t width, uint16_t height, uint8_t depth) {
    return static_cast<size_t>(width) * static_cast<size_t>(height) * (depth / 8);
}

}  // namespace

bool ReadTga(const std::vector<uint8_t> &bytes, Image &out, std::string &error) {
    out = Image();
    if (bytes.size() < kHeaderSize) {
        error = "TGA is shorter than its 18-byte header";
        return false;
    }
    const uint8_t *h = bytes.data();
    const size_t idLength = h[0];
    const uint8_t colorMapType = h[1];
    const uint8_t imageType = h[2];
    if (colorMapType != 0) {
        error = "colour-mapped TGA is not supported (colour map type " + std::to_string(colorMapType) + ")";
        return false;
    }
    if (imageType != kImageTypeUncompressedTrueColor) {
        error = "TGA image type " + std::to_string(imageType) +
                " is not supported (only uncompressed truecolor, type 2)";
        return false;
    }
    for (int i = 0; i < 5; i++) out.colorMapSpec[i] = h[3 + i];
    out.xOrigin = Le16(h + 8);
    out.yOrigin = Le16(h + 10);
    out.width = Le16(h + 12);
    out.height = Le16(h + 14);
    out.depth = h[16];
    const uint8_t descriptor = h[17];
    out.alphaBits = descriptor & 0x0f;
    out.topLeft = (descriptor & 0x20) != 0;
    if (descriptor & 0x10) {
        error = "right-to-left TGA is not supported";
        return false;
    }
    if (descriptor & 0xc0) {
        error = "interleaved TGA is not supported";
        return false;
    }
    if (out.depth != 24 && out.depth != 32) {
        error = "TGA pixel depth " + std::to_string(out.depth) + " is not supported (24 or 32)";
        return false;
    }
    if (out.width == 0 || out.height == 0) {
        error = "TGA has a zero dimension";
        return false;
    }
    if (out.depth == 24 && out.alphaBits != 0) {
        error = "24-bit TGA declares " + std::to_string(out.alphaBits) + " alpha bits";
        return false;
    }
    if (out.depth == 32 && out.alphaBits != 0 && out.alphaBits != 8) {
        error = "32-bit TGA declares " + std::to_string(out.alphaBits) + " alpha bits (0 or 8 accepted)";
        return false;
    }

    const size_t pixelBytes = PixelBytes(out.width, out.height, out.depth);
    const size_t expected = kHeaderSize + idLength + pixelBytes;
    if (bytes.size() < expected) {
        error = "TGA is truncated: " + std::to_string(bytes.size()) + " bytes, expected " + std::to_string(expected);
        return false;
    }
    if (bytes.size() > expected) {
        error = "TGA has " + std::to_string(bytes.size() - expected) + " bytes after the pixel data";
        return false;
    }
    out.id.assign(bytes.begin() + kHeaderSize, bytes.begin() + kHeaderSize + idLength);
    out.pixels.assign(bytes.begin() + kHeaderSize + idLength, bytes.end());
    return true;
}

bool WriteTga(const Image &image, std::vector<uint8_t> &out, std::string &error) {
    out.clear();
    if (image.id.size() > 255) {
        error = "TGA image ID is " + std::to_string(image.id.size()) + " bytes (max 255)";
        return false;
    }
    if (image.width == 0 || image.height == 0) {
        error = "TGA has a zero dimension";
        return false;
    }
    if (image.depth != 24 && image.depth != 32) {
        error = "TGA pixel depth " + std::to_string(image.depth) + " cannot be written (24 or 32)";
        return false;
    }
    if (image.depth == 24 && image.alphaBits != 0) {
        error = "24-bit TGA cannot carry alpha bits";
        return false;
    }
    if (image.depth == 32 && image.alphaBits != 0 && image.alphaBits != 8) {
        error = "32-bit TGA alpha bits must be 0 or 8";
        return false;
    }
    if (image.pixels.size() != PixelBytes(image.width, image.height, image.depth)) {
        error = "TGA pixel buffer is " + std::to_string(image.pixels.size()) + " bytes, expected " +
                std::to_string(PixelBytes(image.width, image.height, image.depth));
        return false;
    }

    out.reserve(kHeaderSize + image.id.size() + image.pixels.size());
    out.push_back(static_cast<uint8_t>(image.id.size()));
    out.push_back(0);  // no colour map
    out.push_back(kImageTypeUncompressedTrueColor);
    for (uint8_t b : image.colorMapSpec) out.push_back(b);
    Put16(out, image.xOrigin);
    Put16(out, image.yOrigin);
    Put16(out, image.width);
    Put16(out, image.height);
    out.push_back(image.depth);
    out.push_back(static_cast<uint8_t>((image.alphaBits & 0x0f) | (image.topLeft ? 0x20 : 0)));
    out.insert(out.end(), image.id.begin(), image.id.end());
    out.insert(out.end(), image.pixels.begin(), image.pixels.end());
    return true;
}

}  // namespace jeteditor::tga

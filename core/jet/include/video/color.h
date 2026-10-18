// core/jet/include/video/color.h
#ifndef CORE_JET_INCLUDE_VIDEO_COLOR_H
#define CORE_JET_INCLUDE_VIDEO_COLOR_H

namespace jet::video {
// RGBA, each channel in [0,1]; packs/unpacks as 0xAABBGGRR (matches Material::m_color and the
// vertex color attribute format used throughout the GLES2 driver).
class Color {
public:
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
    explicit Color(unsigned packed) { SetPacked(packed); }

    unsigned GetPacked() const {
        return (unsigned)(Clamp(r) * 255.0f + 0.5f) | ((unsigned)(Clamp(g) * 255.0f + 0.5f) << 8) |
               ((unsigned)(Clamp(b) * 255.0f + 0.5f) << 16) | ((unsigned)(Clamp(a) * 255.0f + 0.5f) << 24);
    }
    void SetPacked(unsigned packed) {
        r = (packed & 0xff) / 255.0f;
        g = ((packed >> 8) & 0xff) / 255.0f;
        b = ((packed >> 16) & 0xff) / 255.0f;
        a = ((packed >> 24) & 0xff) / 255.0f;
    }

    bool operator==(const Color &o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
    bool operator!=(const Color &o) const { return !(*this == o); }
    Color operator*(float s) const { return Color(r * s, g * s, b * s, a * s); }
    Color operator*(const Color &o) const { return Color(r * o.r, g * o.g, b * o.b, a * o.a); }
    Color operator+(const Color &o) const { return Color(r + o.r, g + o.g, b + o.b, a + o.a); }

    float r, g, b, a;
private:
    static float Clamp(float v) { return v < 0 ? 0 : v > 1 ? 1 : v; }
};
}

#endif

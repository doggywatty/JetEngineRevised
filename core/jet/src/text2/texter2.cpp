// core/jet/src/text2/texter2.cpp
#include "text2/texter2.h"
#include "video/painter.h"
#include <cstdlib>
#include <cstring>

namespace jet::text2 {

typedef Font::Glyph Glyph;

Texter::Texter(const Style &s) : m_style(s), m_wrapOnAnyChar(false), m_rightToLeft(false) {}
Texter::Texter() : m_wrapOnAnyChar(false), m_rightToLeft(false) {}
void Texter::SetWrapOnAnyChar(bool v) { m_wrapOnAnyChar = v; }
void Texter::SetRightToLeft(bool v) { m_rightToLeft = v; }
void Texter::SetShaper(boost::shared_ptr<Shaper> s) { m_shaper = s; }
const Texter::Style &Texter::GetStyle() const { return m_style; }
void Texter::SetStyle(const Style &s) { m_style = s; }

// "#RRGGBB" switches the colour; "##" draws a literal '#'. Advances i past the tag; true if a tag was consumed. @0x6b2408
static bool ParseColorTag(unsigned &i, unsigned *color, const std::vector<Glyph> &g) {
    if (g[i].codepoint != '#') return false;
    unsigned n = (unsigned)g.size();
    if (i + 1 == n) return false;
    if (i + 1 < n && g[i + 1].codepoint == '#') { i++; return false; }
    i++;
    char buf[8] = {0};
    unsigned count = 0;
    while (i < n && count < 6) {
        unsigned c = g[i].codepoint;
        i++;
        if (!((c - '0') <= 9 || ((c & ~0x20u) - 'A') <= 5)) break;
        buf[count++] = (char)c;
    }
    if (color) *color = (unsigned)strtol(buf, 0, 16) | 0xff000000u;
    return true;
}

// the characters a line may not start with after a space (@0xbddba0)
static bool IsBreakPunct(unsigned c) {
    static const unsigned k[] = { '?', '!', ';', ':', '%', '$', '-', '+' };
    for (unsigned v : k) if (v == c) return true;
    return false;
}

// @0x6b2844: cur is the glyph being measured, prev/next its neighbours
bool Texter::CanWrapHere(unsigned cur, unsigned prev, unsigned next) const {
    if (cur == ' ') {
        if (!IsBreakPunct(next)) return true;
    } else if (cur == '|')
        return true;
    if (!m_wrapOnAnyChar) return false;
    if (((prev & ~0x20u) - 'A') <= 25 || (prev - '0') <= 9) return false;
    return !IsBreakPunct(prev);
}

// @0x6b2c00
unsigned Texter::ComputeLineCount(const std::vector<Glyph> &g) const {
    unsigned lines = 1;
    for (const Glyph &x : g) if (x.codepoint == '\n') lines++;
    return lines;
}

// inserts line breaks so no line is wider than maxWidth; returns the line count. @0x6b28e8
unsigned Texter::WordWrap(std::vector<Glyph> &g, float maxWidth) const {
    float width = 0;
    unsigned lines = 0;
    unsigned i = 0;
    int words = 0, wordStart = -1, prevIdx = -1;
    float tabWidth = m_style.scale * 4.0f * m_style.font->GetSpaceWidth(m_style.height);
    maxWidth = (float)(int)maxWidth;
    Glyph nl;
    nl.codepoint = 10; nl.glyphIndex = 0; nl.advance = 0; nl.width = 0; nl.height = 0; nl.page = 0xffff;
    while (i < g.size()) {
        if (ParseColorTag(i, 0, g)) continue;
        unsigned cp = g[i].codepoint;
        unsigned prevCp = i ? g[i - 1].codepoint : i;
        unsigned nextCp = i + 1 < g.size() ? g[i + 1].codepoint : 0;
        if (cp == 10) {
            wordStart = -1; i++; lines++; words = 0; width = 0; prevIdx = -1;
            continue;
        }
        if (cp == 9) {
            width += tabWidth;
            if (wordStart >= 0) words++;
            wordStart = -1;
        } else {
            if (CanWrapHere(cp, prevCp, nextCp)) {
                if (wordStart >= 0) words++;
                wordStart = m_wrapOnAnyChar ? (int)i : -1;
            } else if (wordStart < 0)
                wordStart = (int)i;
        }
        if (prevIdx < 0) {
            prevIdx = (int)i;
            if (prevIdx == 0) prevIdx = -1;
            else prevIdx--;
        }
        if (prevIdx >= 0) {
            float k = m_rightToLeft ? m_style.font->GetKerning(m_style.height, g[i], g[prevIdx])
                                    : m_style.font->GetKerning(m_style.height, g[prevIdx], g[i]);
            width += k * m_style.scale;
        }
        prevIdx = (int)i;
        width += m_style.scale * g[i].advance;
        if (width < maxWidth) { i++; continue; }
        if (words > 0 && wordStart > 0) {
            g.insert(g.begin() + wordStart, nl);
            i = (unsigned)++wordStart;
        } else if (wordStart < 0 && words != 0) {
            g[i] = nl;
            i++;
        } else {
            i++;
            continue;
        }
        lines++; words = 0; width = 0; prevIdx = -1;
    }
    return lines + 1;
}

// @0x6b2790: the width of the line starting at glyph idx
float Texter::GetLineWidth(unsigned idx, const std::vector<Glyph> &g) const {
    Params p;
    p.style = m_style;
    p.onlyOneLine = true;
    math::vec2<float> zero(0, 0);
    return Execute(p, zero, idx, g).x;
}

// @0x6b26fc
math::vec2<float> Texter::ComputeSize(const std::vector<Glyph> &g) const {
    Params p;
    p.style = m_style;
    math::vec2<float> zero(0, 0);
    Execute(p, zero, 0, g);
    return math::vec2<float>(p.maxX - p.minX, p.maxY - p.minY);
}

// @0x6b25f4
math::vec2<float> Texter::ComputeSize(std::vector<Glyph> &g, float maxWidth) const {
    if (maxWidth > 0) WordWrap(g, maxWidth);
    if (g.empty()) return math::vec2<float>(0, 0);
    return ComputeSize((const std::vector<Glyph>&)g);
}

// @0x6b2518
math::vec2<float> Texter::ComputeSize(const char *s, float maxWidth) const {
    if (!s || !*s) return math::vec2<float>(0, 0);
    m_style.font->GetGlyphs(m_glyphs, m_style.height, s);
    m_pages.clear();
    math::vec2<float> r(0, 0);
    if (!m_glyphs.empty()) {
        r = ComputeSize(m_glyphs, maxWidth);
        m_glyphs.clear();
    }
    return r;
}

math::vec2<float> Texter::ComputeSize(const char *s) const { return ComputeSize(s, 0.0f); }

static jet::video::Color ToColor(unsigned argb) {
    math::vec4<float> c = math::color::u32ToRgbaVec4<float>(argb);
    return jet::video::Color(c.x, c.y, c.z, c.w);
}

static void AlignLine(unsigned a, float lineWidth, float dir, float &x) {
    if (a & 0x20) { if (dir == 1.0f) x -= lineWidth; }
    else if (a & 8) x -= lineWidth * dir * 0.5f;
    else if (a & 4) { if (dir == -1.0f) x += lineWidth; }
}

// lays the glyphs out from glyph index i (a new line starts at every '\n') and, with a painter, draws them
// as one textured quad each; the result is the pen position after the last glyph. @0x6b3128
math::vec2<float> Texter::Execute(Params &p, const math::vec2<float> &pos, unsigned i, const std::vector<Glyph> &g) const {
    if (!p.style.font || g.empty()) return pos;
    Font &font = *p.style.font;
    const float lineH = font.GetLineGap(m_style.height) * m_style.scale;
    const float spaceW = font.GetSpaceWidth(m_style.height);
    float x = pos.x, y = pos.y, dir = 1.0f;
    if (!p.onlyOneLine) {
        dir = m_rightToLeft ? -1.0f : 1.0f;
        AlignLine(p.alignment, GetLineWidth(0, g), dir, x);
    }
    p.minX = p.maxX = x;
    p.minY = p.maxY = y;
    const float tabWidth = m_style.scale * 4.0f * spaceW * dir;
    const Glyph *prev = 0;
    int curPage = -1;
    boost::shared_ptr<jet::video::Texture> tex;
    jet::video::Material mat;
    if (p.painter) mat = p.painter->GetMaterial();
    auto addPoint = [&](float px, float py) {
        if (px < p.minX) p.minX = px;
        if (px > p.maxX) p.maxX = px;
        if (py < p.minY) p.minY = py;
        if (py > p.maxY) p.maxY = py;
    };
    for (;;) {
        if (i >= g.size()) {
            y += lineH;
            addPoint(x, y);
            return math::vec2<float>(x, y);
        }
        if (ParseColorTag(i, &p.color, g)) continue;
        if (p.style.decoration) p.color = p.style.decorationColor;
        const Glyph &c = g[i++];
        if (c.codepoint == '|') continue;
        if (c.codepoint == '\n') {
            if (p.onlyOneLine) return math::vec2<float>(x, y);
            y += lineH;
            x = pos.x;
            AlignLine(p.alignment, GetLineWidth(i, g), dir, x);
            continue;
        }
        if (c.codepoint == '\t') { x += tabWidth; continue; }
        float kern = 0;
        if (prev) {
            kern = (m_rightToLeft ? font.GetKerning(m_style.height, c, *prev) : font.GetKerning(m_style.height, *prev, c)) * m_style.scale;
        }
        x += m_rightToLeft ? -kern : kern;
        const float scale = p.style.scale;
        const float advance = scale * c.advance;
        if (p.painter) {
            const int page = (short)c.page;
            if (page >= 0 && page != curPage) {
                tex = m_pages[page];
                if (mat.GetTextureLayerCount(0)) {
                    mat.GetTextureLayer(0, 0).SetTexture(tex);
                    p.painter->SetMaterial(mat);
                }
                curPage = page;
            }
            if (!tex) { prev = &c; continue; }
            const float invW = 1.0f / (float)tex->GetSize().x, invH = 1.0f / (float)tex->GetSize().y;
            float qh = (float)c.h * scale;
            const float qw = (float)c.w * scale;
            float left = (float)c.left * scale;
            const float top = (float)c.top * scale;
            if (qh == 1.0f) qh = 2.0f;
            if (dir != 1.0f) left -= advance;
            const float px = x + left, py = y + top;
            const float u0 = (float)c.x * invW, u1 = (float)(c.x + c.w) * invW;
            const float v0 = (float)c.y * invH, v1 = (float)(c.y + c.h) * invH;
            const unsigned primary = p.color, second = p.style.gradientColor;
            unsigned col[4] = { primary, primary, primary, primary };
            switch (p.style.colorMode) {
            case ColorModeDef::GRADIENT_LR: col[2] = col[3] = second; break;
            case ColorModeDef::GRADIENT_RL: col[0] = col[1] = second; break;
            case ColorModeDef::GRADIENT_TB: col[1] = col[3] = second; break;
            case ColorModeDef::GRADIENT_BT: col[0] = col[2] = second; break;
            }
            typedef jet::video::Painter::Vertex V;
            V bl(math::vec3<float>(px, py + qh, 0), ToColor(col[0]), math::vec2<float>(u0, v1));
            V tl(math::vec3<float>(px, py, 0), ToColor(col[1]), math::vec2<float>(u0, v0));
            V br(math::vec3<float>(px + qw, py + qh, 0), ToColor(col[2]), math::vec2<float>(u1, v1));
            V tr(math::vec3<float>(px + qw, py, 0), ToColor(col[3]), math::vec2<float>(u1, v0));
            p.painter->FillQuad(tl, tr, br, bl);
        }
        x += dir * advance;
        addPoint(x, y);
        prev = &c;
    }
}

math::vec2<float> Texter::DrawString(jet::video::Painter &painter, const char *s, const math::vec2<float> &pos, safe_enum<AlignmentDef,AlignmentDef::type> align) const {
    return DrawString(painter, s, pos, -1.0f, align);
}

// @0x6b2c50
math::vec2<float> Texter::DrawString(jet::video::Painter &painter, const char *s, const math::vec2<float> &pos, float maxWidth, safe_enum<AlignmentDef,AlignmentDef::type> alignment) const {
    if (!s || !*s) return pos;
    const unsigned a = (unsigned)(AlignmentDef::type)alignment;
    Font &font = *m_style.font;
    font.GetGlyphs(m_glyphs, m_pages, m_style.height, s);
    if (m_glyphs.empty()) return pos;
    if (m_shaper) {
        std::vector<Glyph> shaped;
        m_shaper->Convert(m_glyphs, shaped);
        m_glyphs = shaped;
    }
    if (maxWidth > 0) WordWrap(m_glyphs, maxWidth);
    const unsigned lines = ComputeLineCount(m_glyphs);
    math::vec2<float> at = pos;
    const float scale = m_style.scale;
    if (lines == 1) {
        if (a & 0x40) at.y -= font.GetDescender(m_style.height) * scale;
        else if (a & 0x10) at.y += font.GetCenterHeight(m_style.height) * scale;
        else if (a & 2) at.y += font.GetAscender(m_style.height) * scale;
    } else if (a & 0x40) {
        at.y = at.y - font.GetDescender(m_style.height) * scale - ((float)(lines - 1) * scale) * font.GetLineGap(m_style.height);
    } else {
        if (a & 0x10) {
            at.y -= (scale * 0.5f * (float)(lines - 1)) * font.GetLineGap(m_style.height);
            at.y += font.GetCenterHeight(m_style.height) * scale;
        } else if (a & 2)
            at.y += font.GetAscender(m_style.height) * scale;
    }
    const unsigned deco = m_style.decoration;
    if (deco) {
        const int th = (int)m_style.decorationThickness;
        const int start = deco == 1 ? 1 : -th;
        for (int dx = start; dx <= th; dx++)
            for (int dy = start; dy <= th; dy++) {
                if (!(dx | dy)) continue;
                Params p;
                p.style = m_style;
                p.style.colorMode = 0;
                math::vec4<float> c1 = math::color::u32ToRgbaVec4<float>(m_style.primaryColor);
                math::vec4<float> c2 = math::color::u32ToRgbaVec4<float>(m_style.decorationColor);
                p.style.decorationColor = ((unsigned)(c2.w * c1.w * 255.0f) << 24) | ((unsigned)(c2.x * 255.0f) & 0xff) << 16 |
                                          (((unsigned)(c2.y * 255.0f) << 8) & 0xffff) | ((unsigned)(c2.z * 255.0f) & 0xff);
                p.painter = &painter;
                p.lineCount = lines;
                p.alignment = a;
                p.color = m_style.primaryColor;
                Execute(p, math::vec2<float>((float)dx + at.x, (float)dy + at.y), 0, m_glyphs);
            }
    }
    Params p;
    p.style = m_style;
    p.style.decoration = 0;
    p.painter = &painter;
    p.lineCount = lines;
    p.alignment = a;
    p.color = m_style.primaryColor;
    math::vec2<float> end = Execute(p, at, 0, m_glyphs);
    m_pages.clear();
    m_glyphs.clear();
    return end;
}

static bool IsArabic(unsigned c) {
    return (c - 0x600) <= 0xff || (c - 0x750) <= 0x2f || (c - 0xfb50) <= 0xef || (c - 0xfe70) <= 0x8c;
}

// characters that join the run in progress instead of ending it
static bool IsNeutral(unsigned c) {
    return (c - 0x20) <= 2 || (c - 0x3a) <= 1 || c == '?' || c == ']' || c == '[' || (c - 0x28) <= 7 || c == '&' || c == 0xa9;
}

static void FlushRun(std::deque<Glyph> &run, std::deque<Glyph> &out, bool back) {
    for (const Glyph &g : run) {
        if (back) out.push_back(g);
        else out.push_front(g);
    }
}

static void Commit(std::deque<Glyph> &out, std::vector<Glyph> &dst) {
    for (const Glyph &g : out) dst.push_back(g);
    out.clear();
}

// @0x65ed2c. state: 1 inside an Arabic chunk, 0 inside a Latin chunk, -1 at the start of a line
void ArabicShaper::Convert(const std::vector<Glyph> &in, std::vector<Glyph> &dst) {
    if (in.empty()) return;
    std::deque<Glyph> run, out;
    int state = -1;
    bool committed = false;
    const unsigned n = (unsigned)in.size();
    for (unsigned i = 0; i < n; i++) {
        const Glyph &g = in[i];
        const unsigned cp = g.codepoint;
        int strong;   // 1 Arabic, 0 Latin
        if (IsArabic(cp)) strong = 1;
        else if (cp == '\n') {
            FlushRun(run, out, state == 1);
            run.clear();
            out.push_back(g);
            Commit(out, dst);
            state = -1;
            continue;
        } else if (cp == '#') {
            FlushRun(run, out, state == 1);
            run.clear();
            out.push_back(g);
            unsigned k = 1;
            for (; i + k < n;) {
                const Glyph &d = in[i + k];
                unsigned c = d.codepoint;
                if (!((c - '0') <= 9 || ((c & ~0x20u) - 'A') <= 5)) { k--; break; }
                out.push_back(d);
                if (k == 6 || i + k + 1 >= n) break;
                k++;
            }
            i += k;
            continue;
        } else if (IsNeutral(cp)) {
            run.push_back(g);
            continue;
        } else strong = 0;
        if (state != strong && state != -1) {
            FlushRun(run, out, true);
            Commit(out, dst);
            committed = true;
        } else
            FlushRun(run, out, strong == 1);
        run.clear();
        if (strong == 1) out.push_back(g);
        else out.push_front(g);
        state = strong;
    }
    FlushRun(run, out, committed || state == 1);
    run.clear();
    Commit(out, dst);
}

}

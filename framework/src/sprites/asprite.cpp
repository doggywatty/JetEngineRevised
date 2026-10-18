// framework/src/sprites/asprite.cpp
#include "sprites/asprite.h"
#include <cmath>
#include <cstdio>

ASprite::ASprite() : m_currentPalette(0), m_currentColor(1, 1, 1, 1) {}
ASprite::~ASprite() {}

void ASprite::Unload() {
    m_moduleInfo.clear();
    m_framesNumberOfFModules.clear();
    m_framesFMStart.clear();
    m_fmodules.clear();
    m_animsTotalTime.clear();
    m_animsNumberAFrames.clear();
    m_animsFirstAFrameIndex.clear();
    m_aFrames.clear();
    m_frameRects.clear();
    m_frameRectsStart.clear();
    m_frameRectsNum.clear();
    m_paletteTextures.clear();
    m_paletteTexturesAlpha.clear();
    m_imagesName.clear();
    m_imagesInvSize.clear();
    m_moduleNames.clear();
    m_frameNames.clear();
    m_animNames.clear();
}

// ReadAuroraName @0x63e574: u8 length, characters
jet::String ASprite::ReadAuroraName(jet::stream::IStream &s) {
    unsigned char n = 0;
    s >> n;
    std::string str(n, '\0');
    if (n) s.Read(&str[0], n);
    return jet::String(str.c_str());
}

// Builds a transform as Load does: rotation (degrees), then the axes scaled by percent, then the
// offset. The flip flags stay in the record; painting doesn't read them.
static math::mat3<float> AuroraTransform(short rot, short sx, short sy, float x, float y) {
    math::mat3<float> m;
    m.setRotationPart((float)rot * 0.017453294f);
    const float kx = (float)sx * 0.01f, ky = (float)sy * 0.01f;
    m.m[0] *= kx; m.m[1] *= kx;
    m.m[3] *= ky; m.m[4] *= ky;
    m.m[6] = x; m.m[7] = y;
    return m;
}

// Load @0x63d84c
bool ASprite::Load(jet::stream::IStream &s) {
    Unload();
    unsigned short version = 0;
    int flags = 0, flags2 = 0;
    s >> version >> flags;
    if (flags & 0x400000) s >> flags2;

    if (flags2 & 0x400) {
        short count = 0;
        s >> count;
        for (int i = 0; i < count; i++) {
            m_imagesName.push_back(ReadAuroraName(s));
            if (flags2 & 0x10000) {
                short w = 0, h = 0;
                s >> w >> h;
                m_imagesInvSize.push_back(math::vec2<float>(1.0f / (float)w, 1.0f / (float)h));
            }
        }
    }

    unsigned short n = 0;
    s >> n;
    m_moduleInfo.resize(n);
    for (unsigned i = 0; i < n; i++) {
        ModuleInfo &m = m_moduleInfo[i];
        m.w = m.h = 0;
        s >> m.type >> m.x >> m.y;
        if (m.type != 0xfd) s >> m.w >> m.h;
        if (flags2 & 0x800) m_moduleNames[ReadAuroraName(s)] = (int)i;
    }

    s >> n;
    m_fmodules.resize(n);
    for (unsigned i = 0; i < n; i++) {
        FModuleData &f = m_fmodules[i];
        unsigned short module = 0;
        short x = 0, y = 0, rot = 0, sx = 100, sy = 100, ex = 0, ey = 0;
        s >> module >> x >> y >> f.flags;
        if (flags2 & 1) {
            s >> rot >> sx >> sy;
            if (flags2 & 0x2000) s >> ex >> ey;
        }
        f.module = (short)module;
        f.offset = math::vec2<float>((float)x, (float)y);
        f.transform = AuroraTransform(rot, sx, sy, (float)x, (float)y);
    }

    if (flags & 0x8000) {
        s >> n;
        m_frameRects.resize(n);
        for (unsigned i = 0; i < n; i++) s >> m_frameRects[i].x >> m_frameRects[i].y >> m_frameRects[i].w >> m_frameRects[i].h;
    }

    unsigned short frames = 0;
    s >> frames;
    m_framesNumberOfFModules.resize(frames);
    m_framesFMStart.resize(frames);
    unsigned short rectStart = 0;
    for (unsigned i = 0; i < frames; i++) {
        if (flags & 0x100000) s >> m_framesNumberOfFModules[i];
        else { unsigned char c = 0; s >> c; m_framesNumberOfFModules[i] = c; }
        s >> m_framesFMStart[i];
        if (flags & 0x8000) {
            unsigned char r = 0;
            s >> r;
            m_frameRectsStart.push_back(rectStart);
            m_frameRectsNum.push_back(r);
            rectStart += r;
        }
        if (flags2 & 0x80) m_frameNames[ReadAuroraName(s)] = (int)i;
    }
    // precomputed frame bounds, recomputed on demand here
    if (frames && !(flags & 0x1000)) s.SeekCur(frames * 8);

    s >> n;
    m_aFrames.resize(n);
    for (unsigned i = 0; i < n; i++) {
        AFrameData &a = m_aFrames[i];
        short x = 0, y = 0, rot = 0, sx = 100, sy = 100, ex = 0, ey = 0;
        if (flags & 0x200000) s >> a.frame;
        else { unsigned char c = 0; s >> c; a.frame = c; }
        s >> a.time >> x >> y >> a.flags;
        if (flags2 & 2) {
            s >> rot >> sx >> sy;
            if (flags2 & 0x2000) s >> ex >> ey;
        }
        // 8-bit frame indices borrow the two top flag bits
        if (!(flags & 0x200000)) a.frame |= (unsigned short)((a.flags & 0xc0) << 2);
        a.offset = math::vec2<float>((float)x, (float)y);
        a.transform = AuroraTransform(rot, sx, sy, (float)x, (float)y);
    }

    unsigned short anims = 0;
    s >> anims;
    m_animsNumberAFrames.resize(anims);
    m_animsFirstAFrameIndex.resize(anims);
    m_animsTotalTime.resize(anims);
    for (unsigned i = 0; i < anims; i++) {
        if (flags & 0x80000) s >> m_animsNumberAFrames[i];
        else { unsigned char c = 0; s >> c; m_animsNumberAFrames[i] = c; }
        s >> m_animsFirstAFrameIndex[i];
        unsigned total = 0;
        for (unsigned k = 0; k < m_animsNumberAFrames[i]; k++) total += GetAFrameTime((int)i, (int)k);
        m_animsTotalTime[i] = total;
        if (flags2 & 0x80) m_animNames[ReadAuroraName(s)] = (int)i;
    }
    (void)version;
    return true;
}

boost::shared_ptr<jet::video::Texture> ASprite::GetTexture(unsigned i) const {
    return i < m_paletteTextures.size() ? m_paletteTextures[i] : boost::shared_ptr<jet::video::Texture>();
}
boost::shared_ptr<jet::video::Texture> ASprite::GetAlphaTexture(unsigned i) const {
    return i < m_paletteTexturesAlpha.size() ? m_paletteTexturesAlpha[i] : boost::shared_ptr<jet::video::Texture>();
}

int ASprite::GetAFrameTime(int anim, int aframe) const {
    return m_aFrames[m_animsFirstAFrameIndex[anim] + aframe].time;
}

// the aframe showing at timeMs, looping (_PaintAnim @0x640a7c)
int ASprite::GetAFrame(int anim, int timeMs) const {
    unsigned total = m_animsTotalTime[anim] * k_animFrameTimeMs;
    if (!total) return 0;
    unsigned t = (unsigned)timeMs % total, acc = 0;
    int i = 0;
    while (acc <= t) acc += GetAFrameTime(anim, i++) * k_animFrameTimeMs;
    return i - 1;
}

int ASprite::GetAFrameFrame(int anim, int aframe) const {
    return m_aFrames[m_animsFirstAFrameIndex[anim] + aframe].frame;
}
const math::mat3<float> &ASprite::GetAFrameTransform(unsigned anim, unsigned aframe) const {
    return m_aFrames[m_animsFirstAFrameIndex[anim] + aframe].transform;
}
int ASprite::GetFModuleModuleId(int frame, int fmodule) const {
    return m_fmodules[m_framesFMStart[frame] + fmodule].module;
}
const math::mat3<float> &ASprite::GetFModuleTransform(int frame, int fmodule) const {
    return m_fmodules[m_framesFMStart[frame] + fmodule].transform;
}

static void Extend(math::rect<float> &r, bool &empty, const math::vec2<float> &p) {
    if (empty) { r.set(p.x, p.y, p.x, p.y); empty = false; return; }
    if (p.x < r.upperLeftCorner.x) r.upperLeftCorner.x = p.x;
    if (p.y < r.upperLeftCorner.y) r.upperLeftCorner.y = p.y;
    if (p.x > r.lowerRightCorner.x) r.lowerRightCorner.x = p.x;
    if (p.y > r.lowerRightCorner.y) r.lowerRightCorner.y = p.y;
}
static math::vec2<float> Xf(const math::mat3<float> &m, float x, float y) {
    return math::vec2<float>(m.m[0] * x + m.m[3] * y + m.m[6], m.m[1] * x + m.m[4] * y + m.m[7]);
}
static void Union(math::rect<float> &r, bool &empty, const math::rect<float> &o) {
    Extend(r, empty, o.upperLeftCorner);
    Extend(r, empty, o.lowerRightCorner);
}

void ASprite::GetModuleRect(math::rect<float> &r, unsigned module, const math::mat3<float> &m) const {
    const ModuleInfo &mi = m_moduleInfo[module];
    bool empty = true;
    Extend(r, empty, Xf(m, 0, 0));
    Extend(r, empty, Xf(m, mi.w, 0));
    Extend(r, empty, Xf(m, mi.w, mi.h));
    Extend(r, empty, Xf(m, 0, mi.h));
}

bool ASprite::GetFModuleRect(math::rect<float> &r, unsigned frame, unsigned fmodule, const math::mat3<float> &m) const {
    const FModuleData &f = m_fmodules[m_framesFMStart[frame] + fmodule];
    math::mat3<float> t = m * f.transform;
    if (f.flags & FLAG_HYPER_FRAME) { GetFrameRect(r, (unsigned)f.module, t); return true; }
    if (m_moduleInfo[f.module].type == 0xfd) return false;
    GetModuleRect(r, (unsigned)f.module, t);
    return true;
}

void ASprite::GetFrameRect(math::rect<float> &r, unsigned frame, const math::mat3<float> &m) const {
    bool empty = true;
    r.set(0, 0, 0, 0);
    for (unsigned i = 0; i < m_framesNumberOfFModules[frame]; i++) {
        math::rect<float> fr;
        if (GetFModuleRect(fr, frame, i, m)) Union(r, empty, fr);
    }
}

void ASprite::GetAFrameRect(math::rect<float> &r, unsigned anim, unsigned aframe, const math::mat3<float> &m) const {
    const AFrameData &a = m_aFrames[m_animsFirstAFrameIndex[anim] + aframe];
    GetFrameRect(r, a.frame, m * a.transform);
}

void ASprite::GetAnimRect(math::rect<float> &r, unsigned anim, const math::mat3<float> &m) const {
    bool empty = true;
    r.set(0, 0, 0, 0);
    for (unsigned i = 0; i < m_animsNumberAFrames[anim]; i++) {
        math::rect<float> ar;
        GetAFrameRect(ar, anim, i, m);
        Union(r, empty, ar);
    }
}

int ASprite::GetFrameLogicalRectsCount(int frame) const {
    return frame < (int)m_frameRectsNum.size() ? m_frameRectsNum[frame] : 0;
}

// @0x5862e8
int ASprite::GetAFrameLogicalRectsCount(int anim, int aframe) const {
    return aframe < (int)m_animsNumberAFrames[anim] ? GetFrameLogicalRectsCount(m_aFrames[m_animsFirstAFrameIndex[anim] + aframe].frame) : 0;
}

// @0x585d38: the frame's logical rect flipped as the aframe says, then moved by its offset
bool ASprite::GetAFrameLogicalRect(int anim, int aframe, int idx, math::rect<float> &out) const {
    const AFrameData &a = m_aFrames[m_animsFirstAFrameIndex[anim] + aframe];
    math::vec2<float> pos(0, 0), size(0, 0);
    if (!GetFrameLogicalRect(a.frame, idx, pos, size)) return false;
    float x1 = pos.x, y1 = pos.y, x2 = pos.x + size.x, y2 = pos.y + size.y;
    if (a.flags & FLAG_FLIP_X) { float t = x1; x1 = -x2; x2 = -t; }
    if (a.flags & FLAG_FLIP_Y) { float t = y1; y1 = -y2; y2 = -t; }
    out = math::rect<float>(x1 + a.offset.x, y1 + a.offset.y, x2 + a.offset.x, y2 + a.offset.y);
    return true;
}

bool ASprite::GetFrameLogicalRect(int frame, int idx, math::vec2<float> &pos, math::vec2<float> &size) const {
    if (idx >= GetFrameLogicalRectsCount(frame)) return false;
    const FrameRectData &r = m_frameRects[m_frameRectsStart[frame] + idx];
    pos = math::vec2<float>(r.x, r.y);
    size = math::vec2<float>(r.w, r.h);
    return true;
}

const jet::String &ASprite::GetImageName(unsigned i) const {
    static const jet::String empty;
    return i < m_imagesName.size() ? m_imagesName[i] : empty;
}

static int FindName(const std::map<jet::String, int> &m, const jet::String &n) {
    std::map<jet::String, int>::const_iterator it = m.find(n);
    return it == m.end() ? -1 : it->second;
}
int ASprite::FindModuleByName(const jet::String &n) const { return FindName(m_moduleNames, n); }
int ASprite::FindFrameByName(const jet::String &n) const { return FindName(m_frameNames, n); }
int ASprite::FindAnimByName(const jet::String &n) const { return FindName(m_animNames, n); }

// source/game/interfaces/aurora/AuroraLevelData.cpp
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "video/display.h"
#include "video/driver.h"

const jet::String AuroraLevelData::MISSING_AURORA_STR;
std::map<std::string, int> AuroraLevelData::s_auroraStringMap;
std::vector<jet::String> AuroraLevelData::s_auroraStrings;

AuroraLevelData::AuroraLevelData() : m_gameFileScreenSize(0, 0), m_displaySize(0, 0), m_aspectRatioScale(1, 1), m_scale(1), m_elementsCount(0) {}
AuroraLevelData::~AuroraLevelData() {}

void AuroraLevelData::Unload() {
    m_gameDataInitial.clear();
    m_gameDataRescaled.clear();
    m_gameDataRescaleFlags.clear();
    m_layerElemNames.clear();
    m_elementsCount = 0;
}

// @0x493c5c
const jet::String &AuroraLevelData::GetAuroraStringFromIndex(int i) {
    return i >= 0 && i < (int)s_auroraStrings.size() ? s_auroraStrings[i] : MISSING_AURORA_STR;
}

// @0x492ae0: 3 byte records {type, s16 value}; type 4/5 is a number, 8 a length prefixed string kept
// as an index into the shared string table. An element is 9 header values plus the count at [8].
void AuroraLevelData::Load(jet::stream::IStream &s) {
    s.Start();
    unsigned size = s.Size();
    std::vector<unsigned char> buf(size);
    s.Read(buf.data(), size);
    unsigned p = 0;
    int values = 0, total = 0;
    while (p < size) {
        if (values == 0) {
            m_gameDataInitial.push_back(std::vector<short>());
            m_gameDataRescaled.push_back(std::vector<short>());
            total = (short)(buf[p + 0x19] | ((signed char)buf[p + 0x1a] << 8)) + 9;
        }
        std::vector<short> &e = m_gameDataInitial.back();
        if (e.capacity() < 16) e.reserve(16);
        int t = (signed char)buf[p];
        if (t == 4 || t == 5) {
            e.push_back((short)(buf[p + 1] | ((signed char)buf[p + 2] << 8)));
            p += 3;
        } else if (t == 8) {
            int len = (signed char)buf[p + 1];
            p += 2;
            short idx = -1;
            if (len > 0) {
                std::string str((const char*)&buf[p], len);
                auto it = s_auroraStringMap.find(str);
                if (it != s_auroraStringMap.end()) idx = (short)it->second;
                else {
                    s_auroraStrings.push_back(jet::String(str.c_str()));
                    idx = (short)(s_auroraStrings.size() - 1);
                    s_auroraStringMap[str] = idx;
                }
                p += len;
            }
            e.push_back(idx);
        } else if (t != 8) {
            break;   // padding after the last element
        }
        if (++values == total) {
            m_gameDataRescaled.back().assign(e.size(), 0);
            m_elementsCount++;
            values = 0;
        }
    }
    if (values) { m_gameDataInitial.pop_back(); m_gameDataRescaled.pop_back(); }
    m_gameDataRescaleFlags.assign(m_elementsCount, 0);
    s.Finish();
}

// @0x492f00: level name, then one name per element
void AuroraLevelData::LoadElementNames(jet::stream::IStream &s) {
    m_layerElemNames.assign(m_layerElemNames.size(), jet::String());
    s >> m_layerName;
    int n = 0;
    s >> n;
    for (int i = 0; i < n; i++) {
        jet::String name;
        s >> name;
        m_layerElemNames.push_back(name);
    }
}

// @0x492fa0: the GUI was authored for gameW x gameH; scale is the uniform factor used for snapped elements
void AuroraLevelData::ResetGUIToDisplayData(float gameW, float gameH, float scale) {
    math::vec2<unsigned int> d = jet::video::Driver::GetCurrent()->GetDisplay()->GetSize();
    m_gameFileScreenSize = math::vec2<float>(gameW, gameH);
    m_displaySize = math::vec2<float>((float)d.x, (float)d.y);
    m_scale = scale;
    m_aspectRatioScale = math::vec2<float>(m_displaySize.x / gameW, m_displaySize.y / gameH);
    if (m_elementsCount) _RescaleAll();
}

int AuroraLevelData::GetElementParamCount(unsigned e) const { return m_gameDataInitial[e][8] + 9; }

// @0x4939c8 / 0x493a18: the parameters holding an element's width and height (types 1 2 4 5 6 9 13 -> 9/10, scroll bar 14/15)
int AuroraLevelData::GetElementWidthParameterId(unsigned e) {
    int t = GetParamValue(e, 0) - 1;
    if (t < 0 || t > 12) return -1;
    if ((1 << t) & 0x113b) return 9;
    return (1 << t) & 0x400 ? 14 : -1;
}
int AuroraLevelData::GetElementHeightParameterId(unsigned e) {
    int w = GetElementWidthParameterId(e);
    return w < 0 ? -1 : w + 1;
}

// @0x493a68: the parameter that names the parent element depends on the type
int AuroraLevelData::GetElementParentId(unsigned e) {
    static const signed char parentParam[16] = { 12, 11, 15, 13, 17, 17, 17, 13, 15, 18, 11, 12, 10, 11, 11, 10 };
    int t = GetParamValue(e, 0);
    if (t > 15 || parentParam[t] < 0) return -1;
    return GetParamValue(e, parentParam[t]);
}

int AuroraLevelData::GetUniqueIdElemPos(unsigned id) {
    for (unsigned i = 0; i < m_elementsCount; i++)
        if ((int)id == GetParamValue(i, 1)) return (int)i;
    return -1;
}

const jet::String &AuroraLevelData::GetUniqueIdElemName(unsigned id) {
    int p = GetUniqueIdElemPos(id);
    return p < 0 || p >= (int)m_layerElemNames.size() ? MISSING_AURORA_STR : m_layerElemNames[p];
}

int AuroraLevelData::GetElementIndex(const jet::String &name) {
    for (unsigned i = 0; i < m_layerElemNames.size(); i++)
        if (m_layerElemNames[i].Equals(name)) return (int)i;
    return -1;
}

// @0x493b70: where an anchor (3..13 around the screen edge) sits in a w x h area
math::vec2<float> AuroraLevelData::_GetAnchorOffset(int anchor, math::vec2<float> s) {
    switch (anchor) {
    case 3: return math::vec2<float>(0, s.y * 0.5f);
    case 4: case 11: return math::vec2<float>(0, s.y);
    case 5: case 6: return math::vec2<float>(s.x, 0);
    case 7: return math::vec2<float>(s.x, s.y * 0.5f);
    case 8: return math::vec2<float>(s.x, s.y);
    case 10: return math::vec2<float>(s.x * 0.5f, 0);
    case 12: return math::vec2<float>(s.x * 0.5f, s.y);
    case 13: return math::vec2<float>(s.x * 0.5f, s.y * 0.5f);
    default: return math::vec2<float>(0, 0);
    }
}
math::vec2<float> AuroraLevelData::_GetGameFileAnchorOffset(int a) { return _GetAnchorOffset(a, m_gameFileScreenSize); }
math::vec2<float> AuroraLevelData::_GetDisplayAnchorOffset(int a) { return _GetAnchorOffset(a, m_displaySize); }

// @0x493998: the element this one is snapped to (its explicit snap id, else its parent)
int AuroraLevelData::_PostInitGetAnchorSnapId(unsigned e) {
    int s = GetParamValue(e, 5);
    if (s > 0) return s;
    int p = GetElementParentId(e);
    return p < 0 ? 0 : p;
}

// per element flags: 1 upper left in progress, 2 upper left done, 4 lower right in progress, 8 lower right done
// @0x493060
void AuroraLevelData::_RescaleAll() {
    for (unsigned e = 0; e < m_elementsCount; e++) {
        m_gameDataRescaleFlags[e] = 0;
        for (int p = 0; p < GetElementParamCount(e); p++) m_gameDataRescaled[e][p] = m_gameDataInitial[e][p];
    }
    for (unsigned e = 0; e < m_elementsCount; e++) _PostInitElementUpperLeftAnchorScreen(e);
    for (unsigned e = 0; e < m_elementsCount; e++) _PostInitElementUpperLeftAnchorSnap(e);
    for (unsigned e = 0; e < m_elementsCount; e++) _PostInitElementLowerRightAnchorScreen(e);
    for (unsigned e = 0; e < m_elementsCount; e++) _PostInitElementLowerRightAnchorSnap(e);
}

// elements anchored to a screen edge (or to nothing) are scaled by the aspect ratio around that edge. @0x493170
void AuroraLevelData::_PostInitElementUpperLeftAnchorScreen(unsigned e) {
    int anchor = GetParamValue(e, 4);
    if (!anchor && _PostInitGetAnchorSnapId(e) > 0) return;
    m_gameDataRescaleFlags[e] &= ~1;
    m_gameDataRescaleFlags[e] |= 2;
    math::vec2<float> g = _GetGameFileAnchorOffset(anchor), d = _GetDisplayAnchorOffset(anchor);
    _SetParamValue(e, 2, (int)(d.x + ((float)GetInitialParamValue(e, 2) - g.x) * m_aspectRatioScale.x));
    _SetParamValue(e, 3, (int)(d.y + ((float)GetInitialParamValue(e, 3) - g.y) * m_aspectRatioScale.y));
    int wid = GetElementWidthParameterId(e), hid = GetElementHeightParameterId(e);
    if (wid >= 0 && hid >= 0) {
        float w = (float)GetInitialParamValue(e, wid), h = (float)GetInitialParamValue(e, hid);
        _SetParamValue(e, wid, (int)(w * m_aspectRatioScale.x));
        _SetParamValue(e, hid, (int)(h * m_aspectRatioScale.y));
    }
}

// elements snapped to another element keep their offset from it, scaled uniformly. @0x49332c
void AuroraLevelData::_PostInitElementUpperLeftAnchorSnap(unsigned e) {
    if (m_gameDataRescaleFlags[e] & 2) return;
    if (m_gameDataRescaleFlags[e] & 1) return;
    m_gameDataRescaleFlags[e] |= 1;
    int anchor = GetParamValue(e, 4);
    int snap = _PostInitGetAnchorSnapId(e);
    if (!anchor && snap <= 0) return;
    int s = GetUniqueIdElemPos(snap);
    if (!(m_gameDataRescaleFlags[s] & 2)) _PostInitElementUpperLeftAnchorSnap(s);
    float x = ((float)(GetParamValue(e, 2) - GetInitialParamValue(s, 2))) * m_scale + (float)GetParamValue(s, 2);
    float y = ((float)(GetParamValue(e, 3) - GetInitialParamValue(s, 3))) * m_scale + (float)GetParamValue(s, 3);
    _SetParamValue(e, 2, (int)x);
    _SetParamValue(e, 3, (int)y);
    int wid = GetElementWidthParameterId(e), hid = GetElementHeightParameterId(e);
    if (wid >= 0 && hid >= 0) {
        float w = (float)GetParamValue(e, wid), h = (float)GetParamValue(e, hid);
        _SetParamValue(e, wid, (int)(w * m_scale));
        _SetParamValue(e, hid, (int)(h * m_scale));
    }
    m_gameDataRescaleFlags[e] &= ~1;
    m_gameDataRescaleFlags[e] |= 2;
}

// the lower right corner anchored to a screen edge: width and height follow from it. @0x493578
void AuroraLevelData::_PostInitElementLowerRightAnchorScreen(unsigned e) {
    int wid = GetElementWidthParameterId(e), hid = GetElementHeightParameterId(e);
    if (wid <= 0 || hid <= 0) return;
    int anchor = GetParamValue(e, 6);
    if (!anchor && GetParamValue(e, 7) >= 0) return;
    m_gameDataRescaleFlags[e] &= ~4;
    m_gameDataRescaleFlags[e] |= 8;
    math::vec2<float> g = _GetGameFileAnchorOffset(anchor), d = _GetDisplayAnchorOffset(anchor);
    float right = ((float)(GetInitialParamValue(e, 2) + GetInitialParamValue(e, wid)) - g.x) * m_aspectRatioScale.x;
    float bottom = ((float)(GetInitialParamValue(e, 3) + GetInitialParamValue(e, hid)) - g.y) * m_aspectRatioScale.y;
    float w = d.x - (float)GetParamValue(e, 2) + right;
    float h = d.y - (float)GetParamValue(e, 3) + bottom;
    _SetParamValue(e, wid, (int)w);
    _SetParamValue(e, hid, (int)h);
    m_gameDataRescaleFlags[e] &= ~4;
    m_gameDataRescaleFlags[e] |= 8;
}

// the lower right corner snapped to another element's upper left. @0x493754
void AuroraLevelData::_PostInitElementLowerRightAnchorSnap(unsigned e) {
    if (m_gameDataRescaleFlags[e] & 8) return;
    int wid = GetElementWidthParameterId(e), hid = GetElementHeightParameterId(e);
    if (wid <= 0 || hid <= 0) return;
    if (m_gameDataRescaleFlags[e] & 4) return;
    m_gameDataRescaleFlags[e] |= 4;
    int anchor = GetParamValue(e, 6), snap = GetParamValue(e, 7);
    if (anchor > 0) return;
    if (snap <= 0 && anchor == 0) return;
    int s = GetUniqueIdElemPos(snap);
    int right = GetInitialParamValue(e, 2) + GetInitialParamValue(e, wid);
    int sx = GetInitialParamValue(s, 2);
    int bottom = GetInitialParamValue(e, 3) + GetInitialParamValue(e, hid);
    int sy = GetInitialParamValue(s, 3);
    float w = (float)(GetParamValue(s, 2) - GetParamValue(e, 2)) + (float)(right - sx) * m_scale;
    float h = (float)(GetParamValue(s, 3) - GetParamValue(e, 3)) + (float)(bottom - sy) * m_scale;
    _SetParamValue(e, wid, (int)w);
    _SetParamValue(e, hid, (int)h);
    m_gameDataRescaleFlags[e] &= ~4;
    m_gameDataRescaleFlags[e] |= 8;
}

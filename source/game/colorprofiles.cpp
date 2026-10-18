// source/game/colorprofiles.cpp
#include "colorprofiles.h"
#include "CGameConfig.h"
#include "core/pstring.h"
#include "core/util.h"
#include "stream/istream.h"
#include "stream/streammgr.h"
#include "system.h"
#include "video/display.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

// @0xcff024
const char *colorProfilePostfix[8] = { "_minion.conf", "_onlap.conf", "_res.conf", "_macho.conf", "_beach.conf",
                                       "_banana.conf", "_resNight.conf", "_mall.conf" };

// @0x3e775c
int ReplaceChar(char *s, char from, char to) {
    int n = 0;
    for (; *s; s++)
        if (*s == from) {
            *s = to;
            n++;
        }
    return n;
}

// @0x3e7788
static math::mat4<float> GetXRotMat(float angle) {
    math::mat4<float> r;
    const float c = (float)cos((double)angle), s = (float)sin((double)angle);
    r.m[5] = c;
    r.m[6] = -s;
    r.m[9] = s;
    r.m[10] = c;
    return r;
}

// @0x3e7818
static math::mat4<float> GetYRotMat(float angle) {
    math::mat4<float> r;
    const float c = (float)cos((double)angle), s = (float)sin((double)angle);
    r.m[0] = c;
    r.m[2] = s;
    r.m[8] = -s;
    r.m[10] = c;
    return r;
}

// @0x3e78a8
static math::mat4<float> GetZRotMat(float angle) {
    math::mat4<float> r;
    const float c = (float)cos((double)angle), s = (float)sin((double)angle);
    r.m[0] = c;
    r.m[1] = -s;
    r.m[4] = s;
    r.m[5] = c;
    return r;
}

static void Transpose(math::mat4<float> &m) {
    for (int r = 0; r < 4; r++)
        for (int c = r + 1; c < 4; c++) std::swap(m.m[c * 4 + r], m.m[r * 4 + c]);
}

// @0x3e7938: a = {hue in degrees, saturation, brightness}, b = saturation per channel (added to a.y), c = offset
// per channel (added to a.z - 1), d = gain per channel (added to e)
math::mat4<float> ComputeColorOffsetMatrix(const math::vec3<float> &a, const math::vec3<float> &b, const math::vec3<float> &c,
                                           const math::vec3<float> &d, float e) {
    // the hue: a rotation about the grey axis (the diagonal of the colour cube taken to the z axis)
    math::mat4<float> xRot = GetXRotMat(0.785398185f), yRot = GetYRotMat(-0.615478158f);
    math::mat4<float> hue = xRot * yRot;
    hue = hue * GetZRotMat(a.x * 0.0174532942f);
    Transpose(xRot);
    Transpose(yRot);
    hue = hue * yRot;
    hue = hue * xRot;
    // the saturation: the luminance (0.3086, 0.6094, 0.082) mixed with each channel
    const float sr = a.y + b.x, sg = a.y + b.y, sb = a.y + b.z;
    math::mat4<float> sat;
    sat.m[0] = (1.0f - sr) * 0.3086f; sat.m[1] = (1.0f - sg) * 0.3086f; sat.m[2] = (1.0f - sb) * 0.3086f; sat.m[3] = 0;
    sat.m[4] = (1.0f - sr) * 0.6094f; sat.m[5] = (1.0f - sg) * 0.6094f; sat.m[6] = (1.0f - sb) * 0.6094f; sat.m[7] = 0;
    sat.m[8] = (1.0f - sr) * 0.082f;  sat.m[9] = (1.0f - sg) * 0.082f;  sat.m[10] = (1.0f - sb) * 0.082f; sat.m[11] = 0;
    sat.m[12] = sat.m[13] = sat.m[14] = 0;
    sat.m[15] = 1.0f;
    sat.m[0] += sr;
    sat.m[5] += sg;
    sat.m[10] += sb;
    math::mat4<float> gain;
    gain.m[0] = e + d.x;
    gain.m[5] = e + d.y;
    gain.m[10] = e + d.z;
    math::mat4<float> out = hue * gain;
    out = out * sat;
    out.m[12] = a.z + c.x + -1.0f;
    out.m[13] = a.z + c.y + -1.0f;
    out.m[14] = a.z + c.z + -1.0f;
    return out;
}

// @0x3e7eac: the last line of the file holds, after the three words of the device (its maker, model and system
// version), the thirteen numbers of the profile
void LoadProfile(const char *file, int index) {
    jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(jet::String(file));
    if (!s) {
        g_ColorCorrectionMatrix[index].setIdentity();
        return;
    }
    s->Start();
    const unsigned size = s->Size();
    std::string text(size, '\0');
    s->Read(&text[0], size);
    s->Finish();
    delete s;

    // @0x3f0974: the empty pieces are left out
    std::vector<std::string> lines = jet::core::Tokenize(text, '\n');
    while (!lines.empty() && lines.back().size() <= 1) lines.pop_back();
    if (lines.empty()) return;
    std::vector<std::string> words = jet::core::Tokenize(lines.back(), ' ');
    float v[12] = { 0 }, e = 0;  // the original leaves `e` unset when the line is too short
    if (words.size() > 15) {
        const size_t n = words.size();
        ReplaceChar(&words[n - 16][0], '_', ' ');
        ReplaceChar(&words[n - 15][0], '_', ' ');
        ReplaceChar(&words[n - 14][0], '_', ' ');
        for (int i = 0; i < 12; i++) sscanf(words[n - 13 + i].c_str(), "%f", &v[i]);
        v[0] = 360.0f - v[0];
        sscanf(words[n - 1].c_str(), "%f", &e);
    }
    g_ColorCorrectionMatrix[index] = ComputeColorOffsetMatrix(math::vec3<float>(v[0], v[1], v[2]), math::vec3<float>(v[3], v[4], v[5]),
                                                              math::vec3<float>(v[6], v[7], v[8]), math::vec3<float>(v[9], v[10], v[11]), e);
}

// @0x3e83e8
void LoadProfile(const char *base) {
    for (int i = 0; i < 8; i++) {
        std::string file(base);
        file += colorProfilePostfix[i];
        LoadProfile(file.c_str(), i);
    }
}

static bool IsOneOf(const jet::String &model, std::initializer_list<const char *> names) {
    for (const char *n : names)
        if (model.Equals(n)) return true;
    return false;
}

// @0x3e8470: the files are chosen by the model the phone reports, exactly as it spells it
void LoadProfile() {
    jet::String manufacturer(CGameConfig::GetPhoneManufacturerPointer());
    jet::String model(CGameConfig::GetPhoneModelPointer());
    jet::String firmware(CGameConfig::GetPhoneFirmwarePointer());
    firmware = firmware.substr(0, 3);  // the version is not used afterwards
    // these two keep the state the globals have
    if (model.Equals("DROIDX") || model.Equals("Galaxy Nexus")) return;
    g_ColorCorrectionEnable = false;
    const char *base = 0;
    if (IsOneOf(model, { "GT-i9000", "GT-i9000B", "GT-i9000T", "GT-I9001" })) base = "pf_i9000";
    else if (IsOneOf(model, { "GT-i9100", "GT-i9100M", "GT-i9100G", "GT-i9100P", "GT-i9100T", "GT-i9101", "GT-i9210", "Galaxy S II", "Galaxy S2" }))
        base = "pf_i9100";
    else if (IsOneOf(model, { "GT-i9300", "Galaxy S III London Olympic Games Premium Edition", "GT-I9300T", "GT-I9303T", "GT-i9305",
                              "GT-i9305T", "Galaxy S3", "Galaxy S III" }))
        base = "pf_i9300";
    else if (IsOneOf(model, { "Galaxy Note", "GT-N7000", "GT-N7105", "GT-i9220", "GT-N7100", "Galaxy Note 2", "Galaxy Note II", "Note 2",
                              "SGH-I717", "Galaxy Note I717" }))
        base = "pf_galaxy_note";
    else if (IsOneOf(model, { "Nexus 7", "Google Nexus 7" })) {
        // only the first matrix, from a file named without the kind of profile
        g_ColorCorrectionEnable = true;
        // the height of the native window (Android_GetNativeWindowSize @0x3e74d4) tells the two Nexus 7
        const unsigned height = jet::System::GetDisplay(0)->GetSize().y;
        LoadProfile(height > 0x500 ? "pf_nexus7_2" : "pf_nexus7", 0);
        return;
    }
    else if (model.Equals("GT-I9505G")) base = "pf_i9505";
    else if (model.Equals("GT-I9500")) base = "pf_i9500";
    else if (model.Equals("Nexus 10")) base = "pf_Nexus10";
    else if (IsOneOf(model, { "R800i", "R800at", "R800x", "R800a", "Xperia Play" })) base = "pf_xperia_play";
    else if (model.Equals("Nexus 4")) base = "pf_nexus4";
    else if (IsOneOf(model, { "Kindle Fire", "KFTT" })) base = "pf_kindle_fire";
    else {
        for (int i = 0; i < 5; i++) g_ColorCorrectionMatrix[i].setIdentity();
        return;
    }
    g_ColorCorrectionEnable = true;
    LoadProfile(base);
}

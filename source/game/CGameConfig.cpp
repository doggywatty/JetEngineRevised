// source/game/CGameConfig.cpp
#include "CGameConfig.h"
#include "video/driver.h"
#include "video/display.h"
#include "system.h"
#include "colorprofiles.h"
#include <json/json.h>
#include <strings.h>

static void ApplyToggles(const Json::Value &p, PlatformToggles &t) {
    if (!p.isObject()) return;
#define INT(k) if (p.isMember(#k)) t.k = p[#k].asInt()
#define FLT(k) if (p.isMember(#k)) t.k = (float)p[#k].asDouble()
#define BOOL(k) if (p.isMember(#k)) t.k = p[#k].asInt() != 0
    INT(textureStartLOD); FLT(size); FLT(downsamplePercent); INT(msaa); BOOL(usePostFX);
    INT(defaultTextureFiltering); INT(useShadows); BOOL(useLowMaterials); BOOL(useLowModels);
    BOOL(useLowParticles); BOOL(hideOptionalDecos); FLT(decoGenerationGroupAllowedDecosLerp);
    BOOL(useSpecularMask); BOOL(useRimLight); BOOL(useCurvature); BOOL(useLinearSpace);
    BOOL(useSpecularLight); BOOL(useEmissiveness); BOOL(useBarla); BOOL(useNdis); BOOL(forceRoadShader);
    BOOL(useIngameUITextEffect); BOOL(useUIDespicableScoreParticles); INT(maxOutline);
    FLT(backgroundFarPlane); FLT(decosAnimationRadius); FLT(sceneMgrCullingDistance);
    FLT(sceneMgrCullingRadius); FLT(sceneMgrCullingScreenPixels);
    BOOL(useLessSounds); BOOL(useLessBananaSounds);
    if (p.isMember("useCheapNdis")) t.useCheapNdis = p["useCheapNdis"].asInt() != 0;
#undef INT
#undef FLT
#undef BOOL
}

static std::string s_phoneManufacturer, s_phoneModel, s_phoneFirmware;
const char *CGameConfig::GetPhoneManufacturerPointer() { return s_phoneManufacturer.c_str(); }  // @0x3e9234
const char *CGameConfig::GetPhoneModelPointer() { return s_phoneModel.c_str(); }                // @0x3e9274
const char *CGameConfig::GetPhoneFirmwarePointer() { return s_phoneFirmware.c_str(); }          // @0x3e9284
void CGameConfig::SetPhoneInfo(const std::string &manufacturer, const std::string &model, const std::string &firmware) {
    s_phoneManufacturer = manufacturer;
    s_phoneModel = model;
    s_phoneFirmware = firmware;
}

PlatformToggles &CGameConfig::Current() {
    static PlatformToggles t;
    return t;
}

const char *CGameConfig::FontConfigFor(unsigned w, unsigned h) {
    const char *def = "font_android_default.cfg";
    if (w < 400) {
        if (h < 700) return "font_android_480x320.cfg";
    } else if (w > 600) {
        if (w > 800) return def;
        return h > 1280 ? def : "font_android_1280x720.cfg";
    }
    if (h < 960) return "font_android_960x540.cfg";
    return h > 1280 ? def : "font_android_1280x720.cfg";
}

bool CGameConfig::ComputePlatformToggles(const std::string &json, const std::string &device, PlatformToggles &t) {
    t.fontConfig = "font_android_default.cfg";
    if (jet::video::Display *d = jet::System::GetDisplay(0)) {
        math::vec2<unsigned int> sz = d->GetSize();
        t.fontConfig = FontConfigFor(sz.x, sz.y);
    }
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(json, root)) return false;
    // GetCompatMaster @0x3e931c: case-insensitive device lookup
    std::string master = device;
    const Json::Value &compats = root["compats"];
    for (const std::string &k : compats.getMemberNames())
        if (!strcasecmp(k.c_str(), device.c_str())) { master = compats[k].asString(); break; }
    int mem = 0, cpu = 0, gpu = 0;
    const Json::Value &nc = root["noncompats"];
    for (const std::string &k : nc.getMemberNames())
        if (!strcasecmp(k.c_str(), master.c_str())) {
            mem = nc[k]["mem"].asInt();
            cpu = nc[k]["cpu"].asInt();
            gpu = nc[k]["gpu"].asInt();
            break;
        }
    const Json::Value &profiles = root["Profiles"];
    ApplyToggles(profiles["GPU_" + std::to_string(gpu)], t);
    ApplyToggles(profiles["CPU_" + std::to_string(cpu)], t);
    ApplyToggles(profiles["MEM_" + std::to_string(mem)], t);
    if (profiles.isMember(master)) ApplyToggles(profiles[master], t);
    return true;
}

// Game::PostInit @0x528e1c..
void CGameConfig::AddShaderDefines(const PlatformToggles &t, jet::video::Driver *d) {
    if (!d) return;
    if (t.useShadows > 0) d->AddShaderDefine(jet::String("HAS_SHADOWS"));
    if (t.useSpecularMask) d->AddShaderDefine(jet::String("HAS_SPECULAR_MASK"));
    if (t.useRimLight) d->AddShaderDefine(jet::String("HAS_RIM_LIGHT"));
    if (t.useCurvature) d->AddShaderDefine(jet::String("HAS_CURVATURE"));
    if (t.useLinearSpace) d->AddShaderDefine(jet::String("HAS_LINEAR_SPACE"));
    if (t.useSpecularLight) d->AddShaderDefine(jet::String("HAS_SPECULAR_LIGHT"));
    if (t.useEmissiveness) d->AddShaderDefine(jet::String("HAS_EMISSIVENESS"));
    if (t.useBarla) d->AddShaderDefine(jet::String("HAS_BARLA"));
    if (t.useNdis) d->AddShaderDefine(jet::String("HAS_NDIS"));
    if (t.useCheapNdis) d->AddShaderDefine(jet::String("CHEAP_NDIS"));
    d->AddShaderDefine(jet::String("USE_COLORCORRECTION"));  // @0x52903c
    if (g_ColorCorrectionEnable) d->AddShaderDefine(jet::String("USE_COLORCORRECTION"));  // @0x529064
    if (t.forceRoadShader) d->AddShaderDefine(jet::String("HAS_ROAD_SHADER"));
}

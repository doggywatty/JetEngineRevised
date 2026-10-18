// source/game/CGameConfig.h
#ifndef SOURCE_GAME_CGAMECONFIG_H
#define SOURCE_GAME_CGAMECONFIG_H

#include <string>

namespace jet::video { class Driver; }

// Per-device quality toggles from profiles.json (CGameConfig::ApplyTogglesFromProfile @0x3e96ac)
struct PlatformToggles {
    int textureStartLOD = 0;
    float size = 1.0f;
    float downsamplePercent = 0.0f;
    int msaa = 0;
    bool usePostFX = true;
    int defaultTextureFiltering = 2;
    int useShadows = 0;
    bool useLowMaterials = false;
    bool useLowModels = false;
    bool useLowParticles = false;
    bool hideOptionalDecos = false;
    float decoGenerationGroupAllowedDecosLerp = 1.0f;
    bool useSpecularMask = false;
    bool useRimLight = false;
    bool useCurvature = false;
    bool useLinearSpace = false;
    bool useSpecularLight = false;
    bool useEmissiveness = false;
    bool useBarla = false;
    bool useNdis = false;
    bool useCheapNdis = false;
    bool forceRoadShader = false;
    bool useLessSounds = false;  // the banks of the soundpack are cut (GameSoundMgr::Init)
    bool useLessBananaSounds = false;
    bool useIngameUITextEffect = true;
    bool useUIDespicableScoreParticles = true;
    int maxOutline = -1;
    float backgroundFarPlane = 1000.0f;
    float decosAnimationRadius = 200.0f;
    float sceneMgrCullingDistance = 90.0f;
    float sceneMgrCullingRadius = 10.0f;
    float sceneMgrCullingScreenPixels = 3.0f;
    // Game+0x3f4 and +0x3f8 (set in the Game constructor, not by the profiles): how far behind the minion a template that was left is
    // kept (120), and what is added to it while the level fast forwards (0)
    float templateRemoveDistance = 120.0f;
    float templateRemoveDistanceFastForward = 0.0f;
    std::string fontConfig = "font_android_default.cfg";  // the font size policy for the window size (Game+0x3e0)
};

class CGameConfig {
public:
    // the running device's toggles (Game keeps them from PostInit on)
    static PlatformToggles &Current();
    // ComputePlatformToggles @0x3ea260: the device model is mapped through "compats" to its
    // master, "noncompats" gives its {mem, cpu, gpu} levels, then the GPU_<n>, CPU_<n> and
    // MEM_<n> profiles and finally a profile named after the device apply in that order.
    static bool ComputePlatformToggles(const std::string &profilesJson, const std::string &device, PlatformToggles &out);
    // @0x3ea3c0: the font policy file for a native window size (portrait width x height)
    static const char *FontConfigFor(unsigned width, unsigned height);
    // @0x3e9234, @0x3e9274, @0x3e9284: what the phone reports (nativeGetPhoneModel @0x3f564c copies it in
    // from the Java side); empty until SetPhoneInfo
    static const char *GetPhoneManufacturerPointer();
    static const char *GetPhoneModelPointer();
    static const char *GetPhoneFirmwarePointer();
    static void SetPhoneInfo(const std::string &manufacturer, const std::string &model, const std::string &firmware);
    // The shader defines Game::PostInit adds for the toggles
    static void AddShaderDefines(const PlatformToggles &t, jet::video::Driver *d);
};

#endif

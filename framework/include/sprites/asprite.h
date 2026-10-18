// framework/include/sprites/asprite.h
#ifndef FRAMEWORK_INCLUDE_SPRITES_ASPRITE_H
#define FRAMEWORK_INCLUDE_SPRITES_ASPRITE_H

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "core/pstring.h"
#include "video/color.h"
#include "video/texture.h"
#include "stream/istream.h"

// Aurora sprite (.bsprite, version 0x05DF), ASprite::Load @0x63d84c. Modules are atlas rectangles,
// frames list fmodules (a module, or a frame when the fmodule flag 0x10 is set, placed by a 2D
// transform), anims list aframes (a frame, a duration and a transform).
class ASprite {
public:
    struct ModuleInfo {
        unsigned char type;             // 0xFD: no rectangle, not drawn
        unsigned short x, y, w, h;
    };
    struct FModuleData {
        short module;
        math::vec2<float> offset;
        unsigned char flags;
        math::mat3<float> transform;
    };
    struct AFrameData {
        unsigned short frame;
        unsigned char time;
        math::vec2<float> offset;
        unsigned char flags;
        math::mat3<float> transform;
    };
    struct FrameRectData { short x, y; unsigned short w, h; };

    static const int FLAG_FLIP_X = 1, FLAG_FLIP_Y = 2, FLAG_ROT_90 = 4, FLAG_HYPER_FRAME = 0x10;
    // aframe durations are in 1/30 s ticks of 33 ms (_PaintAnim @0x640a7c)
    static const int k_animFrameTimeMs = 33;

    ASprite();
    virtual ~ASprite();
    virtual void Unload();
    virtual bool Load(jet::stream::IStream&);

    int GetNumberOfModules() const { return (int)m_moduleInfo.size(); }
    int GetNumberOfFrames() const { return (int)m_framesNumberOfFModules.size(); }
    int GetNumberOfAnims() const { return (int)m_animsNumberAFrames.size(); }
    int GetNumberOfAnimationFrames(int anim) const { return m_animsNumberAFrames[anim]; }
    int GetNumberOfFrameModules(int frame) const { return m_framesNumberOfFModules[frame]; }
    unsigned GetTextureCount() const { return (unsigned)m_paletteTextures.size(); }
    boost::shared_ptr<jet::video::Texture> GetTexture(unsigned) const;
    boost::shared_ptr<jet::video::Texture> GetAlphaTexture(unsigned) const;
    int GetNumberOfPalettes() const { return (int)m_paletteTextures.size(); }
    void SetCurrentPalette(int p) { m_currentPalette = p; }
    int GetCurrentPalette() const { return m_currentPalette; }
    void SetCurrentColor(const jet::video::Color &c) { m_currentColor = c; }
    const jet::video::Color &GetCurrentColor() const { return m_currentColor; }

    unsigned GetATotalTime(int anim) const { return m_animsTotalTime[anim]; }
    int GetAFrameTime(int anim, int aframe) const;
    int GetAFrame(int anim, int timeMs) const;
    int GetAFrameFrame(int anim, int aframe) const;
    // the first module of the frame an animation frame shows
    int GetAFrameFrameFirstModule(int anim, int aframe) const { return GetFModuleModuleId(GetAFrameFrame(anim, aframe), 0); }
    const math::mat3<float> &GetAFrameTransform(unsigned anim, unsigned aframe) const;
    int GetFModuleModuleId(int frame, int fmodule) const;
    const math::mat3<float> &GetFModuleTransform(int frame, int fmodule) const;
    const ModuleInfo &GetModule(int m) const { return m_moduleInfo[m]; }

    void GetModuleRect(math::rect<float>&, unsigned module, const math::mat3<float>&) const;
    bool GetFModuleRect(math::rect<float>&, unsigned frame, unsigned fmodule, const math::mat3<float>&) const;
    void GetFrameRect(math::rect<float>&, unsigned frame, const math::mat3<float>&) const;
    void GetAFrameRect(math::rect<float>&, unsigned anim, unsigned aframe, const math::mat3<float>&) const;
    void GetAnimRect(math::rect<float>&, unsigned anim, const math::mat3<float>&) const;
    int GetFrameLogicalRectsCount(int frame) const;
    int GetAFrameLogicalRectsCount(int anim, int aframe) const;
    bool GetAFrameLogicalRect(int anim, int aframe, int idx, math::rect<float>&) const;
    virtual bool GetFrameLogicalRect(int frame, int idx, math::vec2<float> &pos, math::vec2<float> &size) const;

    const jet::String &GetLoadedFilename() const { return m_spriteFileName; }
    const jet::String &GetImageName(unsigned i = 0) const;
    int FindModuleByName(const jet::String&) const;
    int FindFrameByName(const jet::String&) const;
    int FindAnimByName(const jet::String&) const;

protected:
    static jet::String ReadAuroraName(jet::stream::IStream&);

    std::vector<ModuleInfo> m_moduleInfo;
    std::vector<unsigned short> m_framesNumberOfFModules;
    std::vector<unsigned short> m_framesFMStart;
    std::vector<FModuleData> m_fmodules;
    std::vector<unsigned> m_animsTotalTime;
    std::vector<unsigned short> m_animsNumberAFrames;
    std::vector<unsigned short> m_animsFirstAFrameIndex;
    std::vector<AFrameData> m_aFrames;
    std::vector<FrameRectData> m_frameRects;
    std::vector<unsigned short> m_frameRectsStart;
    std::vector<unsigned char> m_frameRectsNum;
    std::vector<boost::shared_ptr<jet::video::Texture> > m_paletteTextures;
    std::vector<boost::shared_ptr<jet::video::Texture> > m_paletteTexturesAlpha;
    int m_currentPalette;
    jet::video::Color m_currentColor;
    jet::String m_spriteFileName;
    std::vector<jet::String> m_imagesName;
    std::vector<math::vec2<float> > m_imagesInvSize;
    std::map<jet::String, int> m_moduleNames, m_frameNames, m_animNames;
};

#endif

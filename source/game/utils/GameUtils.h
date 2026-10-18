// source/game/utils/GameUtils.h
#ifndef SOURCE_GAME_UTILS_GAMEUTILS_H
#define SOURCE_GAME_UTILS_GAMEUTILS_H

#include "smath.h"
#include "video/color.h"
#include "video/painter.h"
#include <vector>

namespace jet::scene { class Model; class SceneMgr; class Camera; }
namespace jet::video { class Material; }

// One uniform of one pass of a model material (ShaderUniformRef)
struct ShaderUniformRef {
    jet::video::Material *material;
    unsigned pass;
    int uniform;
};
// Every uniform called name of every pass of every material of the model, appended (@0x3bbfa8)
void GetUniformsByName(jet::scene::Model*, const jet::String &name, std::vector<ShaderUniformRef> &out);

class ControlledRandom;

class GameUtils {  // sizeof 0x1
public:
    // count picks of the indices of weights, each weight spread over the rest once picked, so every index comes in turn (@0x3bd84c)
    static std::vector<unsigned> IndexDistributedRandom(std::vector<float> &weights, unsigned count, ControlledRandom *rnd);
    // the same for count equal weights (@0x3bda10)
    static std::vector<unsigned> IndexDistributedRandom(unsigned weightCount, unsigned count, ControlledRandom *rnd);
    // A screen rectangle of one colour; the world variant draws in world coordinates
    static void DrawRect(jet::video::Painter&, const math::rect<float>&, const jet::video::Color&);
    static void DrawRectWorld(jet::video::Painter&, const math::rect<float>&, const jet::video::Color&);
    // The screen rectangle (pixels, 50 more all around) a world box covers (@0x3bd670)
    static void GetProjectedAABB(const jet::scene::Camera&, const math::aabb<float>&, math::rect<float>&);
    // Clear the depth of a screen rectangle, of the whole target, or of what a scene manager's tag will draw
    static void ClearDepthBuffer(const math::rect<float>&);
    static void ClearDepthBuffer();
    static void ClearDepthBuffer(jet::scene::SceneMgr*, unsigned tag);
    // Draw a scene manager's tag over what is on screen: its depth area cleared first, the shadow tag dropped
    // unless the shadow map is rendered (@0x3bd0b4)
    static void MenuRenderScene(jet::scene::SceneMgr*, unsigned tag, bool clearAllDepth, bool renderShadowMap);
    // "QUICK_TIP_01" and 2 give "QUICK_TIP_03": the number after the last "_" moves on, keeping its width (@0x3be2ac)
    static jet::String IncreaseKey(const jet::String&, int);
    // How many texts key, key + 1, ... exist (@0x3be4e8)
    static int ElementsCount(const jet::String &firstKey);
    // The language's index for the platform's tracking (@0x3be56c)
    static int GetLoadedLanguageAndroidIndex();
private:
    static void FillRect(jet::video::Painter&, const math::rect<float>&, const jet::video::Color&, const char *technique);
};

#endif

// source/game/DebugRenderer.h
#ifndef SOURCE_GAME_DEBUGRENDERER_H
#define SOURCE_GAME_DEBUGRENDERER_H

class DebugRenderer {  // sizeof 0x1
    static void RenderFPS();
    static void RenderClock(FontMgr*, __int64, float, float);
public:
    static void SetOptions(const DebugRendererOptions&);
    static const DebugRendererOptions &GetOptions();
    static void Enable(bool);
    static bool IsEnabled();
    static void Render(const jet::scene::Camera&, jet::video::Painter&);
    static std::vector<jet::String,std::allocator<jet::String> > GetOptionList();
    static void ToggleOption(const jet::String&);
    static bool IsOptionEnabled(const jet::String&);
    static void LoadSettings();
    static void SaveSettings();
    static void ApplyOptions();
private:
    static void ApplyCollisionOptions();
    static void ApplyAIOptions();
    static DebugRendererOptions s_options;
    static bool s_isEnabled;
};

#endif

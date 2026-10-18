// source/game/interfaces/menus/MenuMgr.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENUMGR_H
#define SOURCE_GAME_INTERFACES_MENUS_MENUMGR_H

#include <map>
#include <vector>
#include "jsingleton.h"
#include "core/pstring.h"
#include "smath.h"

class InterfaceButton;
class Menu_Base;
class LightDef;
class CameraEntity;
namespace clara { class Movie; class Entity; }
namespace jet::scene { class Camera; }
namespace jet::video { class Painter; }

// Owns the menus (pages of GUI elements), the stack of the open ones, the menu camera and light, the menu
// movie and the buttons pressed this frame
class MenuMgr : public Singleton<MenuMgr> {  // sizeof 0xE4
public:
    struct SMenuAction { int type; jet::String menu; };  // 0 push, 1 pop
    MenuMgr();
    virtual ~MenuMgr();
    void Init();
    void Unload();
    void AddNewMenu(Menu_Base*);
    Menu_Base *GetMenu(const jet::String&);
    void Loading_AddExtraMenus();
    void OnMouseWheel(int);
    void Render(jet::video::Painter&);
    void Update();
    void PushMenu(const jet::String&);
    void PopMenu();
    void ClearStack();
    const jet::String &GetCurrentMenuName() const { return m_currentMenuName; }
    Menu_Base *GetCurrentPlayingMenu() { return m_menuStack.empty() ? 0 : m_menuStack.back(); }
    // the menu under the top one (0 when there are fewer than two): the .so reads the same entry in Menu_Pause::OnQuit
    Menu_Base *GetMenuBelowCurrent() { return m_menuStack.size() < 2 ? 0 : m_menuStack[m_menuStack.size() - 2]; }
    void RefreshMenus();
    void RefreshMenu(const jet::String&);
    void OnUnloadClaraData();
    void IgnoreNextTracking() { m_ignoreNextTracking = true; }
    void SetDefault3dLightFrom(LightDef*);

    void ACTION_Reset();
    void ACTION_AddReleasedButton(InterfaceButton*);
    int ACTION_GetNumReleasedButtons() { return (int)m_actionReleasedButtons.size(); }
    InterfaceButton *ACTION_GetReleasedButton(int i) { return m_actionReleasedButtons[i]; }
    void ACTION_AddPressedButton(InterfaceButton *b) { m_actionPressedButtons.push_back(b); }
    int ACTION_GetNumPressedButtons() { return (int)m_actionPressedButtons.size(); }
    int ACTION_GetNumPressedButtonsLastFrame() { return m_actionNumPressedButtonsLastFrame; }
    void DisableTouchesFor(int ms);
    void UpdateDisableTouchesTimer();
    void ResetDisableTouchesTimer() { m_disableTouchesTimer = 0; }
    void UpdateNoise(int dt);
    void Apply3DLayersTransform();

    void Movie_StartMovie(clara::Movie*);
    void Movie_StartMovie(const jet::String&);
    bool Movie_IsPlaying();
    void Movie_Stop();
    jet::scene::Camera *GetCamera() const { return m_camera; }
    LightDef *GetLight() const { return m_light; }
    void SetCameraDataForParticles(const jet::scene::Camera&);
    // the level's UI camera: the menu camera copies its controller's position, rotation and fov each frame
    void SetCameraEntity(CameraEntity *e) { m_cameraEntity = e; }
    static bool s_paintGUI;
private:
    void _MenuActions_SetActions();
    void _MenuActions_Push(const jet::String&);
    void _MenuActions_Pop();

    std::map<jet::String, Menu_Base*> m_menuArray;  // 0x8
    bool m_extraMenusLoaded;                        // 0x20
    std::vector<Menu_Base*> m_menuStack;            // 0x24
    int m_menuState;                                // 0x34: 0 idle, 1 a menu was just put on top, 2 actions pending
    jet::String m_currentMenuName;                  // 0x38
    std::vector<SMenuAction> m_menuActions;         // 0x3c
    bool m_hasActions;                              // 0x4
    std::vector<InterfaceButton*> m_actionReleasedButtons;  // 0x48
    std::vector<InterfaceButton*> m_actionPressedButtons;   // 0x58
    int m_actionNumPressedButtonsLastFrame;         // 0x68
    math::vec3<float> m_layer3DPos[3];              // 0x6C
    math::quat<float> m_layer3DRot[3];              // 0x90
    float m_layer3DRotationAnim;                    // 0xC0
    CameraEntity *m_cameraEntity;                   // 0xC8
    jet::scene::Camera *m_camera;                   // 0xCC
    LightDef *m_light;                              // 0xD0
    clara::Movie *m_movie;                          // 0xD4
    Menu_Base *m_previousMenu;                      // 0xD8
    bool m_ignoreNextTracking;                      // 0xDC
    int m_disableTouchesTimer;                      // 0xE0
};

#endif

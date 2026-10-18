// source/game/interfaces/interfaceobjects/InterfaceObject.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEOBJECT_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEOBJECT_H

#include <vector>
#include "core/pstring.h"
#include "smath.h"
#include "scene/camera.h"
#include "video/material.h"
#include "video/painter.h"

class AuroraLevelData;

// A textured quad placed in the 3D scene by the menus: center, the two edge vectors and the size it was authored with
class BodyPartOwner;

struct Interface3DLayer {  // sizeof 0x2C
    Interface3DLayer() { for (int i = 0; i < 11; i++) v[i] = 0; }
    math::vec3<float> &pos() { return *(math::vec3<float>*)&v[0]; }
    math::vec3<float> &edgeA() { return *(math::vec3<float>*)&v[3]; }
    math::vec3<float> &edgeB() { return *(math::vec3<float>*)&v[6]; }
    math::vec2<float> &size() { return *(math::vec2<float>*)&v[9]; }
    const math::vec3<float> &pos() const { return *(const math::vec3<float>*)&v[0]; }
    const math::vec3<float> &edgeA() const { return *(const math::vec3<float>*)&v[3]; }
    const math::vec3<float> &edgeB() const { return *(const math::vec3<float>*)&v[6]; }
    const math::vec2<float> &size() const { return *(const math::vec2<float>*)&v[9]; }
    float v[11];
};

// Base of every GUI element: a tree with position, offset, size, scale and a clip rectangle
class InterfaceObject {  // sizeof 0x1A4
public:
    virtual const jet::String &GetUITemplateName() = 0;
    InterfaceObject();
    virtual ~InterfaceObject();
    virtual void Init();
    virtual void Update(int dt);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RenderDebug(jet::video::Painter&);
    virtual void UnloadClaraData();
    virtual void UpdateChildren(int dt);
    virtual void RenderChildren(const jet::scene::Camera&, jet::video::Painter&);
    virtual void UpdateChildOffsets();
    virtual void SetPos(math::vec2<float>);
    virtual math::vec2<float> GetPos();
    virtual math::vec2<float> GetUnscaledPos();
    virtual math::vec2<float> GetCombinedPos();
    virtual void SetOffset(const math::vec2<float>&);
    virtual math::vec2<float> GetOffset();
    virtual const math::vec2<float> &GetOffsetNoParent();
    virtual math::vec2<float> GetOffsetForClip();
    virtual void SetSpriteArtOffset(const math::vec2<float>&);
    virtual math::vec2<float> GetSpriteArtOffset();
    virtual void SetVisible(bool);
    virtual bool IsVisible();
    virtual void SetActive(bool);
    virtual bool IsActive();
    virtual bool IsScreenPosVisible(math::vec2<float>);
    virtual void SetSize(const math::vec2<float>&);
    virtual math::vec2<float> GetSize();
    virtual const math::vec2<float> &GetUnscaledSize();
    virtual void Reset();
    virtual void OnMenuQuit();
    virtual void SetRotation(float);
    virtual void SetUIScale(float);
    virtual math::vec2<float> GetUIScale();
    virtual void SetScale(float);
    virtual void SetScale(const math::vec2<float>&);
    virtual const math::vec2<float> &GetScale();
    virtual math::vec2<float> GetCombinedScale();
    virtual void SetScaleForChildren(const math::vec2<float>&);
    virtual void AddChild(InterfaceObject*);
    virtual void RemoveChild(InterfaceObject*);
    virtual math::rect<float> GetClip();
    virtual void _Debug_Render(jet::video::Painter&);
    virtual unsigned _Debug_RenderColor();

    static int GetPaletteColor(int);
    static int GetPaletteOutlineColor(int);
    static void LoadPalettesValues(const jet::String&);
    void SetParent(InterfaceObject*);
    int GetLayerIdx() const { return m_layerIdx; }
    int GetGuiParentId() const { return m_guiParentId; }
    InterfaceObject *GetChild(int i) { return m_childObjects[i]; }
    unsigned GetChildCount() { return (unsigned)m_childObjects.size(); }
    int GetChildIdx(InterfaceObject*);
    void SetClip(math::rect<float> r) { m_clip = r; }
    bool HasClip();
    void DisableClip() { m_clip = math::rect<float>(0, 0, 0, 0); }
    static void Get3DLayerTransfrom(const Interface3DLayer&, math::vec3<float>& pos, math::quat<float>& rot, math::vec3<float>& scale, math::vec2<float>& size);
    static bool Update3DLayerTransform(Interface3DLayer&, BodyPartOwner*, unsigned bodyPartIdx, const math::vec2<float>& size);
    static bool Update3DLayerTransform(Interface3DLayer&, const math::vec3<float>& pos, const math::quat<float>& rot, const math::vec3<float>& scale, const math::vec2<float>& size);
    static bool Update3DLayerTransform(Interface3DLayer&, const math::vec3<float>& center, const math::vec3<float>& edgeA, const math::vec3<float>& edgeB, const math::vec2<float>& size);
    static math::rect<float> GetTransformedRect(const jet::scene::Camera&, const math::mat4<float>&, const math::rect<float>&);
    static void SetCustomI3DLayerData(int i, const Interface3DLayer &l) { s_customI3DLayer[i] = l; }
    static const Interface3DLayer &GetCustomI3DLayer(int i) { return s_customI3DLayer[i]; }
    int GetGUIParentId() { return m_guiParentId; }
    int GetGUILayerIdx() { return m_guiId; }
    void SetDebugName(const jet::String &n) { m_debugName = n; }
    const jet::String &GetDebugName() { return m_debugName; }
    static math::vec3<float> MenuModel_Get3DPos(const jet::scene::Camera&, const math::vec2<float>&);
    static math::vec3<float> MenuModel_Get3DPos(const jet::scene::Camera&, const math::vec2<float>&, const math::vec3<float>& planePoint);
    static math::mat3<float> MenuModel_FaceCameraPlane(const jet::scene::Camera&, const math::vec3<float>&);
    static void UpdateAutoScrollText();
    static void Set3DAspectRatioScale(float s) { s_3DAspectRatioScale = s; }
    static float Get3DAspectRatioScale() { return s_3DAspectRatioScale; }
protected:
    void SetScaleDirect(const math::vec2<float> &s) { m_scale = s; }
    void PushI3DTransform(const Interface3DLayer&);
    void PopI3DTransform();
    static const int k_textAutoScroll_pauseTime = 3000;
    static const int k_textAutoScroll_scrollTime = 3000;
    static const int k_textAutoScroll_totalTime = 6000;
    static std::vector<int> s_palettesColor;
    static std::vector<int> s_palettesOutlineColor;
    static float s_autoScrollText_animScale;
    bool m_visible;  // 0x4
    bool m_active;  // 0x5
    InterfaceObject *m_parent;  // 0x8
    std::vector<InterfaceObject*> m_childObjects;  // 0xC
    int m_layerIdx;  // 0x18
    math::rect<float> m_clip;  // 0x1C
    float m_rotation;  // 0x2C
    math::mat4<float> m_oldPainterTransform;  // 0x30
    jet::video::Material m_oldMaterial;  // 0x70
    bool m_oldText2DWorldCoords;  // 0x160
    jet::String m_debugName;  // 0x164
    AuroraLevelData *m_guiLevel;  // 0x168
    int m_guiId;  // 0x16C
    int m_guiParentId;  // 0x170
    static bool s_debugEnabled;
private:
    void _Apply3dTransform(const math::vec3<float>& center, const math::vec3<float>& edgeA, const math::vec3<float>& edgeB, const math::vec2<float>& size);
    math::vec2<float> m_pos;  // 0x174
    math::vec2<float> m_offset;  // 0x17C
    math::vec2<float> m_size;  // 0x184
    math::vec2<float> m_spriteArtOffset;  // 0x18C
    math::vec2<float> m_scale;  // 0x194
    math::vec2<float> m_UIScale;  // 0x19C
    static float s_3DAspectRatioScale;
    static std::vector<Interface3DLayer> s_customI3DLayer;
    static std::vector<Interface3DLayer> s_renderI3DLayerStack;
};

#endif

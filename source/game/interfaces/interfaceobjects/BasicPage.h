// source/game/interfaces/interfaceobjects/BasicPage.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_BASICPAGE_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_BASICPAGE_H

#include "InterfaceObject.h"
#include <vector>

class AuroraLevelData;
class InterfaceButton;
class InterfaceGraph;
class InterfaceText;
class InterfaceArea;
class InterfaceList;
class InterfaceListButtonScroll;
class InterfaceGrid;
class InterfacePBar;
class InterfaceScrollBar;
class Interface3DElement;
class Interface3DBodyPart;
class Interface3DCostumePos;

// The widgets of one Aurora GUI layer, created from its elements and parented as the layer says
class BasicPage : public InterfaceObject {  // sizeof 0x1BC
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    BasicPage(const jet::String &layerName);
    virtual ~BasicPage();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void GenerateFromAuroraFile(AuroraLevelData*);
    virtual void UnloadClaraData();
    InterfaceButton *ManualAddButton(InterfaceObject *parent, math::vec2<float> pos, int sprite, int idleAnim, int pressedAnim, int selectedAnim, const jet::String &text, int font,
                                     int palIdle, int palPressed, int palSelected, int iconPos, int iconIdleAnim, int iconPressedAnim, int iconSelectedAnim, int vAlign, int hAlign,
                                     int textOffset, bool wrap, bool scaleToWidth, int shadow, int outline, int strikeout, int underline, bool clipText);
    InterfaceList *ManualAddList(InterfaceObject *parent, math::vec2<float> pos, math::vec2<float> size, int orientation, int hAlign, int vAlign, int spacing, bool enableClip, bool enableTouch);
    InterfaceText *ManualAddLayerText(InterfaceObject *parent, math::vec2<float> pos, int w, int h, int font, const jet::String &text, int vAlign, int hAlign, int palette, bool rotation);
    Interface3DElement *ManualAddI3DElement(InterfaceObject *parent, math::vec2<float> pos, float scale, const jet::String &claraNode, bool renderAtScreenPos);
    InterfacePBar *ManualAddPBar(InterfaceObject *parent, math::vec2<float> pos, int type, int sprite, int bgAnim, int barAnim, int extra1Anim, int extra2Anim, int limitAnim, int logicRectIdx);
    InterfaceButton *GetUIButton(const jet::String &n) { return (InterfaceButton*)GetUIObject(n); }
    InterfaceGraph *GetUIGraph(const jet::String &n) { return (InterfaceGraph*)GetUIObject(n); }
    InterfaceText *GetUIText(const jet::String &n) { return (InterfaceText*)GetUIObject(n); }
    InterfaceArea *GetUIArea(const jet::String &n) { return (InterfaceArea*)GetUIObject(n); }
    Interface3DBodyPart *GetUI3DBodyPart(const jet::String &n) { return (Interface3DBodyPart*)GetUIObject(n); }
    InterfaceList *GetUIList(const jet::String &n) { return (InterfaceList*)GetUIObject(n); }
    InterfaceListButtonScroll *GetUIListButtonScroll(const jet::String &n) { return (InterfaceListButtonScroll*)GetUIObject(n); }
    InterfaceGrid *GetUIGrid(const jet::String &n) { return (InterfaceGrid*)GetUIObject(n); }
    InterfacePBar *GetUIPBar(const jet::String &n) { return (InterfacePBar*)GetUIObject(n); }
    InterfaceScrollBar *GetUIScrollBar(const jet::String &n) { return (InterfaceScrollBar*)GetUIObject(n); }
    Interface3DCostumePos *GetUI3DCostumePos(const jet::String &n) { return (Interface3DCostumePos*)GetUIObject(n); }
    // bounds-checked: an element type not yet ported (BasicPage::GenerateFromAuroraFile's case 14,
    // 3DCostumePos) never gets added, so its slot can be out of the array's current bounds entirely
    // (not just null) when it's the highest-indexed element skipped in a layer. In the original,
    // fully-ported engine every index 0..n-1 always has a real object, so this never came up; found
    // via a real crash (SendChallenge_Taunt, which has a 3DCostumePos element).
    InterfaceObject *GetUIObject(int i) { return i >= 0 && (size_t)i < m_elementsArray.size() ? m_elementsArray[i] : 0; }
    InterfaceObject *GetUIObject(const jet::String&);
    const jet::String &GetUIObjectName(unsigned i);
    unsigned GetUIObjectCount() { return (unsigned)m_elementsArray.size(); }
    void RemoveObject(InterfaceObject*);
    void ClearManualObjects();
    virtual void Reset();
    virtual void OnMenuQuit();
    static void OnUnloadClaraData();
    static const jet::String k_uiTmplName;
private:
    void AddInterfaceObj(InterfaceObject*, int);
    void AutoSetGUIParent(InterfaceObject*);
    InterfaceObject *GetInterfaceObjFromLayerIdx(int);
    unsigned m_numAuroraElements;  // 0x1A4
    std::vector<InterfaceObject*> m_elementsArray;  // 0x1A8
    AuroraLevelData *m_auroraFile;  // 0x1B8
    static std::vector<BasicPage*> s_basicPagesInstances;
};

#endif

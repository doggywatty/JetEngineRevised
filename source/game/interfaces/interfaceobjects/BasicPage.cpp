// source/game/interfaces/interfaceobjects/BasicPage.cpp
#include "BasicPage.h"
#include "Interface3DBodyPart.h"
#include "Interface3DElement.h"
#include "Interface3DTouchInteract.h"
#include "Interface3DCostumePos.h"
#include "InterfaceArea.h"
#include "InterfaceButton.h"
#include "InterfaceDeviceSpecificAnim.h"
#include "InterfaceFillRect.h"
#include "InterfaceGraph.h"
#include "InterfaceGrid.h"
#include "InterfaceList.h"
#include "InterfaceListButtonScroll.h"
#include "InterfacePBar.h"
#include "InterfaceScrollBar.h"
#include "InterfaceSlide.h"
#include "InterfaceText.h"
#include "game/interfaces/aurora/AuroraLevelData.h"
#include "game/interfaces/aurora/AuroraLevelMgr.h"
#include <algorithm>
#include <cstdio>

const jet::String BasicPage::k_uiTmplName("BasicPage");
std::vector<BasicPage*> BasicPage::s_basicPagesInstances;

// @0x446cd4
BasicPage::BasicPage(const jet::String &layerName) : m_numAuroraElements(0), m_auroraFile(0) {
    s_basicPagesInstances.push_back(this);
    if (!layerName.empty()) GenerateFromAuroraFile(AuroraLevelMgr::GetInstance()->GetLevel(layerName));
}

// @0x446de0
BasicPage::~BasicPage() {
    for (size_t i = 0; i < m_elementsArray.size(); i++) delete m_elementsArray[i];
    s_basicPagesInstances.erase(std::remove(s_basicPagesInstances.begin(), s_basicPagesInstances.end(), this), s_basicPagesInstances.end());
}

// @0x446ed4: the slot is replaced, or the array grows to hold it
void BasicPage::AddInterfaceObj(InterfaceObject *obj, int idx) {
    if (idx < (int)m_elementsArray.size()) {
        delete m_elementsArray[idx];
        m_elementsArray[idx] = 0;
    } else {
        m_elementsArray.resize(idx + 1, (InterfaceObject*)0);
    }
    m_elementsArray[idx] = obj;
}

// @0x446f78
void BasicPage::AutoSetGUIParent(InterfaceObject *obj) {
    obj->SetParent(obj->GetGuiParentId() >= 0 ? GetInterfaceObjFromLayerIdx(obj->GetGuiParentId()) : this);
}

// @0x446fb0: one widget per element by its type, then the parents, then the lists' drag button and
// background and the grids' scroll bar
void BasicPage::GenerateFromAuroraFile(AuroraLevelData *level) {
    m_auroraFile = level;
    int n = (int)level->GetElementsCount();
    m_numAuroraElements = n;
    for (int i = 0; i < n; i++) {
        InterfaceObject *o = 0;
        switch (level->GetParamValue(i, 0)) {
            case 0: o = new InterfaceGraph(level, i); break;
            case 1: o = new InterfaceArea(level, i); break;
            case 2: o = new InterfaceFillRect(level, i); break;
            case 3: o = new InterfaceButton(level, i); break;
            case 4: o = new InterfaceText(level, i); break;
            case 5: o = new InterfaceList(level, i); break;
            case 6: o = new InterfaceListButtonScroll(level, i); break;
            case 7: o = new InterfaceSlide(level, i); break;
            case 8: o = new InterfacePBar(level, i); break;
            case 9: o = new InterfaceGrid(level, i); break;
            case 10: o = new InterfaceDeviceSpecificAnim(level, i); break;
            case 11: o = new InterfaceScrollBar(level, i); break;
            case 12: o = new Interface3DElement(level, i); break;
            case 13: o = new Interface3DBodyPart(level, i); break;
            case 14: o = new Interface3DCostumePos(level, i); break;
            case 15: o = new Interface3DTouchInteract(level, i); break;
            default: continue;
        }
        o->Init();
        AddInterfaceObj(o, i);
    }
    // an element type not yet ported (case 14, 3DCostumePos) leaves its slot null -- skip it rather
    // than dereference null in AutoSetGUIParent. Found via a real crash: any Aurora page with a
    // 3DCostumePos element (e.g. SendChallenge_Taunt) segfaulted here before this guard.
    for (int i = 0; i < n; i++) if (InterfaceObject *o = GetUIObject(i)) AutoSetGUIParent(o);
    for (int i = 0; i < n; i++) {
        InterfaceObject *o = GetUIObject(i);
        int type = level->GetParamValue(i, 0);
        if (type == 5) {
            InterfaceList *list = static_cast<InterfaceList*>(o);
            if (InterfaceObject *drag = GetInterfaceObjFromLayerIdx(level->GetParamValue(i, 18))) list->SetDragButton(static_cast<InterfaceButton*>(drag));
            list->SetBGObject(GetInterfaceObjFromLayerIdx(level->GetParamValue(i, 19)));
        } else if (type == 9) {
            InterfaceGrid *grid = static_cast<InterfaceGrid*>(o);
            if (grid->GetScrollBarLayerIdx() >= 0) grid->SetScrollBar(GetInterfaceObjFromLayerIdx(grid->GetScrollBarLayerIdx()));
        }
    }
}

// @0x447340
void BasicPage::Update(int dt) {
    if (!IsVisible() || !IsActive()) return;
    UpdateChildren(dt);
}

// @0x447390
void BasicPage::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    InterfaceObject::Render(cam, painter);
    if (!IsVisible()) return;
    RenderChildren(cam, painter);
}

// The manual widgets are added after the Aurora ones and parented to the page unless told otherwise
#define MANUAL_ADD(obj, parent) \
    AddInterfaceObj(obj, (int)m_elementsArray.size()); \
    obj->SetParent((parent) ? (parent) : this); \
    return obj

// @0x4474a4
InterfaceButton *BasicPage::ManualAddButton(InterfaceObject *parent, math::vec2<float> pos, int sprite, int idleAnim, int pressedAnim, int selectedAnim, const jet::String &text,
                                            int font, int palIdle, int palPressed, int palSelected, int iconPos, int iconIdleAnim, int iconPressedAnim, int iconSelectedAnim,
                                            int vAlign, int hAlign, int textOffset, bool wrap, bool scaleToWidth, int shadow, int outline, int strikeout, int underline,
                                            bool clipText) {
    InterfaceButton *o = new InterfaceButton();
    o->Init(pos, sprite, idleAnim, pressedAnim, selectedAnim, text, font, palIdle, palPressed, palSelected, iconPos, iconIdleAnim, iconPressedAnim, iconSelectedAnim, vAlign, hAlign,
            textOffset, wrap, scaleToWidth, shadow, outline, strikeout, underline, clipText);
    MANUAL_ADD(o, parent);
}

// @0x44750c
InterfaceList *BasicPage::ManualAddList(InterfaceObject *parent, math::vec2<float> pos, math::vec2<float> size, int orientation, int hAlign, int vAlign, int spacing, bool enableClip,
                                        bool enableTouch) {
    InterfaceList *o = new InterfaceList();
    o->Init(pos, size, orientation, hAlign, vAlign, spacing, enableClip, enableTouch);
    MANUAL_ADD(o, parent);
}

// @0x4475d0
InterfaceText *BasicPage::ManualAddLayerText(InterfaceObject *parent, math::vec2<float> pos, int w, int h, int font, const jet::String &text, int vAlign, int hAlign, int palette,
                                             bool rotation) {
    InterfaceText *o = new InterfaceText();
    o->Init(pos, w, h, font, text, vAlign, hAlign, palette, (float)rotation, 0, 0, 0, 0, false, false, true);
    MANUAL_ADD(o, parent);
}

// @0x4476b8
Interface3DElement *BasicPage::ManualAddI3DElement(InterfaceObject *parent, math::vec2<float> pos, float scale, const jet::String &claraNode, bool renderAtScreenPos) {
    Interface3DElement *o = new Interface3DElement();
    o->Init(pos, scale, claraNode, renderAtScreenPos);
    MANUAL_ADD(o, parent);
}

// @0x447748
InterfacePBar *BasicPage::ManualAddPBar(InterfaceObject *parent, math::vec2<float> pos, int type, int sprite, int bgAnim, int barAnim, int extra1Anim, int extra2Anim, int limitAnim,
                                        int logicRectIdx) {
    InterfacePBar *o = new InterfacePBar();
    o->Init(type, pos, sprite, bgAnim, barAnim, extra1Anim, extra2Anim, limitAnim, logicRectIdx);
    MANUAL_ADD(o, parent);
}

// @0x447804

// @0x4478a8: the element is found by its name in the layer
InterfaceObject *BasicPage::GetUIObject(const jet::String &name) {
    int i = m_auroraFile->GetElementIndex(name);
    return i < 0 ? 0 : GetUIObject(i);
}

const jet::String &BasicPage::GetUIObjectName(unsigned i) { return m_auroraFile->GetElementName(i); }

// @0x447938
InterfaceObject *BasicPage::GetInterfaceObjFromLayerIdx(int layerIdx) {
    if (layerIdx < 0) return 0;
    for (size_t i = 0; i < m_elementsArray.size(); i++)
        if (m_elementsArray[i] && m_elementsArray[i]->GetLayerIdx() == layerIdx) return m_elementsArray[i];
    return 0;
}

// @0x447990
void BasicPage::Reset() {
    // null slots are elements of a type not yet ported (see GenerateFromAuroraFile's case 14); see
    // the comment on GetUIObject(int) for why these can exist.
    for (size_t i = 0; i < m_elementsArray.size(); i++) if (m_elementsArray[i]) m_elementsArray[i]->Reset();
}

// @0x4479c0
void BasicPage::OnMenuQuit() {
    for (size_t i = 0; i < m_elementsArray.size(); i++) if (m_elementsArray[i]) m_elementsArray[i]->OnMenuQuit();
}

// @0x4479f0: only the manual objects leave the array
void BasicPage::RemoveObject(InterfaceObject *obj) {
    size_t i = 0;
    while (i < m_elementsArray.size() && m_elementsArray[i] != obj) i++;
    if (i == m_elementsArray.size()) return;
    delete obj;
    m_elementsArray[i] = 0;
    if (i >= m_numAuroraElements) m_elementsArray.erase(m_elementsArray.begin() + i);
}

// @0x447a74
void BasicPage::ClearManualObjects() {
    for (size_t i = m_numAuroraElements; i < m_elementsArray.size(); i++) delete m_elementsArray[i];
    m_elementsArray.resize(m_numAuroraElements);
}

// @0x447ae4
void BasicPage::UnloadClaraData() {
    for (size_t i = 0; i < m_elementsArray.size(); i++) if (m_elementsArray[i]) m_elementsArray[i]->UnloadClaraData();
}

// @0x447b20
void BasicPage::OnUnloadClaraData() {
    for (size_t i = 0; i < s_basicPagesInstances.size(); i++) s_basicPagesInstances[i]->UnloadClaraData();
}

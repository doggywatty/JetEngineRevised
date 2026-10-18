// source/game/interfaces/interfaceobjects/InterfaceList.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACELIST_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACELIST_H

#include "InterfaceObject.h"

class InterfaceButton;
class Menu_Shop;
class Menu_Options;
class Menu_Goals;

// Lays its children out in a row or a column and scrolls them by dragging; a drag button can fold the list open
class InterfaceList : public InterfaceObject {  // sizeof 0x1F0
    friend class Menu_Options;  // reads the touch scrolling and the drag for its scroll bar
    friend class Menu_Goals;
    friend class Menu_Shop;  // keeps and restores the scroll target, turns the touch scrolling off and asks if it is dragged
public:
    enum Orientation { Horizontal = 0, Vertical = 1 };
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceList();
    InterfaceList(AuroraLevelData *level, int id);
    virtual ~InterfaceList();
    void Init(math::vec2<float> pos, math::vec2<float> size, int orientation, int hAlign, int vAlign, int spacing, bool enableClip, bool enableTouch);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset();
    virtual void UpdateChildOffsets();
    virtual math::vec2<float> GetOffset();
    virtual math::vec2<float> GetOffsetForClip();
    InterfaceButton *GetDragButton() const { return m_dragButton; }
    void SetDragButton(InterfaceButton *b) { m_dragButton = b; }
    void SetBGObject(InterfaceObject*);
    float GetTotalChildrenWidth() const;
    float GetTotalChildrenHeight() const;
    void ResetScrollOffset();
    void CloseIfDragListOpen();
    bool IsDragListOpen() const { return m_dragListOpen; }
    bool IsDragButtonWasPressed() const { return m_dragButtonWasPressed; }
    static const jet::String k_uiTmplName;
protected:
    void UpdateTouches();
    void UpdateScrollOffsetTarget(float width, float height);
    int m_orientation;  // 0x1A8
    bool m_enableTouch;  // 0x1AC
    int m_hAlign;  // 0x1B0
    int m_vAlign;  // 0x1B4
    float m_spacing;  // 0x1B8
    math::vec2<float> m_scrollOffset;  // 0x1BC
    math::vec2<float> m_scrollTarget;  // 0x1C4
    float m_velocity;  // 0x1CC
    InterfaceButton *m_dragButton;  // 0x1D0
    bool m_dragButtonWasPressed;  // 0x1D4
    bool m_dragListOpen;  // 0x1D5
    math::rect<float> m_inputRect;  // 0x1D8
    bool m_flick;  // 0x1E8: releasing a drag keeps its momentum
    bool m_isDragging;  // 0x1E9
    InterfaceObject *m_bgObject;  // 0x1EC
};

#endif

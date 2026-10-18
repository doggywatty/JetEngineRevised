// source/game/interfaces/interfaceobjects/InterfaceGrid.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEGRID_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACEGRID_H

#include "InterfaceObject.h"

// Children in rows and columns of the size of the first one, scrolled one row (or column) at a time
class InterfaceGrid : public InterfaceObject {  // sizeof 0x208
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceGrid();
    InterfaceGrid(AuroraLevelData *level, int id);
    virtual ~InterfaceGrid();
    void Init(math::vec2<float> pos, math::vec2<float> size, int orientation, int hAlign, int vAlign, int rows, int columns, int spacing, bool enableClip);
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Reset();
    virtual void UpdateChildOffsets();
    virtual math::vec2<float> GetOffset();
    virtual math::vec2<float> GetOffsetForClip();
    virtual void UpdateChildren(int);
    virtual void RenderChildren(const jet::scene::Camera&, jet::video::Painter&);
    virtual void RemoveChild(InterfaceObject*);
    virtual void _Debug_Render(jet::video::Painter&) {}
    bool IsChidlVisible(int i);
    void Focus(int index, bool animate);
    int GetNumPages();
    bool IsDragging() const { return m_isDragging; }
    void SetRows(int r) { m_rows = r; }
    void SetRowCount(int n) { m_rows = n; }
    bool IsGridDragging() const { return m_isDragging; }
    void SetMainChild(InterfaceObject *c) { m_mainChild = c; }
    void SetScrollBar(InterfaceObject *b) { m_scrollBar = b; }
    int GetScrollBarLayerIdx() { return m_scrollBarLayerIdx; }
    static const jet::String k_uiTmplName;
private:
    void UpdateTouches();
    math::rect<float> ChildRect(InterfaceObject*);
    int m_orientation;  // 0x1A8: 0 pages of columns, 1 rows
    int m_hAlign;  // 0x1AC
    int m_vAlign;  // 0x1B0
    int m_rows;  // 0x1B4
    int m_columns;  // 0x1B8
    math::vec2<float> m_scrollOffset;  // 0x1BC
    math::vec2<float> m_scrollTarget;  // 0x1C4
    float m_spacing;  // 0x1CC
    float m_scrollPos;  // 0x1D0
    float m_scrollRange;  // 0x1D4
    bool m_dragging;  // 0x1D8
    bool m_isDragging;  // 0x1D9
    int m_page;  // 0x1DC
    int m_focusIndex;  // 0x1E4: -1 none
    math::rect<float> m_inputRect;  // 0x1E8
    InterfaceObject *m_mainChild;  // 0x1F8
    int m_scrollBarLayerIdx;  // 0x1FC
    InterfaceObject *m_scrollBar;  // 0x200
    bool m_enableTouch;  // 0x204
};

#endif

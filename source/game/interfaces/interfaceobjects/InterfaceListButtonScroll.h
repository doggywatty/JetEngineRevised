// source/game/interfaces/interfaceobjects/InterfaceListButtonScroll.h
#ifndef SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACELISTBUTTONSCROLL_H
#define SOURCE_GAME_INTERFACES_INTERFACEOBJECTS_INTERFACELISTBUTTONSCROLL_H

#include "InterfaceList.h"

// A list that is scrolled one child at a time with the up/down buttons
class InterfaceListButtonScroll : public InterfaceList {  // sizeof 0x200
public:
    virtual const jet::String &GetUITemplateName() { return k_uiTmplName; }
    InterfaceListButtonScroll();
    InterfaceListButtonScroll(AuroraLevelData *level, int id);
    virtual ~InterfaceListButtonScroll();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    unsigned CurrentElement() const;
    void ScrollUp();
    void ScrollDown();
    void ScrollAt(unsigned idx);
    bool IsScrolling() const;
    static const jet::String k_uiTmplName;
private:
    void CalculateScrollByClick();
    math::vec2<float> PositionOffsetAt(unsigned idx) const;
    unsigned m_count;  // 0x1F0
    float m_stepSize;  // 0x1F4
    float m_scrollRange;  // 0x1F8
    bool m_wrap;  // 0x1FC
    bool m_measured;  // 0x1FD
};

#endif

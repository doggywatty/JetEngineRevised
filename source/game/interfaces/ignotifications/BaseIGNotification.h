// source/game/interfaces/ignotifications/BaseIGNotification.h
#ifndef SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_BASEIGNOTIFICATION_H
#define SOURCE_GAME_INTERFACES_IGNOTIFICATIONS_BASEIGNOTIFICATION_H

class BaseIGNotification : public BasicPage {  // sizeof 0x1D4
public:
    BaseIGNotification(const jet::String&, int);
    virtual ~BaseIGNotification();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void Refresh();
    virtual void OnShow();
    virtual void OnHide();
    virtual float GetRenderedHeight();
protected:
    virtual void UpdateOffset();
public:
    static const int k_transitionTime;
    static const int k_defaultShowTime;
protected:
    bool m_refreshOnNextFrame;  // 0x1C4
    int m_timeToDisplayLeft;  // 0x1C8
    int m_totalTimeToDisplay;  // 0x1CC
    float m_height;  // 0x1D0
};

#endif

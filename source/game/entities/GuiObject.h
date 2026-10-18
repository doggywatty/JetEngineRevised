// source/game/entities/GuiObject.h
#ifndef SOURCE_GAME_ENTITIES_GUIOBJECT_H
#define SOURCE_GAME_ENTITIES_GUIOBJECT_H

class GuiObject : public Object, public InputMgr::TutorialInputListener {  // sizeof 0x108
public:
    GuiObject(const clara::Template*);
    virtual ~GuiObject();
    virtual void PostInit();
    virtual void Update();
    virtual void CollidesWith(Collision3d*, const CollisionPair&);
    void ShowGui();
    virtual void OnTutorialDoneCallback();
    void OnTimeSpeedDone();
    static jet::String k_tmplName;
private:
    unsigned m_portraitId;  // 0xE0
    jet::String m_textId;  // 0xE4
    int m_messageTime;  // 0xE8
    TutorialId m_id;  // 0xEC
    InputMgr::TutorialInputId m_tutorialInputId;  // 0xF0
    bool m_slowMotionActiveInputNone;  // 0xF4
    float m_slowMotionTimePgt;  // 0xF8
    int m_slowMotionTimeIn;  // 0xFC
    int m_slowMotionTimeOut;  // 0x100
    bool m_isColliding;  // 0x104
    bool m_wasColliding;  // 0x105
    static const jet::String k_portraitId;
    static const jet::String k_messageText;
    static const jet::String k_messageTime;
    static const jet::String k_tutorialId;
    static const jet::String k_tutorialInput;
    static const jet::String k_slowMotionActiveInputNone;
    static const jet::String k_slowMotionTimePgt;
    static const jet::String k_slowMotionTimeIn;
    static const jet::String k_slowMotionTimeOut;
};

#endif

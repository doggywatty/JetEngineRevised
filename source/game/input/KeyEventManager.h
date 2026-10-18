// source/game/input/KeyEventManager.h
#ifndef SOURCE_GAME_INPUT_KEYEVENTMANAGER_H
#define SOURCE_GAME_INPUT_KEYEVENTMANAGER_H

class KeyEventManager : public Singleton<KeyEventManager> {  // sizeof 0x1C
public:
    KeyEventManager();
    virtual ~KeyEventManager();
    void Update();
    KeyEvent *GetNextKeyEvent();
    void ClearKeyEvents();
    void OnKeyPress(int);
    void OnKeyRelease(int);
    void ResetKeyEventsPosition();
private:
    std::vector<KeyEvent,std::allocator<KeyEvent> > m_keyEvents;  // 0x4
    unsigned m_position;  // 0x18
};

#endif

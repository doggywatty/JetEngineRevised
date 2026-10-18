// source/game/interfaces/popups/PopupKeyBoard.h
#ifndef SOURCE_GAME_INTERFACES_POPUPS_POPUPKEYBOARD_H
#define SOURCE_GAME_INTERFACES_POPUPS_POPUPKEYBOARD_H

class PopupKeyBoard : public BasePopup {  // sizeof 0x228
public:
    PopupKeyBoard();
    virtual ~PopupKeyBoard();
    virtual void Update(int);
    static void PushPopup(boost::function<void __cdecl(int)>);
protected:
    void SetCallBack(boost::function<void __cdecl(int)>);
    void AddNumber(int);
    void RemoveNumber();
public:
    static const unsigned k_max_digits;
private:
    boost::function<void __cdecl(int)> m_callback;  // 0x1C8
    int m_result;  // 0x1E8
    unsigned m_digitCount;  // 0x1EC
    InterfaceButton *m_ok;  // 0x1F0
    InterfaceButton *m_exit;  // 0x1F4
    InterfaceButton *m_remove;  // 0x1F8
    InterfaceButton *m_numericButtons[10];  // 0x1FC
    InterfaceText *m_text;  // 0x224
};

#endif

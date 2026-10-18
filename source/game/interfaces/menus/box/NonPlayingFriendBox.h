// source/game/interfaces/menus/box/NonPlayingFriendBox.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_BOX_NONPLAYINGFRIENDBOX_H
#define SOURCE_GAME_INTERFACES_MENUS_BOX_NONPLAYINGFRIENDBOX_H

class NonPlayingFriendBox : public BasicPage {  // sizeof 0x200
public:
    NonPlayingFriendBox(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    virtual ~NonPlayingFriendBox();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    void SetState(int);
    int GetState();
protected:
    void Refresh();
    int m_state;  // 0x1C4
    InterfaceText *m_name;  // 0x1C8
    InterfaceButton *m_button;  // 0x1CC
    InterfaceObject *m_photoArea;  // 0x1D0
    InterfaceObject *m_invitedText;  // 0x1D4
    InterfaceObject *m_bgFriend;  // 0x1D8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_uid;  // 0x1DC
    bool m_invited;  // 0x1FC
};

#endif

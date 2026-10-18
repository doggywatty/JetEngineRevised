// source/game/interfaces/menus/Menu_InviteFriends.h
#ifndef SOURCE_GAME_INTERFACES_MENUS_MENU_INVITEFRIENDS_H
#define SOURCE_GAME_INTERFACES_MENUS_MENU_INVITEFRIENDS_H

#include <vector>
#include "Menu_Base.h"

class BasicPage;
class InterfaceButton;
class InterfaceGraph;
class InterfaceGrid;
class InterfaceText;

// The friends of the social network who do not play yet, each with a box to invite them, and a button to invite them all.
// The online services are not in this build, so the menu is ported with what they would answer left out: nobody is logged in
// (OnlinePlayerData), no friends are loading or loaded (FriendsMgr, NonPlayingFriendBox) and the invitation of all of them
// (FriendsMgr::InviteAllNonPlayingFriends) is not sent; the sounds and the tracking are not ported either.
class Menu_InviteFriends : public Menu_Base {  // sizeof 0x150
public:
    virtual const jet::String &GetMenuName() const { return k_menuName; }
    Menu_InviteFriends();
    virtual ~Menu_InviteFriends();
    virtual void Init();
    virtual void Update(int);
    virtual void Render(const jet::scene::Camera&, jet::video::Painter&);
    virtual void SetState(int);
    virtual void Refresh();
    virtual bool RenderGameInBG() { return false; }  // @0x47f4f8
    virtual bool ShowBanner() const { return true; }  // @0x47f500
    virtual int GetCurrentTrackingScreen() const { return -1; }  // @0x482604
    void OnBox(int) {}  // @0x482600
    static const jet::String k_menuName;
private:
    void ClearFriends();  // @0x4824ac
    void PopulateFriends();  // @0x48250c
    InterfaceGrid *m_friendsGrid;          // 0x124
    InterfaceButton *m_backButton;         // 0x128
    InterfaceGraph *m_loadingIcon;         // 0x12C
    InterfaceObject *m_bg;                 // 0x130
    InterfaceButton *m_inviteAllButton;    // 0x134
    InterfaceText *m_noMoreFriendsText;    // 0x138
    InterfaceObject *m_scrollBarFriends;   // 0x13C
    std::vector<BasicPage*> m_userBoxList; // 0x140: the boxes (NonPlayingFriendBox, not ported) of the friends
    bool m_isLoading;                      // 0x14C
};

#endif

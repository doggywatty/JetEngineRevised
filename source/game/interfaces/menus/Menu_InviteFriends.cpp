// source/game/interfaces/menus/Menu_InviteFriends.cpp
//
// The online services are not ported (see Menu_InviteFriends.h): the answers of OnlinePlayerData and FriendsMgr are those of an
// offline player. Not ported: the sounds, the invitation of all the friends (FriendsMgr::InviteAllNonPlayingFriends) and the
// boxes of the friends (NonPlayingFriendBox).
#include "Menu_InviteFriends.h"
#include "MenuMgr.h"
#include "game/Game.h"
#include "game/gamestates/GameState.h"
#include "game/interfaces/interfaceobjects/BasicPage.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceGraph.h"
#include "game/interfaces/interfaceobjects/InterfaceGrid.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"

const jet::String Menu_InviteFriends::k_menuName("InviteFriends");

// OnlinePlayerData: nobody is logged in to any network (IsLoggedInFacebook @0x3fe270, IsLoggedInGameCenter @0x3fe368,
// IsLoggedInGameAPI @0x401ca4, IsLoggedInQihoo @0x3fe2ec)
static bool IsLoggedInToAnyNetwork() { return false; }
// FriendsMgr: no friends are loading (@0x40), none was loaded (GetNumNonPlayingFriendsNotInvited @0x468a60)
static bool IsLoadingNonPlayingFriends() { return false; }
static int GetNumNonPlayingFriendsNotInvited() { return 0; }

// @0x481ba8
Menu_InviteFriends::Menu_InviteFriends()
    : Menu_Base(jet::String("InviteFriendsMenu")), m_friendsGrid(0), m_backButton(0), m_loadingIcon(0), m_bg(0), m_inviteAllButton(0), m_noMoreFriendsText(0),
      m_scrollBarFriends(0), m_isLoading(false) {}

// @0x481c94
Menu_InviteFriends::~Menu_InviteFriends() { ClearFriends(); }

// @0x481d0c
void Menu_InviteFriends::Init() {
    Menu_Base::Init();
    m_friendsGrid = GetUIGrid(jet::String("FriendsGrid"));
    m_bg = GetUIObject(jet::String("MenuBG"));
    m_backButton = GetUIButton(jet::String("BackButton"));
    m_loadingIcon = GetUIGraph(jet::String("LoadingIcon"));
    m_scrollBarFriends = GetUIObject(jet::String("InterfaceScrollBarFriends"));
    m_inviteAllButton = GetUIButton(jet::String("InviteAll"));
    m_noMoreFriendsText = GetUIText(jet::String("NoMoreFriendsText"));
}

// @0x482078: without the online services the menu goes away; the back key and the back button close it, the invite all button
// asks to invite everybody
void Menu_InviteFriends::Update(int dt) {
    MenuMgr *mm = MenuMgr::GetInstance();
    if (Game::GetInstance()->AreOnlineServicesReady()) {
        if (m_refreshOnNextFrame) Refresh();
        if (!GameState::GetCrtState()->IsBackKeyPressed()) {
            Menu_Base::Update(dt);
            int n = mm->ACTION_GetNumReleasedButtons();
            for (int i = 0; i < n; i++) {
                InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
                if (b == m_backButton) {
                    mm->PopMenu();
                    break;
                }
                if (b == m_inviteAllButton) {
                    // FriendsMgr::InviteAllNonPlayingFriends()
                    m_refreshOnNextFrame = true;
                    break;
                }
            }
            if (m_isLoading && !IsLoadingNonPlayingFriends()) Refresh();
            return;
        }
        // KeyEventManager::ClearKeyEvents()
    }
    mm->PopMenu();
}

// @0x4821f4: the background is drawn first, over what the menu scene shows
void Menu_InviteFriends::Render(const jet::scene::Camera &cam, jet::video::Painter &p) {
    m_bg->SetVisible(true);
    m_bg->Render(cam, p);
    m_bg->SetVisible(false);
    Menu_Base::Render(cam, p);
}

// @0x482258
void Menu_InviteFriends::SetState(int s) {
    Menu_Base::SetState(s);
    if (s == 1) Refresh();
}

// @0x482284: the friends are shown once they are loaded, with the button to invite them all or the text that there are none
void Menu_InviteFriends::Refresh() {
    bool loggedIn = IsLoggedInToAnyNetwork();
    m_isLoading = IsLoadingNonPlayingFriends();
    m_loadingIcon->SetVisible(m_isLoading);
    m_refreshOnNextFrame = false;
    ClearFriends();
    if (loggedIn && !m_isLoading) PopulateFriends();
    int notInvited = GetNumNonPlayingFriendsNotInvited();
    m_inviteAllButton->SetVisible(loggedIn ? (m_isLoading ? false : notInvited > 0) : false);
    GetUIObject(jet::String("NoMoreFriendsText"))->SetVisible(loggedIn ? (m_isLoading ? false : m_userBoxList.empty()) : false);
    m_noMoreFriendsText->SetVisible(m_isLoading ? false : notInvited <= 0);
    if (!m_isLoading && notInvited == 0) m_friendsGrid->SetVisible(false);
}

// @0x4824ac
void Menu_InviteFriends::ClearFriends() {
    for (size_t i = 0; i < m_userBoxList.size(); i++) {
        m_userBoxList[i]->SetParent(0);
        delete m_userBoxList[i];
    }
    m_userBoxList.clear();
}

// @0x48250c: a box for each friend who does not play, whether he was invited, in the grid
void Menu_InviteFriends::PopulateFriends() {
    int friends = GetNumNonPlayingFriendsNotInvited();  // the .so counts the list of FriendsMgr, which is empty here
    m_friendsGrid->SetRowCount(friends);
}

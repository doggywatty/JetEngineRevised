// source/game/online/users/OnlineUsersMgr.h
#ifndef SOURCE_GAME_ONLINE_USERS_ONLINEUSERSMGR_H
#define SOURCE_GAME_ONLINE_USERS_ONLINEUSERSMGR_H

class OnlineUsersMgr : public Singleton<OnlineUsersMgr> {  // sizeof 0x20
public:
    OnlineUsersMgr();
    virtual ~OnlineUsersMgr();
    void Init();
    void Update();
    OnlineUser *AddOnlineUser(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    OnlineUser *AddOnlineUser(social::User*);
    OnlineUser *GetOnlineUser(const jet::String&);
    OnlineUser *GetOnlineUser(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    void CleanOnlineUsers();
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,OnlineUser *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,OnlineUser *> > > m_onlineUsersMap;  // 0x4
};

#endif

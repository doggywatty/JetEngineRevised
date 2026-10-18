// libs/social_framework/include/social/multiloadable.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_MULTILOADABLE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_MULTILOADABLE_H

namespace social {
class MultiLoadable : public social::Loadable {  // sizeof 0xD8
public:
    MultiLoadable(social::User*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~MultiLoadable();
    void LoadKeys(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    bool AreLoaded(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    bool AreInErrorState(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    const social::OnlineStatus<enum social::Loadable::LoadState> &GetKeyStatus(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > &GetallKeys();
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
    void AddLoadable(social::Loadable*);
    void OnElementLoaded(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > &GetCurrentRequest();
    void QueueRequest(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    bool CheckKeys(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
private:
    void CheckRequests();
    static bool sOnLoaded(void*, bool, social::OnlineEventData);
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::Loadable *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::Loadable *> > > m_loadables;  // 0x8C
    std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_keys;  // 0xA8
    std::vector<std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,std::allocator<std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > m_loadRequests;  // 0xC4
};
}

#endif

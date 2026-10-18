// libs/social_framework/include/social/multistorable.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_MULTISTORABLE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_MULTISTORABLE_H

namespace social {
class MultiStorable : public social::MultiLoadable {  // sizeof 0x174
public:
    MultiStorable(social::User*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~MultiStorable();
    void SaveKeys(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    bool AreUptodate(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    bool AreInSaveErrorState(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    bool AreInSavingState(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    const social::OnlineStatus<enum social::Storable::SaveState> &GetSaveStatus(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > &GetallSaveKeys();
    void Save();
protected:
    void AddStorable(social::Storable*);
    void OnElementSaved(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    void QueueSaveRequest(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    bool CheckSaveKeys(const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > &GetCurrentSaveRequest();
private:
    void CheckSaveRequests();
    static bool sOnSaved(void*, bool, social::OnlineEventData);
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,social::Storable *,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,social::Storable *> > > m_storables;  // 0xD8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_key;  // 0xF4
    social::User *m_owner;  // 0x114
    std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_saveKeys;  // 0x118
    std::vector<std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >,std::allocator<std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > > m_saveRequests;  // 0x134
    social::OnlineStatus<enum social::Storable::SaveState> m_saveStatus;  // 0x148
};
}

#endif

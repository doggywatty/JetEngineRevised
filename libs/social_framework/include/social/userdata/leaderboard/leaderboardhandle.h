// libs/social_framework/include/social/userdata/leaderboard/leaderboardhandle.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_LEADERBOARD_LEADERBOARDHANDLE_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_LEADERBOARD_LEADERBOARDHANDLE_H

namespace social {
class LeaderboardEntry : public social::LeaderboardResource<social::LeaderboardEntry> {  // sizeof 0x8C
public:
    LeaderboardEntry();
    ~LeaderboardEntry();
    unsigned GetIndex();
    unsigned GetRank();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetCredential();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetDisplayName();
    unsigned GetScore();
    const social::CustomAttributes &GetCustomAttributes();
private:
    unsigned m_index;  // 0x14
    unsigned m_rank;  // 0x18
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_credential;  // 0x1C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_displayName;  // 0x3C
    unsigned m_score;  // 0x5C
    social::CustomAttributes m_customAttributes;  // 0x60
};
}

namespace social {
class LeaderboardEntryHandle : public social::leaderboard::Handle<social::LeaderboardEntryHandle,social::PointerStorage<social::LeaderboardEntry> > {  // sizeof 0xC
public:
    LeaderboardEntryHandle();
    LeaderboardEntryHandle(social::ID, social::PointerStorage<social::LeaderboardEntry>&);
    unsigned GetIndex();
    unsigned GetRank();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetCredential();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetDisplayName();
    unsigned GetScore();
    bool HasCustomAttributes();
    const social::CustomAttributes &GetCustomAttributes();
    bool operator==(const social::LeaderboardEntryHandle&);
private:
    static social::CustomAttributes m_customAttributes;
};
}

namespace social {
class LeaderboardRange : public social::Loadable, public social::LeaderboardResource<social::LeaderboardRange> {  // sizeof 0xEC
public:
    LeaderboardRange(social::Leaderboard&, social::leaderboard::E_LOAD_TYPE, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned);
    LeaderboardRange(social::Leaderboard&, social::leaderboard::E_LOAD_TYPE, unsigned);
    LeaderboardRange(social::Leaderboard&, social::leaderboard::E_LOAD_TYPE, unsigned, unsigned);
    virtual ~LeaderboardRange();
    bool IsEmpty();
    unsigned GetOffset();
    unsigned GetLimit();
    social::leaderboard::E_LOAD_TYPE GetType();
    unsigned GetEntryCount();
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
    unsigned GetPositionFromIndex(unsigned);
    social::LeaderboardEntry *GetEntryAtPos(unsigned);
    social::LeaderboardEntry *GetFirstEntry();
    social::LeaderboardEntry *GetEntryForUser(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static void sOnLoaded(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
private:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
    social::Leaderboard &m_leaderboard;  // 0xA0
    unsigned m_offset;  // 0xA4
    unsigned m_limit;  // 0xA8
    social::leaderboard::E_LOAD_TYPE m_type;  // 0xAC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_entryName;  // 0xB0
    social::LeaderboardEntry *m_firstEntry;  // 0xD0
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_rawServerData;  // 0xD4
    unsigned m_entryCount;  // 0xE8
};
}

namespace social {
class LeaderboardRangeHandle : public social::leaderboard::Handle<social::LeaderboardRangeHandle,social::PointerStorage<social::LeaderboardRange> > {  // sizeof 0xC
public:
    LeaderboardRangeHandle();
    LeaderboardRangeHandle(social::ID, social::PointerStorage<social::LeaderboardRange>&);
    ~LeaderboardRangeHandle();
    unsigned GetOffset();
    unsigned GetLimit();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetEntryName();
    social::leaderboard::E_LOAD_TYPE GetType();
    bool IsLoaded();
    social::OnlineStatus<enum social::Loadable::LoadState> GetLoadStatus();
    unsigned GetEntryCount();
    bool operator==(const social::LeaderboardRangeHandle&);
    social::LeaderboardEntryHandle GetEntryAtGlobalIndex(unsigned);
    social::LeaderboardEntryHandle GetEntryAtPos(unsigned);
    social::LeaderboardEntryHandle GetEntryForUser(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    social::LeaderboardEntryHandle GetFirstEntry();
};
}

#endif

// libs/social_framework/include/social/userdata/leaderboard/leaderboard.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_LEADERBOARD_LEADERBOARD_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_LEADERBOARD_LEADERBOARD_H

namespace social {
class FriendsLeaderboard {  // sizeof 0x190
public:
    FriendsLeaderboard(const social::Leaderboard::CreationSettings&);
    FriendsLeaderboard(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::Leaderboard::E_SORT_TYPE);
    ~FriendsLeaderboard();
    void Update();
    social::LeaderboardRangeHandle LoadRangeFromTop(unsigned, unsigned);
    social::LeaderboardRangeHandle LoadNextRange(social::LeaderboardRangeHandle&);
    social::LeaderboardRangeHandle LoadRangeMore(social::LeaderboardRangeHandle&, unsigned);
    void RefreshRange(social::LeaderboardRangeHandle&);
    void ReleaseRange(social::LeaderboardRangeHandle&);
    social::leaderboard::Error SendScore(const social::leaderboard::SendScoreParams&);
    social::leaderboard::Error SendScore(int);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetName();
    bool IsAscendent();
    bool IsGlobal();
    social::OnlineStatus<enum social::Storable::SaveState> GetSaveStatus();
private:
    social::Leaderboard m_leaderboard;  // 0x0
};
}

namespace social {
class IDBasedLeaderboardStorage : public social::ILeaderboardStorage {  // sizeof 0x34
public:
    IDBasedLeaderboardStorage();
    virtual ~IDBasedLeaderboardStorage();
    virtual social::LeaderboardEntry *NewLeaderboardEntry();
    virtual bool ReleaseLeaderboardEntry(social::LeaderboardEntry*&);
    virtual social::LeaderboardRange *NewTopLeaderboardRange(social::Leaderboard&, unsigned, unsigned);
    virtual social::LeaderboardRange *NewLeaderboardAroundPlayerRange(social::Leaderboard&, unsigned);
    virtual social::LeaderboardRange *NewLeaderboardAroundArbitraryEntryRange(social::Leaderboard&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned);
    virtual bool ReleaseLeaderboarRange(social::LeaderboardRange*&);
    virtual unsigned GetAllocatedRangeCount();
    virtual unsigned GetAllocatedEntryCount();
    virtual social::LeaderboardEntryHandle CreateEntryHandle(social::LeaderboardEntry&);
    virtual social::LeaderboardRangeHandle CreateRangeHandle(social::LeaderboardRange&);
private:
    social::PointerStorage<social::LeaderboardEntry> m_entriesStorage;  // 0x4
    social::PointerStorage<social::LeaderboardRange> m_rangesStorage;  // 0x1C
};
}

namespace social {
class Leaderboard {  // sizeof 0x190
protected:
    Leaderboard(const social::Leaderboard::CreationSettings&, social::Leaderboard::E_DOMAIN_TYPE);
    Leaderboard(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::Leaderboard::E_SORT_TYPE, social::Leaderboard::E_DOMAIN_TYPE);
public:
    Leaderboard(const social::Leaderboard::CreationSettings&);
    Leaderboard(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::Leaderboard::E_SORT_TYPE);
    ~Leaderboard();
    void Update();
    social::LeaderboardRangeHandle LoadRangeFromTop(unsigned, unsigned);
    social::LeaderboardRangeHandle LoadRangeAroundPlayer(unsigned);
    social::LeaderboardRangeHandle LoadRangeAroundArbitraryEntry(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned);
    social::LeaderboardRangeHandle LoadNextRange(social::LeaderboardRangeHandle&);
    social::LeaderboardRangeHandle LoadRangeMore(social::LeaderboardRangeHandle&, unsigned);
    void RefreshRange(social::LeaderboardRangeHandle&);
    void ReleaseRange(social::LeaderboardRangeHandle&);
    social::leaderboard::Error SendScore(const social::leaderboard::SendScoreParams&);
    social::leaderboard::Error SendScore(int);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetName();
    bool IsAscendent();
    bool IsGlobal();
    social::OnlineStatus<enum social::Storable::SaveState> GetSaveStatus();
    unsigned GetPosterRequestCount();
    unsigned GetRangeRequestCount();
    unsigned GetAllocatedRangeCount();
protected:
    bool ParseServerData(const std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&);
    void InsertRangeByIndex(social::LeaderboardRange&);
    bool SetRangeToDelete(social::LeaderboardRange&);
    bool ReleaseLeaderboardRange(social::LeaderboardRange*&);
    bool ReleaseLeaderboardEntry(social::LeaderboardEntry*&);
    bool ReleaseRangeEntries(social::LeaderboardRange&);
    void OnRangeLoaded(social::LeaderboardRange&, bool);
    bool PostNextScore();
    static social::LeaderboardRange *FindTopRangeInList(unsigned, unsigned, const std::list<social::LeaderboardRange *,std::allocator<social::LeaderboardRange *> >&);
    static social::LeaderboardRange *FindPlayerRangeInList(unsigned, const std::list<social::LeaderboardRange *,std::allocator<social::LeaderboardRange *> >&);
    static social::LeaderboardRange *FindArbitraryRangeInList(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned, const std::list<social::LeaderboardRange *,std::allocator<social::LeaderboardRange *> >&);
public:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_empty_string;
private:
    social::Leaderboard::E_SORT_TYPE m_sortType;  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_name;  // 0x4
    social::Leaderboard::E_DOMAIN_TYPE m_domainType;  // 0x24
    social::Leaderboard::LeaderboardData m_data;  // 0x28
    social::ILeaderboardStorage *m_storage;  // 0x6C
    social::request::SocialRequest::CreationSettings m_requestSettings;  // 0x70
    std::list<social::LeaderboardRange *,std::allocator<social::LeaderboardRange *> > m_ranges;  // 0xE4
    std::list<social::LeaderboardRange *,std::allocator<social::LeaderboardRange *> > m_rangesToDelete;  // 0xFC
    social::LeaderboardRange *m_firstRange;  // 0x114
    social::Leaderboard::LeaderboardRangeRequester m_requester;  // 0x118
    social::Leaderboard::LeaderboardScorePoster m_poster;  // 0x130
};
}

namespace social {
class LeaderboardDataParser {  // sizeof 0x1
public:
    static bool ParseLeaderboardData(social::Leaderboard&, const std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&);
    static bool ParseLeaderboardRangeData(social::LeaderboardRange&, const std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> >&);
    static bool ParseLeaderboardEntryData(social::LeaderboardEntry&, const Json::Value&);
private:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_id;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_created;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_total_entries;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_data;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_entry_credential;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_entry_display_name;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_entry_index;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_entry_rank;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key_entry_score;
};
}

namespace social::leaderboard {
struct SendScoreParams {  // sizeof 0xE0
    SendScoreParams(int, const social::request::SocialRequest::CreationSettings&);
    SendScoreParams(int);
    SendScoreParams(const social::leaderboard::SendScoreParams&);
    SendScoreParams();
    ~SendScoreParams();
    bool HasCustomAttributes();
    bool IsScriptRequest();
    social::CustomAttributes &GetCustomAttributes();
    social::leaderboard::SendScoreParams &operator=(const social::leaderboard::SendScoreParams&);
    bool operator==(const social::leaderboard::SendScoreParams&);
    bool operator!=(const social::leaderboard::SendScoreParams&);
    int m_score;  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_replaceIf;  // 0x4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_expirationDate;  // 0x24
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_expirationDuration;  // 0x44
    social::request::SocialRequest::CreationSettings m_requestSettings;  // 0x64
private:
    social::IntrusivePointer<social::CustomAttributes,social::IntrusivePointerNoLock> m_customAttributes;  // 0xD8
};
}

#endif

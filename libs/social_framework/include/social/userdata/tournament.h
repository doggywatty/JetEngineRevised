// libs/social_framework/include/social/userdata/tournament.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_TOURNAMENT_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_TOURNAMENT_H

namespace social {
class Tournament : public social::Storable {  // sizeof 0x12C
public:
    Tournament(social::Event*, const Json::Value&);
    Tournament(social::Event*);
    virtual ~Tournament();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetType();
    void SetType(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetLeaderboardId();
    void SetLeaderboardId(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const bool GetLeaderboardIsAscendent();
    void SetLeaderboardIsAscendent(bool);
    std::vector<social::TournamentAward,std::allocator<social::TournamentAward> > &GetAwards();
    void Parse(const Json::Value&);
    void AddAward(const social::TournamentAward&);
    void DeleteAward(const social::TournamentAward&);
protected:
    virtual void SaveImpl();
    void FinishSave();
    virtual void SaveableLoadImpl();
    virtual void SaveableUnLoadImpl();
    virtual void SaveableCancelLoadImpl();
private:
    static void sOnTournamentSet(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
protected:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    social::Event *m_owner;  // 0xBC
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_type;  // 0xC0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_leaderboardId;  // 0xE0
    bool m_leaderboardIsAscendent;  // 0x100
    std::vector<social::TournamentAward,std::allocator<social::TournamentAward> > m_awards;  // 0x104
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_tournamentJson;  // 0x118
};
}

#endif

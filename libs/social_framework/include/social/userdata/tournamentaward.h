// libs/social_framework/include/social/userdata/tournamentaward.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_TOURNAMENTAWARD_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_TOURNAMENTAWARD_H

namespace social {
class TournamentAward {  // sizeof 0x20
public:
    TournamentAward(const Json::Value&);
    TournamentAward();
    virtual ~TournamentAward();
    unsigned GetStartRank();
    unsigned GetEndRank();
    void SetRankInterval(unsigned, unsigned);
    std::vector<social::Gift,std::allocator<social::Gift> > &Gifts();
    void ComputeGiftsJson(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Parse(const Json::Value&);
private:
    unsigned m_startRank;  // 0x4
    unsigned m_endRank;  // 0x8
    std::vector<social::Gift,std::allocator<social::Gift> > m_gifts;  // 0xC
};
}

#endif

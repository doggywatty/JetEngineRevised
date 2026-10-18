// libs/social_framework/include/social/onlineeventdata.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_ONLINEEVENTDATA_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_ONLINEEVENTDATA_H

namespace social {
class OnlineEventData {  // sizeof 0x58
public:
    OnlineEventData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, social::OnlineEventData::ConflictData&);
    OnlineEventData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    OnlineEventData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    OnlineEventData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Set(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &Get(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual ~OnlineEventData();
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_attributes;  // 0x4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > key;  // 0x20
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > secondaryKeys;  // 0x40
    social::OnlineEventData::ConflictData conflictData;  // 0x54
};
}

#endif

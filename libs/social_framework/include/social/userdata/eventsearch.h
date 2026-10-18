// libs/social_framework/include/social/userdata/eventsearch.h
#ifndef LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_EVENTSEARCH_H
#define LIBS_SOCIAL_FRAMEWORK_INCLUDE_SOCIAL_USERDATA_EVENTSEARCH_H

namespace social {
class EventSearch : public social::Loadable {  // sizeof 0xFC
public:
    EventSearch();
    virtual ~EventSearch();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetEventCategory();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetEventStatus();
    const int GetLimit();
    const int GetOffset();
    const std::vector<social::Event *,std::allocator<social::Event *> > &GetEvents();
    void SearchEvents(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, int);
protected:
    virtual void LoadImpl();
    virtual void UnLoadImpl();
    virtual void CancelLoadImpl();
private:
    static void sOnEventsSearch(gaia::OpCodes, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, void*);
protected:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > k_key;
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_eventCategory;  // 0x8C
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_eventStatus;  // 0xAC
    int m_limit;  // 0xCC
    int m_offset;  // 0xD0
    std::vector<social::Event *,std::allocator<social::Event *> > m_events;  // 0xD4
    std::vector<gaia::BaseJSONServiceResponse,std::allocator<gaia::BaseJSONServiceResponse> > m_eventsJson;  // 0xE8
};
}

#endif

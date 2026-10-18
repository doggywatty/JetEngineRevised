// libs/glwt2/include/glwebtools/sse/glwebtools_serversideevent.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SSE_GLWEBTOOLS_SERVERSIDEEVENT_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SSE_GLWEBTOOLS_SERVERSIDEEVENT_H

namespace glwebtools {
class ServerSideEvent {  // sizeof 0x84
public:
    int SetEventName(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int AddData(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int SetLastEventId(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int SetRetry(unsigned);
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetEventName();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetData();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetLastEventId();
    unsigned GetRetry();
    void Clear();
    bool IsValid();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString();
    int ToString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    glwebtools::OptionalArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::AttributeValidator,glwebtools::AttributeFormatter> m_eventName;  // 0x0
    glwebtools::RequiredArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::AttributeValidator,glwebtools::AttributeFormatter> m_data;  // 0x28
    glwebtools::OptionalArgument<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::AttributeValidator,glwebtools::AttributeFormatter> m_lastEventId;  // 0x50
    glwebtools::OptionalArgument<unsigned int,glwebtools::AttributeValidator,glwebtools::AttributeFormatter> m_retry;  // 0x78
};
}

#endif

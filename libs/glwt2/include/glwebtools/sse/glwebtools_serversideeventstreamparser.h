// libs/glwt2/include/glwebtools/sse/glwebtools_serversideeventstreamparser.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SSE_GLWEBTOOLS_SERVERSIDEEVENTSTREAMPARSER_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SSE_GLWEBTOOLS_SERVERSIDEEVENTSTREAMPARSER_H

namespace glwebtools {
class ServerSideEventStreamParser {  // sizeof 0x54
public:
    ServerSideEventStreamParser();
    int PushStream(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int PopEvent(glwebtools::ServerSideEvent&);
    bool HasEvent();
    void Clear();
private:
    int Push(char);
    int PushField();
    int PushEvent();
    void ClearBuffer();
    bool IsFeeding();
    glwebtools::ServerSideEventParser m_eventParser;  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_buffer;  // 0x14
    std::deque<glwebtools::ServerSideEvent,std::allocator<glwebtools::ServerSideEvent> > m_eventQueue;  // 0x34
    char m_lastChar;  // 0x50
};
}

#endif

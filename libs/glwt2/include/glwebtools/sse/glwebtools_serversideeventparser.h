// libs/glwt2/include/glwebtools/sse/glwebtools_serversideeventparser.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SSE_GLWEBTOOLS_SERVERSIDEEVENTPARSER_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SSE_GLWEBTOOLS_SERVERSIDEEVENTPARSER_H

namespace glwebtools {
class ServerSideEventParser {  // sizeof 0x14
public:
    int PushField(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int PopEvent(glwebtools::ServerSideEvent&);
    bool HasEvent();
    void Clear();
private:
    std::vector<glwebtools::ServerSideEventParser::Field,std::allocator<glwebtools::ServerSideEventParser::Field> > m_fields;  // 0x0
};
}

namespace glwebtools {
struct scoped_clear {  // sizeof 0x4
    scoped_clear(glwebtools::ServerSideEventParser&);
    ~scoped_clear();
    glwebtools::ServerSideEventParser &m_parser;  // 0x0
};
}

#endif

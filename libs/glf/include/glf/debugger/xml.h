// libs/glf/include/glf/debugger/xml.h
#ifndef LIBS_GLF_INCLUDE_GLF_DEBUGGER_XML_H
#define LIBS_GLF_INCLUDE_GLF_DEBUGGER_XML_H

namespace glf::debugger {
class XmlReader {  // sizeof 0x4
public:
    XmlReader(glf::debugger::XmlReader::Handler*);
    bool ParseTag(std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&, const char*);
    void Parse(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    glf::debugger::XmlReader::Handler *mHandler;  // 0x0
};
}

namespace glf::debugger {
class XmlWriter {  // sizeof 0x34
public:
    XmlWriter();
    void BeginTag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > >&);
    void BeginTag(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void EndTag();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetXml();
    std::vector<glf::debugger::XmlWriter::Tag,std::allocator<glf::debugger::XmlWriter::Tag> > mStack;  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mXml;  // 0x14
};
}

#endif

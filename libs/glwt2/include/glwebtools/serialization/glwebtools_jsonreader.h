// libs/glwt2/include/glwebtools/serialization/glwebtools_jsonreader.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_JSONREADER_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_JSONREADER_H

namespace glwebtools {
class JsonReader {  // sizeof 0x10
    JsonReader(const glwebtools::Json::Value&);
public:
    JsonReader(const char*);
    JsonReader(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    JsonReader();
    int parse(const char*);
    int parse(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int read(glwebtools::JSONArray&);
    int read(glwebtools::JSONObject&);
    int read(glwebtools::JSONValue&);
    int read(glwebtools::CustomAttributeList&);
    int read(glwebtools::CustomArgument&);
    int read(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int read(bool&);
    int read(double&);
    int read(unsigned&);
    int read(int&);
    int read(unsigned short&);
    int read(short&);
    int include(unsigned*, unsigned*, glwebtools::JSONArray&);
    int include(const char**, const char**, glwebtools::JSONObject&);
    int include(const char**, const char**, glwebtools::CustomAttributeList&);
    int exclude(unsigned*, unsigned*, glwebtools::JSONArray&);
    int exclude(const char**, const char**, glwebtools::JSONObject&);
    int exclude(const char**, const char**, glwebtools::CustomAttributeList&);
    glwebtools::JsonReader operator[](const char*);
    glwebtools::JsonReader operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    glwebtools::JsonReader operator[](unsigned);
    bool IsValid();
    bool isInt();
    bool isUInt();
    bool isDouble();
    bool isBool();
    bool isString();
    bool isObject();
    bool isArray();
    glwebtools::JsonReader::Iterator begin();
    glwebtools::JsonReader::Iterator end();
    unsigned size();
    void ToString(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString();
private:
    glwebtools::Json::Value m_root;  // 0x0
};
}

#endif

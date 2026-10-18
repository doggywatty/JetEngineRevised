// libs/glwt2/include/glwebtools/serialization/glwebtools_jsonbase.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_JSONBASE_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_JSONBASE_H

namespace glwebtools {
struct JSONArray {  // sizeof 0x14
    int Set(unsigned, const glwebtools::JSONValue&);
    int Get(unsigned, glwebtools::JSONValue&);
    const glwebtools::JSONValue &operator[](unsigned);
    glwebtools::JSONValue &operator[](unsigned);
    void Clear();
    std::_Vector_const_iterator<std::pair<unsigned int,glwebtools::JSONValue>,std::allocator<std::pair<unsigned int,glwebtools::JSONValue> > > begin();
    std::_Vector_iterator<std::pair<unsigned int,glwebtools::JSONValue>,std::allocator<std::pair<unsigned int,glwebtools::JSONValue> > > begin();
    std::_Vector_const_iterator<std::pair<unsigned int,glwebtools::JSONValue>,std::allocator<std::pair<unsigned int,glwebtools::JSONValue> > > end();
    std::_Vector_iterator<std::pair<unsigned int,glwebtools::JSONValue>,std::allocator<std::pair<unsigned int,glwebtools::JSONValue> > > end();
private:
    std::_Vector_const_iterator<std::pair<unsigned int,glwebtools::JSONValue>,std::allocator<std::pair<unsigned int,glwebtools::JSONValue> > > Find(unsigned);
    std::_Vector_iterator<std::pair<unsigned int,glwebtools::JSONValue>,std::allocator<std::pair<unsigned int,glwebtools::JSONValue> > > Find(unsigned);
    std::vector<std::pair<unsigned int,glwebtools::JSONValue>,std::allocator<std::pair<unsigned int,glwebtools::JSONValue> > > m_values;  // 0x0
    static glwebtools::JSONValue s_invalid_value;
};
}

namespace glwebtools {
struct JSONObject {  // sizeof 0x14
    int Set(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const glwebtools::JSONValue&);
    int Get(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, glwebtools::JSONValue&);
    const glwebtools::JSONValue &operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    glwebtools::JSONValue &operator[](const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void Clear();
    std::_Vector_const_iterator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue> > > begin();
    std::_Vector_iterator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue> > > begin();
    std::_Vector_const_iterator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue> > > end();
    std::_Vector_iterator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue> > > end();
private:
    std::_Vector_const_iterator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue> > > Find(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::_Vector_iterator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue> > > Find(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue>,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,glwebtools::JSONValue> > > m_values;  // 0x0
    static glwebtools::JSONValue s_invalid_value;
};
}

namespace glwebtools {
struct JSONValue {  // sizeof 0x20
    JSONValue(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    JSONValue();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &operator const class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > &();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_value;  // 0x0
};
}

#endif

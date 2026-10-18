// libs/glwt2/include/glwebtools/serialization/glwebtools_attributeformatter.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_ATTRIBUTEFORMATTER_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_SERIALIZATION_GLWEBTOOLS_ATTRIBUTEFORMATTER_H

namespace glwebtools {
class AttributeFormatter {  // sizeof 0x1
public:
    bool operator==(const glwebtools::AttributeFormatter&);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString(const char*);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString(double, int);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString(float, int);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString(__int64);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString(unsigned);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString(int);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString(bool);
protected:
    static void Assert(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
};
}

#endif

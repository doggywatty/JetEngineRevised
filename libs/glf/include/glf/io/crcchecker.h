// libs/glf/include/glf/io/crcchecker.h
#ifndef LIBS_GLF_INCLUDE_GLF_IO_CRCCHECKER_H
#define LIBS_GLF_INCLUDE_GLF_IO_CRCCHECKER_H

namespace glf {
class CrcChecker {  // sizeof 0x1
public:
    static void AddCrcMap(const char*, bool);
    static unsigned CRC32(const void*, int, unsigned);
    static bool HasFileEntry(const char*);
    static bool DoCrcCheck(const char*, const void*, int);
    static void SetFileEntry(const char*, unsigned, bool);
    static void AddFileEntry(const char*, unsigned, bool);
    static void UpdateFileEntry(const char*, const void*, int);
    static void SetDynamicMapFileName(const char*);
private:
    static void _WriteDynamicCrcMap();
    static void _CypherBuffer(const char*, char*&, int);
    static void _DecypherBuffer(const char*, char*&, int);
    static std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<unsigned int,bool>,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::pair<unsigned int,bool> > > > mCrcMap;
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > mDynMapFileName;
};
}

#endif

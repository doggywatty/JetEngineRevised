// libs/manhattan/include/dlc/hashfileparser.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_HASHFILEPARSER_H
#define LIBS_MANHATTAN_INCLUDE_DLC_HASHFILEPARSER_H

namespace manhattan::dlc {
class HashFileParser {  // sizeof 0x1
public:
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetHashFile(const Json::Value&);
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetHash(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned, const Json::Value&);
    static unsigned GetChunkSize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const Json::Value&);
    static unsigned GetFileSize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const Json::Value&);
private:
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > HASH_FILE_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > FILE_METADATA_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > HASHES_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > CHUNK_SIZE_KEY;
    static const std::basic_string<char,std::char_traits<char>,std::allocator<char> > SIZE_KEY;
};
}

#endif

// libs/glwt2/include/glwebtools/tool/glwebtools_codec.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_TOOL_GLWEBTOOLS_CODEC_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_TOOL_GLWEBTOOLS_CODEC_H

namespace glwebtools {
class Codec {  // sizeof 0x1
public:
    static bool EncodeBase64(const unsigned char*, unsigned, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, unsigned);
    static bool EncodeBase64(const unsigned char*, unsigned, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, glwebtools::Codec::MaxLineLength);
    static unsigned DecodeBase64(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*, bool);
    static unsigned DecodeBase64(const char*, unsigned, void*, bool);
    static bool IsInBase64Alphabet(char);
    static unsigned GetDecodedBase64DataSize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool);
    static bool EncodeBase64Custom(const unsigned char*, unsigned, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*);
    static unsigned DecodeBase64Custom(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*, const char*);
    static unsigned DecodeBase64Custom(const char*, unsigned, void*, const char*);
    static void GenerateBase64CustomKey(char*, unsigned, unsigned);
    static void GenerateBase64CustomKey(char*);
    static bool EncodeBlob(const unsigned char*, unsigned, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static unsigned DecodeBlob(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*);
    static unsigned GetDecodedBlobDataSize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool EncodeUrlRFC1738(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool EncodeUrlRFC3986(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool DecodeUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static bool GenerateMD5(const void*, unsigned, char*);
    static bool GenerateSHA256(const void*, unsigned, char*);
    static bool EncryptTEA(const void*, unsigned, void*, unsigned, const unsigned*);
    static bool DecryptTEA(const void*, unsigned, void*, unsigned, const unsigned*);
    static unsigned GetEncryptedTEADataSize(unsigned);
    static bool EncryptXTEA(const void*, unsigned, void*, unsigned, const unsigned*);
    static bool DecryptXTEA(const void*, unsigned, void*, unsigned, const unsigned*);
    static unsigned GetEncryptedXTEADataSize(unsigned);
    static bool EncryptXXTEA(const void*, unsigned, void*, unsigned, const unsigned*);
    static bool DecryptXXTEA(const void*, unsigned, void*, unsigned, const unsigned*);
    static unsigned GetEncryptedXXTEADataSize(unsigned);
    static unsigned Random(unsigned&, unsigned&);
private:
    static char SSEncDec_GetKeyFromChar(char);
    static char Dec64_GetKeyFromChar(char);
    static void EncUrl_GetKeysFromChar(char, char&, char&);
    static char EncUrl_GetCharFromKeys(const char*);
};
}

#endif

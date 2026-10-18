// libs/glwt2/include/glwebtools/tool/glwebtools_sha256.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_TOOL_GLWEBTOOLS_SHA256_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_TOOL_GLWEBTOOLS_SHA256_H

namespace glwebtools {
struct SHA256_CTX {  // sizeof 0x4
    SHA256_CTX();
    ~SHA256_CTX();
    void* m_implData;  // 0x0
    static const unsigned digest_length;
    static const unsigned digest_string_length;
};
}

#endif

// source/hostname.h
#ifndef SOURCE_HOSTNAME_H
#define SOURCE_HOSTNAME_H

struct hostname {  // sizeof 0x10
    char* rawalloc;  // 0x0
    char* encalloc;  // 0x4
    char* name;  // 0x8
    const char *dispname;  // 0xC
};

#endif

// core/jet/src/platform/win32/stackwalker.h
#ifndef CORE_JET_SRC_PLATFORM_WIN32_STACKWALKER_H
#define CORE_JET_SRC_PLATFORM_WIN32_STACKWALKER_H

class StackWalker {  // sizeof 0x1C
public:
    StackWalker(unsigned long, void*);
    StackWalker(int, const char*, unsigned long, void*);
    virtual ~StackWalker();
    int LoadModules();
    int ShowCallstack(void*, const _CONTEXT*, int (void*, unsigned __int64, void*, unsigned long, unsigned long*, void*)*, void*);
protected:
    virtual void OnSymInit(const char*, unsigned long, const char*);
    virtual void OnLoadModule(const char*, const char*, unsigned __int64, unsigned long, unsigned long, const char*, const char*, unsigned __int64);
    virtual void OnCallstackEntry(StackWalker::CallstackEntryType, StackWalker::CallstackEntry&);
    virtual void OnDbgHelpErr(const char*, unsigned long, unsigned __int64);
    virtual void OnOutput(const char*);
    static int myReadProcMem(void*, unsigned __int64, void*, unsigned long, unsigned long*);
    StackWalkerInternal *m_sw;  // 0x4
    void* m_hProcess;  // 0x8
    unsigned long m_dwProcessId;  // 0xC
    int m_modulesLoaded;  // 0x10
    char* m_szSymPath;  // 0x14
    int m_options;  // 0x18
};

class StackWalkerInternal {  // sizeof 0x44
public:
    StackWalkerInternal(StackWalker*, void*);
    ~StackWalkerInternal();
    int Init(const char*);
private:
    int GetModuleListTH32(void*, unsigned long);
    int GetModuleListPSAPI(void*);
    unsigned long LoadModule(void*, const char*, const char*, unsigned __int64, unsigned long);
public:
    int LoadModules(void*, unsigned long);
    int GetModuleInfo(void*, unsigned __int64, StackWalkerInternal::IMAGEHLP_MODULE64_V2*);
    StackWalker *m_parent;  // 0x0
    HINSTANCE__ *m_hDbhHelp;  // 0x4
    void* m_hProcess;  // 0x8
    char* m_szSymPath;  // 0xC
    int (*pSC)(void*);  // 0x10
    void* (*pSFTA)(void*, unsigned __int64);  // 0x14
    int (*pSGLFA)(void*, unsigned __int64, unsigned long*, _IMAGEHLP_LINE64*);  // 0x18
    unsigned __int64 (*pSGMB)(void*, unsigned __int64);  // 0x1C
    int (*pSGMI)(void*, unsigned __int64, StackWalkerInternal::IMAGEHLP_MODULE64_V2*);  // 0x20
    unsigned long (*pSGO)();  // 0x24
    int (*pSGSFA)(void*, unsigned __int64, unsigned __int64*, _IMAGEHLP_SYMBOL64*);  // 0x28
    int (*pSI)(void*, char*, int);  // 0x2C
    unsigned __int64 (*pSLM)(void*, void*, char*, char*, unsigned __int64, unsigned long);  // 0x30
    unsigned long (*pSSO)(unsigned long);  // 0x34
    int (*pSW)(unsigned long, void*, void*, _tagSTACKFRAME64*, void*, int (void*, unsigned __int64, void*, unsigned long, unsigned long*)*, void* (void*, unsigned __int64)*, unsigned __int64 (void*, unsigned __int64)*, unsigned __int64 (void*, void*, _tagADDRESS64*)*);  // 0x38
    unsigned long (*pUDSN)(const char*, char*, unsigned long, unsigned long);  // 0x3C
    int (*pSGSP)(void*, char*, unsigned long);  // 0x40
};

#endif

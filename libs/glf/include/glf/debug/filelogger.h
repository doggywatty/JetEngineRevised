// libs/glf/include/glf/debug/filelogger.h
#ifndef LIBS_GLF_INCLUDE_GLF_DEBUG_FILELOGGER_H
#define LIBS_GLF_INCLUDE_GLF_DEBUG_FILELOGGER_H

namespace glf {
struct FileLogger {  // sizeof 0xF8
    static glf::FileLogger *GetInstance();
    bool OpenFile(bool);
    void Enable(bool);
    bool IsEnabled();
    void Write(const char*);
    void GetLastLog(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void GetCurrentLog(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static void Print(const char*, );
    static void Println(const char*, );
    static void PrintTimestamp();
private:
    static void GetLog(std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const char*);
    FileLogger();
    ~FileLogger();
    void WriteEot();
    void RewindToLastEot();
public:
    static const char EOT;
private:
    glf::FileStream mLogFile;  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mLogFilename;  // 0xB0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > mOldLogFilename;  // 0xD0
    glf::Mutex mMutex;  // 0xF0
    bool mIsEnabled;  // 0xF4
};
}

#endif

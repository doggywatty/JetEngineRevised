// core/jet/src/system.cpp
#include "system.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <unordered_map>
#include "jassert.h"
#include "thread/taskmgr.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef GetFreeSpace
#else
#include <unistd.h>
#endif

namespace jet {

unsigned System::s_memoryPageAlignment = 0;
video::Display* System::s_displays[System::k_max_displays] = {};
video::Driver* System::s_driver = nullptr;
Application* System::s_application = nullptr;
String System::s_applicationPath;
String System::s_applicationTitle;

static System::Platform s_platform;
static bool s_initialized = false;
static bool s_logging = true;
static FILE* s_logFile = nullptr;
static std::unordered_map<String, String> s_params;

void System::SetPlatform(const Platform& p) { s_platform = p; }

// Asserts show a message box (the file name without its directory, the line and the message).
static assertLib::AssertType JetDefaultHandler(const char* expr, const char* msg, const char* file, int line)
{
    const char* name = file;
    for (const char* c = file; c && *c; ++c)
        if (*c == '\\' || *c == '/')
            name = c + 1;
    char buf[512];
    snprintf(buf, sizeof(buf), "%s(%d): %s%s%s", name ? name : "", line, expr ? expr : "", msg ? " " : "", msg ? msg : "");
    if (s_platform.messageBox)
        s_platform.messageBox("Assert", buf);
    else
        fprintf(stderr, "%s\n", buf);
    return assertLib::AssertTypeDef::BREAK;
}

void System::InitHeadless()
{
    if (s_initialized)
        return;
    s_initialized = true;
    assertLib::SetAssertHandler(JetDefaultHandler);
    SetLoggingEnabled(true);
    ComputeMemoryPageAlignment();
    thread::TaskMgr* m = new thread::TaskMgr;
    m->Init();
}

void System::ComputeMemoryPageAlignment()
{
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    s_memoryPageAlignment = si.dwAllocationGranularity;
#else
    long p = sysconf(_SC_PAGESIZE);
    s_memoryPageAlignment = p > 0 ? unsigned(p) : 1;
#endif
    if (!s_memoryPageAlignment)
        s_memoryPageAlignment = 1;
}

unsigned System::GetMemoryPageAlignment()
{
    if (!s_memoryPageAlignment)
        ComputeMemoryPageAlignment();
    return s_memoryPageAlignment;
}

// "-key value" pairs; a flag followed by another flag (or nothing) maps to itself.
void System::ParseCommandLine(int argc, const char** argv)
{
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a.size() < 2 || a[0] != '-')
            continue;
        std::string key = a.substr(1);
        if (i + 1 < argc && argv[i + 1][0] && argv[i + 1][0] != '-') {
            s_params[String(key)] = String(argv[i + 1]);
            ++i;
        } else {
            s_params[String(key)] = String(key);
        }
    }
}

const String& System::GetParam(const String& key)
{
    auto it = s_params.find(key);
    return it == s_params.end() ? String::null : it->second;
}

bool System::HasParam(const String& key) { return s_params.count(key) != 0; }

void System::Init(video::Driver* driver, video::Display* display)
{
    InitHeadless();
    s_driver = driver;
    AttachDisplay(0, display);
}

void System::Free()
{
    if (thread::TaskMgr* m = thread::TaskMgr::GetInstance()) {
        m->FinishAllTasks();
        delete m;
    }
    for (int i = 0; i < k_max_displays; ++i)
        s_displays[i] = nullptr;
    s_driver = nullptr;
    s_application = nullptr;
    if (s_logFile) {
        fclose(s_logFile);
        s_logFile = nullptr;
    }
    s_initialized = false;
}

void System::AttachDisplay(unsigned i, video::Display* d)
{
    if (i < unsigned(k_max_displays))
        s_displays[i] = d;
}

void System::DetachDisplay(unsigned i)
{
    if (i < unsigned(k_max_displays))
        s_displays[i] = nullptr;
}

video::Display* System::GetDisplay(unsigned i) { return i < unsigned(k_max_displays) ? s_displays[i] : nullptr; }

void System::SetApplication(const String& path, Application* app, const String& title)
{
    s_applicationPath = path;
    s_application = app;
    s_applicationTitle = title;
}

void System::SetupLogFile(const String& path)
{
    if (s_logFile)
        fclose(s_logFile);
    s_logFile = fopen(path.c_str(), "w");
}

void System::EnableLogFile(bool on)
{
    if (!on && s_logFile) {
        fclose(s_logFile);
        s_logFile = nullptr;
    }
}

void System::SetLoggingEnabled(bool on) { s_logging = on; }

s64 System::CurrentTimeMillis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

s64 System::GetUTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

s64 System::GetTime() { return GetUTime() / 1000; }

String System::GetDeviceString() { return s_platform.deviceString ? s_platform.deviceString() : String("desktop"); }

String System::GetDeviceFirmwareString() { return s_platform.deviceFirmware ? s_platform.deviceFirmware() : String::null; }

String System::GetTempFolder()
{
    if (s_platform.tempFolder)
        return s_platform.tempFolder();
    std::error_code ec;
    return String(std::filesystem::temp_directory_path(ec).u8string());
}

static String s_userFolder;

void System::SetUserFolder(const String& path)
{
    s_userFolder = path;
    std::error_code ec;
    if (!path.empty())
        std::filesystem::create_directories(std::filesystem::u8path(path.c_str()), ec);
}

String System::GetUserFolder()
{
    if (!s_userFolder.empty())
        return s_userFolder;
    if (s_platform.userFolder)
        return s_platform.userFolder();
    String folder("savegame");
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::u8path(folder.c_str()), ec);
    return folder;
}

u64 System::GetFreeSpace()
{
    if (s_platform.freeSpace)
        return s_platform.freeSpace();
    std::error_code ec;
    return u64(std::filesystem::space(std::filesystem::u8path(GetUserFolder().c_str()), ec).available);
}

}

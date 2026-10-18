// core/jet/include/system.h
#ifndef CORE_JET_INCLUDE_SYSTEM_H
#define CORE_JET_INCLUDE_SYSTEM_H

#include "core/pstring.h"

namespace jet {

class Application;
namespace video {
class Driver;
class Display;
}

// Platform services. Anything a host platform must provide goes through Platform (set before Init).
class System {
public:
    struct Platform {
        String (*userFolder)() = nullptr;      // saves; default: "savegame" in the working directory
        String (*tempFolder)() = nullptr;
        String (*deviceString)() = nullptr;
        String (*deviceFirmware)() = nullptr;
        u64 (*freeSpace)() = nullptr;           // bytes
        void (*messageBox)(const char* title, const char* text) = nullptr;
    };
    static void SetPlatform(const Platform& p);

    static void InitHeadless();
    static void ParseCommandLine(int argc, const char** argv);
    static void Init(video::Driver* driver, video::Display* display);
    static void Free();
    static void AttachDisplay(unsigned i, video::Display* d);
    static void DetachDisplay(unsigned i);
    static video::Display* GetDisplay(unsigned i);
    static video::Display* GetMainDisplay() { return GetDisplay(0); }
    static Application* GetApplication() { return s_application; }
    static void SetApplication(const String& path, Application* app, const String& title);
    static video::Driver* GetDriver() { return s_driver; }
    static const String& GetApplicationPath() { return s_applicationPath; }
    static const String& GetApplicationTitle() { return s_applicationTitle; }

    static void SetupLogFile(const String& path);
    static void EnableLogFile(bool on);
    static void SetLoggingEnabled(bool on);

    static s64 CurrentTimeMillis();  // wall clock
    static s64 GetTime();            // monotonic ms
    static s64 GetUTime();           // monotonic us
    static String GetDeviceString();
    static String GetDeviceFirmwareString();
    static void SetDeviceString(const String&) {}
    static String GetTempFolder();
    // The folder the saves go in: SetUserFolder's, else the platform's, else a "savegame" folder in the working directory
    // (created when asked for). A host picks another with SetUserFolder before the game loads its profile.
    static String GetUserFolder();
    static void SetUserFolder(const String& path);
    static u64 GetFreeSpace();
    static unsigned GetMemoryPageAlignment();

    // Command line pairs "-key value" / "-flag" collected by ParseCommandLine.
    static const String& GetParam(const String& key);
    static bool HasParam(const String& key);

private:
    static void ComputeMemoryPageAlignment();

    static const int k_max_displays = 10;
    static unsigned s_memoryPageAlignment;
    static video::Display* s_displays[k_max_displays];
    static video::Driver* s_driver;
    static Application* s_application;
    static String s_applicationPath;
    static String s_applicationTitle;
};

}

#endif

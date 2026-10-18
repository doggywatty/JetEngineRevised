// libs/glf/include/glf/app.h
#ifndef LIBS_GLF_INCLUDE_GLF_APP_H
#define LIBS_GLF_INCLUDE_GLF_APP_H

namespace glf {
class App : public glf::EventReceiver, private glf::NonCopyable {  // sizeof 0x304
public:
    virtual void SetCreationSettings(glf::CreationSettings&);
    virtual void SetPlatformInfos(glf::PlatformInfo&);
    void SetAsMainThread();
    void UnsetAsMainThread();
    virtual bool Init(glf::CreationSettings&);
    virtual void Deinit();
    virtual void Run();
    virtual void Destroy();
    virtual void Update();
    virtual bool QueryInfo(const char*, char*, unsigned&);
    virtual void BeginProfilerEvent(const char*);
    virtual void EndProfilerEvent();
    void EnableProfilerEvents(bool);
    bool IsProfilerEventsEnabled();
    void* GetFrameBufferObject(int);
    bool HasFrameBufferObject(int);
    bool Prepare(int);
    void Present(int);
    bool ConnectDisplay(int);
    bool DisconnectDisplay(int);
    glf::DisplayStatus GetDisplayStatus(int);
    bool IsRunnable();
    bool IsRunning();
    void Stop();
    void SetTargetFps(float);
    float GetTargetFps();
    glf::PlatformInfo &GetPlatformInfos();
    const glf::AppSettings &GetAppSettings();
    const glf::CreationSettings &GetCreationSettings();
    bool HasContext();
    bool ReserveContext();
    bool ReleaseContext();
    glf::Orientation GetOrientation();
    bool IsOrientationSupported(glf::Orientation);
    bool SetOrientation(glf::Orientation);
    void SetSupportedOrientations(int);
    glf::Point<short> ConvertPosDeviceToScreen(const glf::Point<float>&);
    glf::Point<float> ConvertPosScreenToDevice(const glf::Point<short>&);
    glf::Point<float> ConvertPosScreenToNormalizedDevice(const glf::Point<short>&);
    glf::Point<short> ConvertPosNormalizedDeviceToScreen(const glf::Point<float>&);
    glf::Point<float> ConvertPosScreenToNormalizedScreen(const glf::Point<short>&);
    glf::Point<short> ConvertPosNormalizedScreenToScreen(const glf::Point<float>&);
    static glf::App *GetInstance();
    glf::EventManager &GetEventMgr();
    glf::InputManager &GetInputMgr();
    glf::Fs &GetFs();
    glf::remote::Controller *GetRemoteController();
    void SetRemoteController(glf::remote::Controller*);
    const char *GetDeviceName();
    void GetScreenSize(int&, int&, int);
    void GetWindowSize(int&, int&, int);
    void FileOutAllFiles();
    void StartRemoteController(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
protected:
    App(glf::App::CreationFlag);
    virtual ~App();
public:
    bool MyInit();
    glf::App::Impl &GetImpl();
protected:
    bool mIsRunning;  // 0x8
    glf::EventManager &mEventManager;  // 0xC
    glf::InputManager &mInputManager;  // 0x10
    glf::Fs &mFs;  // 0x14
private:
    glf::Orientation mOrientation;  // 0x18
    int mSupportedOrientations;  // 0x1C
    glf::AppSettings mAppSettings;  // 0x20
    glf::PlatformInfo mPlatformInfos;  // 0x38
    int mReservedExtraContexts;  // 0x60
    glf::SpinLock mExtraContextLock;  // 0x64
    glf::CreationSettings mCreationSettings;  // 0x68
    float mTargetFps;  // 0x2F8
    static glf::App *mInstance;
    glf::remote::Controller *mRemoteController;  // 0x2FC
    glf::App::Impl &m;  // 0x300
};
}

#endif

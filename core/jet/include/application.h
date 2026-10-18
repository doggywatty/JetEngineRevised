// core/jet/include/application.h
#ifndef CORE_JET_INCLUDE_APPLICATION_H
#define CORE_JET_INCLUDE_APPLICATION_H

#include <map>
#include "core/pstring.h"
#include "util/fps.h"

namespace jet {
// The game's host: named launch parameters, the frame timing and the per frame bookkeeping.
// The debugger server and the memory/profile contexts of the shipped class are developer tools and are not part of it.
class Application {  // sizeof 0x290
public:
    Application();
    virtual ~Application() {}
    Application(const Application&) = delete;
    Application &operator=(const Application&) = delete;
    virtual void OnDisplayAttached(unsigned) = 0;
    virtual void OnDisplayDetached(unsigned) = 0;
    virtual void Init() { m_isInitialized = true; }
    virtual void Shutdown() { m_isInitialized = false; }
    bool IsInitialized() const { return m_isInitialized; }
    virtual void OnUpdate() = 0;
    // the host is about to stop the application (or keep it in the background): the user data is saved (Game::OnSaveUserData @0x52c210)
    virtual void OnSaveUserData() {}
    void PreUpdate() { m_fps.Tick(false); }
    void AddParam(const jet::String &k, const jet::String &v) { m_params[k] = v; }
    float GetParam(const jet::String&, float def);
    int GetParam(const jet::String&, int def);
    jet::String GetParam(const jet::String&);
    unsigned GetTotalFrameTime() const { return m_totalFrameTime; }
    float GetTotalFrameTimeAsFloat() const { return m_totalFrameTimeF; }
    float GetTotalFrameTimeSeconds() const { return m_totalFrameTimeS; }
    float GetTotalFrameTimeInv() const { return m_totalFrameTimeInv; }
    unsigned GetUpdateTime() const { return m_updateTimeUs / 1000; }
    unsigned GetUpdateTimeUs() const { return m_updateTimeUs; }
    float GetUpdateTimeAsFloat() const { return m_updateTimeF; }
    float GetUpdateTimeSeconds() const { return m_updateTimeS; }
    float GetUpdateTimeInv() const { return m_updateTimeInv; }
    unsigned GetRenderTime() const { return m_renderTime; }
    float GetRenderTimeAsFloat() const { return m_renderTimeF; }
    float GetRenderTimeSeconds() const { return m_renderTimeS; }
    float GetRenderTimeInv() const { return m_renderTimeInv; }
    unsigned GetRenderTimeNoSlomo() const { return m_renderTimeNoSlomo; }
    void SetTotalFrameTime(unsigned);
    void SetUpdateTime(unsigned ms) { SetUpdateTimeUs(ms * 1000); }
    void SetUpdateTimeUs(unsigned);
    void SetRenderTime(unsigned);
    void SetRenderTimeNoSlomo(unsigned t) { m_renderTimeNoSlomo = t; }
    const jet::util::Fps &GetFps() const { return m_fps; }
private:
    bool m_isInitialized;  // 0x4
    unsigned m_totalFrameTime;  // 0x8
    float m_totalFrameTimeF, m_totalFrameTimeS, m_totalFrameTimeInv;  // 0xC 0x10 0x14
    unsigned m_updateTimeUs;  // 0x18
    float m_updateTimeF, m_updateTimeS, m_updateTimeInv;  // 0x1C 0x20 0x24
    unsigned m_renderTime;  // 0x28
    float m_renderTimeF, m_renderTimeS, m_renderTimeInv;  // 0x2C 0x30 0x34
    unsigned m_renderTimeNoSlomo;  // 0x38
    std::map<jet::String, jet::String> m_params;  // 0x54
    jet::util::Fps m_fps;  // 0x190
};
}

#endif

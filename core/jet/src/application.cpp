// core/jet/src/application.cpp
#include <cstdlib>
#include "application.h"

namespace jet {

// @0x6a6780
Application::Application()
    : m_isInitialized(false), m_totalFrameTime(0), m_totalFrameTimeF(0), m_totalFrameTimeS(0), m_totalFrameTimeInv(0),
      m_updateTimeUs(0), m_updateTimeF(0), m_updateTimeS(0), m_updateTimeInv(0), m_renderTime(0), m_renderTimeF(0),
      m_renderTimeS(0), m_renderTimeInv(0), m_renderTimeNoSlomo(0) {}

// @0x6a69ac
jet::String Application::GetParam(const jet::String &k) {
    std::map<jet::String, jet::String>::const_iterator it = m_params.find(k);
    return it == m_params.end() ? jet::String() : it->second;
}

// @0x6a69f0
int Application::GetParam(const jet::String &k, int def) {
    std::map<jet::String, jet::String>::const_iterator it = m_params.find(k);
    return it == m_params.end() ? def : atoi(it->second.c_str());
}

// @0x6a6a28
float Application::GetParam(const jet::String &k, float def) {
    std::map<jet::String, jet::String>::const_iterator it = m_params.find(k);
    return it == m_params.end() ? def : (float)strtod(it->second.c_str(), 0);
}

// @0x6a6a78: the frame time in microseconds, as ms, seconds and 1 / ms
void Application::SetUpdateTimeUs(unsigned us) {
    m_updateTimeUs = us;
    m_updateTimeF = (float)us * 0.001f;
    m_updateTimeInv = us ? 1.0f / m_updateTimeF : 1.0f;
    m_updateTimeS = m_updateTimeF * 0.001f;
}

// @0x6a6ab4
void Application::SetRenderTime(unsigned ms) {
    m_renderTime = ms;
    m_renderTimeF = (float)ms;
    m_renderTimeInv = ms ? 1.0f / m_renderTimeF : 1.0f;
    m_renderTimeS = m_renderTimeF * 0.001f;
}

// @0x6a6ae8
void Application::SetTotalFrameTime(unsigned ms) {
    m_totalFrameTime = ms;
    m_totalFrameTimeF = (float)ms;
    m_totalFrameTimeInv = ms ? 1.0f / m_totalFrameTimeF : 1.0f;
    m_totalFrameTimeS = m_totalFrameTimeF * 0.001f;
}

}

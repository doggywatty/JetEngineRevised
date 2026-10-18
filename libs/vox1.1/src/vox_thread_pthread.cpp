// libs/vox1.1/src/vox_thread_pthread.cpp
// vox::VoxThread (the mutex is header only: std::recursive_mutex). One portable implementation for every platform; the .so
// builds vox_thread_pthread.cpp, whose thread is a loop that sleeps between two calls of its function.
#include "vox_mutex.h"
#include <chrono>

namespace vox {

VoxThread::VoxThread(Function fn, void *arg0, void *arg1, int periodMs, const char *name)
    : m_function(fn), m_arg0(arg0), m_arg1(arg1), m_periodMs(periodMs), m_name(name), m_running(true) {
    m_thread = std::thread(&VoxThread::Run, this);
}

VoxThread::~VoxThread() { Stop(); }

void VoxThread::Stop() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void VoxThread::Run() {
    while (m_running) {
        m_function(m_arg0, m_arg1);
        std::this_thread::sleep_for(std::chrono::milliseconds(m_periodMs));
    }
}

}  // namespace vox

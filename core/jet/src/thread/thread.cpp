// core/jet/src/thread/thread.cpp
#include "thread/thread.h"

namespace jet {
namespace thread {

thread_local unsigned t_crtThreadIdx = 0;
static ContextHook s_contextHook = nullptr;

void SetContextHook(ContextHook h) { s_contextHook = h; }

Thread::Thread(unsigned idx, bool needsContext) : m_idx(idx), m_running(false), m_needsContext(needsContext) {}

Thread::~Thread() { Stop(); }

void Thread::ThreadFunc(Thread* t)
{
    t_crtThreadIdx = t->m_idx;
    if (t->m_needsContext && s_contextHook)
        s_contextHook(t);
    t->Run();
}

void Thread::Run()
{
    while (m_running) {
        std::shared_ptr<Task> task;
        {
            std::lock_guard<std::mutex> l(m_tasksMutex);
            task = m_crt.task;
        }
        if (!task)
            WaitForNewTask();
        else if (task->Run())
            FinishTask();
    }
}

void Thread::Start()
{
    if (m_running)
        return;
    m_running = true;
    if (m_idx == 0 || m_thread)
        return;
    m_thread.reset(new std::thread(ThreadFunc, this));
}

void Thread::Stop()
{
    if (m_thread) {
        {
            std::lock_guard<std::mutex> l(m_tasksMutex);
            m_running = false;
        }
        m_cond.notify_one();
        if (m_thread->joinable())
            m_thread->join();
        m_thread.reset();
    }
    std::lock_guard<std::mutex> l(m_tasksMutex);
    m_crt = detail::TaskData();
}

void Thread::NotifyTaskAvailable()
{
    if (m_idx)
        m_cond.notify_one();
}

void Thread::WaitForNewTask()
{
    std::unique_lock<std::mutex> l(m_tasksMutex);
    if (m_tasks.empty()) {
        m_crt = detail::TaskData();
        if (m_idx && m_running)
            m_cond.wait(l);
    } else {
        m_crt = m_tasks.front();
        m_tasks.pop_front();
    }
}

void Thread::FinishTask()
{
    std::lock_guard<std::mutex> l(m_tasksMutex);
    if (m_crt.task)
        m_crt.task->SetIsFinished();
    m_crt = detail::TaskData();
}

}
}

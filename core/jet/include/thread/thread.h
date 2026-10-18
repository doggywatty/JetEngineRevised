// core/jet/include/thread/thread.h
#ifndef CORE_JET_INCLUDE_THREAD_THREAD_H
#define CORE_JET_INCLUDE_THREAD_THREAD_H

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include "thread/task.h"

namespace jet {
namespace thread {

namespace detail {
struct TaskData {
    unsigned id = 0;
    std::shared_ptr<Task> task;
};
}

// Called on a worker thread that needs the GL context (platform layer sets it; Android attaches JNI too).
typedef void (*ContextHook)(void* thread);
void SetContextHook(ContextHook h);

// Index 0 is the calling (main) thread and never gets an OS thread.
class Thread {
public:
    Thread(unsigned idx, bool needsContext);
    ~Thread();

    void Start();
    void Stop();
    bool IsRunning() const { return m_running; }
    unsigned GetIdx() const { return m_idx; }

    void NotifyTaskAvailable();
    bool HasAnyTask() const { return m_crt.id != 0; }
    bool HasTask(unsigned id) const { return m_crt.id == id; }

private:
    friend class TaskMgr;
    static void ThreadFunc(Thread* t);
    void Run();
    void WaitForNewTask();
    void FinishTask();

    std::deque<detail::TaskData> m_tasks;
    detail::TaskData m_crt;
    std::unique_ptr<std::thread> m_thread;
    unsigned m_idx;
    volatile bool m_running;
    bool m_needsContext;
    std::condition_variable m_cond;
    mutable std::mutex m_tasksMutex;
};

}
}

#endif

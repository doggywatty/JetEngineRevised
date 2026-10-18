// core/jet/include/thread/task.h
#ifndef CORE_JET_INCLUDE_THREAD_TASK_H
#define CORE_JET_INCLUDE_THREAD_TASK_H

#include <atomic>

namespace jet {
namespace thread {

// Run() is called until it returns true.
class Task {
public:
    virtual ~Task() {}
    virtual bool Run() = 0;
    virtual void Reset()
    {
        if (m_isFinished > 0)
            --m_isFinished;
    }

    bool IsFinished() const { return m_isFinished != 0; }
    unsigned GetId() const { return m_id; }
    void RunAll()
    {
        Reset();
        while (!Run()) {
        }
        ++m_isFinished;
    }

protected:
    Task() : m_isFinished(0), m_id(0) {}
    void SetIsFinished()
    {
        if (m_isFinished == 0)
            ++m_isFinished;
    }

private:
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    friend class TaskMgr;
    friend class Thread;

    std::atomic<int> m_isFinished;
    unsigned m_id;
};

}
}

#endif

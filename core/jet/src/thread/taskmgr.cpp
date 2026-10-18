// core/jet/src/thread/taskmgr.cpp
#include "thread/taskmgr.h"
#include <algorithm>
#include <chrono>

namespace jet {
namespace thread {

extern thread_local unsigned t_crtThreadIdx;

// Main, rendering, streaming and skinning share thread 0; the user types get a worker each.
TaskMgr::TaskMgr() : m_lastId(1), m_threadCount(5)
{
    m_typeToThreadIdx = {0, 0, 0, 0, 1, 2, 3, 4};
    m_threadIdxToTypes.resize(m_threadCount, 0);
}

TaskMgr::~TaskMgr()
{
    for (Thread* t : m_threads)
        delete t;
}

void TaskMgr::Init()
{
    m_threads.resize(m_threadCount, nullptr);
    for (unsigned i = 0; i < m_threadCount; ++i) {
        unsigned mask = 0;
        for (unsigned t = 0; t < TypeDef::COUNT; ++t)
            if (m_typeToThreadIdx[t] == i)
                mask |= 1u << t;
        m_threadIdxToTypes[i] = mask;
        bool gl = m_typeToThreadIdx[TypeDef::RENDERING] == i || m_typeToThreadIdx[TypeDef::STREAMING] == i ||
                  m_typeToThreadIdx[TypeDef::SKINNING] == i;
        m_threads[i] = new Thread(i, gl);
    }
}

unsigned TaskMgr::GetCrtThreadIdx() const { return t_crtThreadIdx; }

bool TaskMgr::CrtThreadHasType(Type type) const { return ThreadHasType(t_crtThreadIdx, type); }

bool TaskMgr::ThreadHasType(unsigned idx, Type type) const { return (m_threadIdxToTypes[idx] & (1u << type)) != 0; }

unsigned TaskMgr::AddTask(const std::shared_ptr<Task>& task, Type type)
{
    if (!task)
        return 0;
    task->Reset();
    task->m_id = m_lastId++;
    detail::TaskData d;
    d.id = task->m_id;
    d.task = task;
    unsigned idx = m_typeToThreadIdx[type];
    if (idx == 0) {
        task->RunAll();
        return d.id;
    }
    Thread* t = m_threads[idx];
    t->Start();
    {
        std::lock_guard<std::mutex> l(t->m_tasksMutex);
        t->m_tasks.push_back(d);
    }
    t->NotifyTaskAvailable();
    return d.id;
}

bool TaskMgr::HasTasks() const
{
    for (Thread* t : m_threads) {
        if (!t->m_running)
            continue;
        std::lock_guard<std::mutex> l(t->m_tasksMutex);
        if (!t->m_tasks.empty() || t->m_crt.id)
            return true;
    }
    return false;
}

bool TaskMgr::HasTasks(Type type) const
{
    Thread* t = m_threads[m_typeToThreadIdx[type]];
    if (!t->m_running)
        return false;
    std::lock_guard<std::mutex> l(t->m_tasksMutex);
    return !t->m_tasks.empty() || t->m_crt.id;
}

bool TaskMgr::HasTask(unsigned id) const
{
    if (!id)
        return false;
    for (Thread* t : m_threads) {
        if (!t->m_running)
            continue;
        std::lock_guard<std::mutex> l(t->m_tasksMutex);
        if (t->m_crt.id == id)
            return true;
        for (const detail::TaskData& d : t->m_tasks)
            if (d.id == id)
                return true;
    }
    return false;
}

void TaskMgr::CancelTask(unsigned id)
{
    if (!id)
        return;
    for (Thread* t : m_threads) {
        if (!t->m_running)
            continue;
        std::lock_guard<std::mutex> l(t->m_tasksMutex);
        t->m_tasks.erase(std::remove_if(t->m_tasks.begin(), t->m_tasks.end(), [id](const detail::TaskData& d) { return d.id == id; }),
                         t->m_tasks.end());
    }
}

void TaskMgr::CancelAllTasks(Type type)
{
    Thread* t = m_threads[m_typeToThreadIdx[type]];
    std::lock_guard<std::mutex> l(t->m_tasksMutex);
    t->m_tasks.clear();
}

void TaskMgr::CancelAllTasks()
{
    for (Thread* t : m_threads) {
        std::lock_guard<std::mutex> l(t->m_tasksMutex);
        t->m_tasks.clear();
    }
}

void TaskMgr::FinishTask(unsigned id)
{
    while (HasTask(id))
        Sleep(1);
}

void TaskMgr::FinishAllTasks()
{
    while (HasTasks())
        Sleep(1);
}

void TaskMgr::FinishAllTasks(Type type)
{
    while (HasTasks(type))
        Sleep(1);
}

void TaskMgr::Sleep(unsigned ms)
{
    if (ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    else
        std::this_thread::yield();
}

void TaskMgr::PreciseSleep(unsigned ms) { Sleep(ms); }

}
}

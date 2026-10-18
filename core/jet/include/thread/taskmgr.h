// core/jet/include/thread/taskmgr.h
#ifndef CORE_JET_INCLUDE_THREAD_TASKMGR_H
#define CORE_JET_INCLUDE_THREAD_TASKMGR_H

#include <memory>
#include <vector>
#include "jenum.h"
#include "jsingleton.h"
#include "thread/thread.h"

namespace jet {
namespace thread {

struct TypeDef {
    enum type { MAIN, RENDERING, STREAMING, SKINNING, USER1, USER2, USER3, USER4, COUNT };
};
typedef safe_enum<TypeDef> Type;

class TaskMgr : public Singleton<TaskMgr> {
public:
    TaskMgr();
    virtual ~TaskMgr();

    unsigned AddTask(const std::shared_ptr<Task>& task, Type type);
    void CancelTask(unsigned id);
    void FinishTask(unsigned id);
    void CancelAllTasks(Type type);
    void CancelAllTasks();
    void FinishAllTasks(Type type);
    void FinishAllTasks();
    bool HasTask(unsigned id) const;
    bool HasTasks(Type type) const;
    bool HasTasks() const;
    void Sleep(unsigned ms);
    void PreciseSleep(unsigned ms);
    unsigned GetCrtThreadIdx() const;
    bool CrtThreadHasType(Type type) const;
    bool ThreadHasType(unsigned idx, Type type) const;

    void Init();

private:
    std::vector<Thread*> m_threads;
    std::vector<unsigned> m_typeToThreadIdx;
    std::vector<unsigned> m_threadIdxToTypes;
    unsigned m_lastId;
    unsigned m_threadCount;
};

}
}

#endif

// libs/glwt2/include/glwebtools/os/glwebtools_taskgroup.h
#ifndef LIBS_GLWT2_INCLUDE_GLWEBTOOLS_OS_GLWEBTOOLS_TASKGROUP_H
#define LIBS_GLWT2_INCLUDE_GLWEBTOOLS_OS_GLWEBTOOLS_TASKGROUP_H

namespace glwebtools {
class TaskGroup {  // sizeof 0x68
public:
    TaskGroup();
    ~TaskGroup();
    int Initialize(const glwebtools::ThreadPool::CreationSettings&);
    void Terminate();
    int Update(unsigned __int64);
    bool IsInitialized();
    int PushTask(unsigned long*, const glwebtools::Job&, int);
    int RemoveTask(unsigned long);
    bool IsScheduled(unsigned long);
    unsigned Size();
    bool Empty();
    bool Completed();
    bool SomeThreadStarted();
private:
    glwebtools::TaskQueue m_tasks;  // 0x0
    glwebtools::ThreadPool m_threadpool;  // 0x18
    glwebtools::Mutex m_mutex;  // 0x58
    unsigned long m_task_count;  // 0x60
};
}

namespace glwebtools {
class TaskQueue {  // sizeof 0x18
public:
    int PushTask(const glwebtools::Task&);
    int Reschedule(std::list<glwebtools::Task,std::allocator<glwebtools::Task> >::_Const_iterator<1>, std::list<glwebtools::Task,std::allocator<glwebtools::Task> >::_Const_iterator<1>);
    int PopTask(glwebtools::Task&);
    int RemoveTask(unsigned long);
    bool IsScheduled(unsigned long);
    void Clear();
    unsigned Size();
    bool Empty();
private:
    std::list<glwebtools::Task,std::allocator<glwebtools::Task> > m_tasks;  // 0x0
};
}

#endif

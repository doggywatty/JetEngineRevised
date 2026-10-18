// source/game/online/tasks/OnlineTask.h
#ifndef SOURCE_GAME_ONLINE_TASKS_ONLINETASK_H
#define SOURCE_GAME_ONLINE_TASKS_ONLINETASK_H

class OnlineTask {  // sizeof 0x40
public:
    OnlineTask(boost::function<void __cdecl(bool,OnlineTask const *)>);
    virtual ~OnlineTask();
    bool IsFinished();
    void DoCallback();
    virtual void Start();
    virtual void Update(int) = 0;
    virtual bool CanBeDeleted();
protected:
    void FinishTask(bool);
    boost::function<void __cdecl(bool,OnlineTask const *)> m_cbOnSuccess;  // 0x8
    bool m_canBeDeleted;  // 0x28
    glwebtools::Mutex m_mutex;  // 0x2C
    OnlineTask::State m_state;  // 0x34
    bool m_success;  // 0x38
};

#endif

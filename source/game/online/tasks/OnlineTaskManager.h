// source/game/online/tasks/OnlineTaskManager.h
#ifndef SOURCE_GAME_ONLINE_TASKS_ONLINETASKMANAGER_H
#define SOURCE_GAME_ONLINE_TASKS_ONLINETASKMANAGER_H

class OnlineTaskManager : public Singleton<OnlineTaskManager> {  // sizeof 0x1C
public:
    OnlineTaskManager();
    virtual ~OnlineTaskManager();
    void Update(const unsigned);
    void StartSyncTask(boost::shared_ptr<OnlineTask>);
    void StartAsyncTask(boost::shared_ptr<OnlineTask>, int);
private:
    glwebtools::Thread *CreateTaskThread(boost::shared_ptr<OnlineTaskManager::TaskData>);
    static void UpdateThread(void*, void*);
    void StartTask(boost::shared_ptr<OnlineTask>, OnlineTaskManager::ONLINE_TASK_TYPE, int);
    bool CanTaskBeDeleted(boost::shared_ptr<OnlineTaskManager::TaskData>);
    std::list<boost::shared_ptr<OnlineTaskManager::TaskData>,std::allocator<boost::shared_ptr<OnlineTaskManager::TaskData> > > m_tasks;  // 0x4
};

#endif

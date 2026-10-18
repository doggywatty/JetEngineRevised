// source/game/online/tasks/InitializeGaiaTask.h
#ifndef SOURCE_GAME_ONLINE_TASKS_INITIALIZEGAIATASK_H
#define SOURCE_GAME_ONLINE_TASKS_INITIALIZEGAIATASK_H

class InitializeGaiaTask : public OnlineTask {  // sizeof 0x60
public:
    InitializeGaiaTask(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, boost::function<void __cdecl(bool,OnlineTask const *)>);
    virtual ~InitializeGaiaTask();
    virtual void Update(int);
private:
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_federationClientId;  // 0x40
};

#endif

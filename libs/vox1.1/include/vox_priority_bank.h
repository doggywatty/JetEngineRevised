// libs/vox1.1/include/vox_priority_bank.h
#ifndef LIBS_VOX1_1_INCLUDE_VOX_PRIORITY_BANK_H
#define LIBS_VOX1_1_INCLUDE_VOX_PRIORITY_BANK_H

namespace vox {
class PriorityBank {  // sizeof 0x30
public:
    PriorityBank(vox::priority_bank::CreationSettings&, vox::PriorityBank*);
    PriorityBank();
    ~PriorityBank();
    const char *GetName();
    bool AddEmitter(vox::EmitterObj*, vox::PriorityBank*, int);
    bool RemoveEmitter(vox::EmitterObj*, bool, bool, bool);
    void Update();
    char* m_name;  // 0x0
    int m_threshold;  // 0x4
    unsigned m_maxPlayback;  // 0x8
    vox::priority_bank::Behaviour m_behaviour;  // 0xC
    vox::PriorityBank *m_parent;  // 0x10
    bool m_overrideChildPriority;  // 0x14
    int m_bankPriority;  // 0x18
    std::vector<vox::PriorityBankElement,vox::SAllocator<vox::PriorityBankElement,0> > m_bankElements;  // 0x1C
};
}

namespace vox {
class PriorityBankManager {  // sizeof 0x18
public:
    PriorityBankManager();
    ~PriorityBankManager();
    unsigned AddPriorityBank(vox::priority_bank::CreationSettings&);
    bool SetPriorityBank(unsigned, vox::priority_bank::CreationSettings&);
    unsigned GetPriorityBankId(const char*);
    bool AddEmitter(unsigned, vox::EmitterObj*);
    bool RemoveEmitter(unsigned, vox::EmitterObj*);
    void Update();
private:
    std::vector<vox::PriorityBank *,vox::SAllocator<vox::PriorityBank *,0> > m_banks;  // 0x0
    vox::Mutex m_mutex;  // 0x14
};
}

#endif

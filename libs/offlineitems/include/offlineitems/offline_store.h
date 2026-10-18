// libs/offlineitems/include/offlineitems/offline_store.h
#ifndef LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OFFLINE_STORE_H
#define LIBS_OFFLINEITEMS_INCLUDE_OFFLINEITEMS_OFFLINE_STORE_H

namespace oi {
class OfflineStore {  // sizeof 0x3C
public:
    int Initialize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static oi::OfflineStore *GetInstance();
    static bool DestroyInstance();
    bool IsInitialized();
    void Update();
    oi::StoreOfflineItemArray *GetOfflineItemList();
    const char *GetPromotionName();
    const char *GetEndDate();
    int Refresh(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int DecryptOfflineItems(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, void*&, long&, unsigned*);
    int EncryptOfflineItems(void*, long, void*&, long&, unsigned*);
private:
    OfflineStore();
    ~OfflineStore();
    int ShutDown();
    int Parse(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool checkMemmory();
    void makeBackup();
    static oi::OfflineStore *s_instance;
    oi::StoreOfflineItemArray *m_items;  // 0x0
    oi::PromotionOI *oi_promotion;  // 0x4
    bool m_isInitialized;  // 0x8
    std::vector<char const *,glwebtools::SAllocator<char const *,4> > m_backup;  // 0xC
    std::vector<double,glwebtools::SAllocator<double,4> > m_backup_numerical;  // 0x20
    bool finishedParsing;  // 0x34
    void* backupData;  // 0x38
};
}

#endif

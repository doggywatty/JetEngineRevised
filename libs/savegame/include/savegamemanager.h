// libs/savegame/include/savegamemanager.h
#ifndef LIBS_SAVEGAME_INCLUDE_SAVEGAMEMANAGER_H
#define LIBS_SAVEGAME_INCLUDE_SAVEGAMEMANAGER_H

namespace savemanager {
class SaveGameManager {  // sizeof 0x78
public:
    SaveGameManager();
    ~SaveGameManager();
    bool Initialize(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    int BeginSave(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int SaveBuffer(const void*, int);
    int EndSave(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (savemanager::OpCode, std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> >&, int, void*)*, void*);
    int BeginLoad(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int LoadBuffer(void*&, int&);
    int EndLoad();
    int GetCloudSaves(int&, std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> >&, bool, void (savemanager::OpCode, std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> >&, int, void*)*, void*);
private:
    int RestoreCloudSave(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const gaia::GLUID&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (savemanager::OpCode, std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> >&, int, void*)*, void*);
public:
    int RestoreCloudSave(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const gaia::BaseServiceManager::Credentials, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (savemanager::OpCode, std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> >&, int, void*)*, void*);
    int RestoreCloudSave(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, savemanager::CloudSave*, bool, void (savemanager::OpCode, std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> >&, int, void*)*, void*);
    int RestoreCustomerCareSave(const Json::Value&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, bool, void (savemanager::OpCode, std::vector<savemanager::CloudSave *,std::allocator<savemanager::CloudSave *> >&, int, void*)*, void*);
    int AcceptSaveRestore(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int RejectSaveRestore();
    int GenerateCustomerCareLink(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static savemanager::SaveGameManager *GetInstance();
    static void Release();
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetSaveFilePath(const char*);
    int ResetCloud();
    gaia::BaseServiceManager::Credentials GetCredentialTypeUsedForBackup();
    int SetCredentialTypeUsedForBackup(gaia::BaseServiceManager::Credentials);
private:
    int GetFreeSpaceAvailableOnSeshat(int&, int&, int&);
    int UploadTableOfContents(const Json::Value&, gaia::BaseServiceManager::Credentials);
    long StringToUnixTimestamp(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int SaveBufferWithGLUID(const void*, int, const gaia::GLUID&);
    int LoadBufferWithGLUID(void*&, int&, const gaia::GLUID&);
    static long GetUNIXTimeStamp();
    int UploadSaveToCloud(savemanager::CloudSave*);
    static void PerformAsyncAction(void*, void*);
    int RetrieveTableOfContents(Json::Value&, const gaia::BaseServiceManager::Credentials);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_saveGameFileName;  // 0x0
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_backupSaveGameFileName;  // 0x20
    static savemanager::SaveGameManager *s_instance;
    savemanager::CloudSave *m_saveGameObj;  // 0x40
    gaia::Gaia *m_gaia;  // 0x44
    _iobuf *m_SaveFile;  // 0x48
    _iobuf *m_LoadFile;  // 0x4C
    int m_NoOfBufferFromSaveFile;  // 0x50
    int m_NoOfCurrentLoadedBuffer;  // 0x54
    bool m_preventFromSaving;  // 0x58
    gaia::BaseServiceManager::Credentials m_accountTypeToUseForBackup;  // 0x5C
    bool m_IsWritingSavegame;  // 0x60
    bool m_IsReadingSavegame;  // 0x61
    bool m_isInitialized;  // 0x62
    glwebtools::Thread *m_threadLoadSaves;  // 0x64
    glwebtools::Thread *m_threadUploadSave;  // 0x68
    glwebtools::Thread *m_threadRestoreSave;  // 0x6C
    glwebtools::Mutex m_mutex;  // 0x70
};
}

#endif

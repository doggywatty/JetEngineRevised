// libs/savegame/include/cloudsave.h
#ifndef LIBS_SAVEGAME_INCLUDE_CLOUDSAVE_H
#define LIBS_SAVEGAME_INCLUDE_CLOUDSAVE_H

namespace savemanager {
class CloudSave {  // sizeof 0x9C
public:
    CloudSave(const savemanager::CloudSave&);
    CloudSave();
    ~CloudSave();
    void SetTimeStamp(long);
    void SetDescription(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void SetSaveVersion(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void AddData(const savemanager::SaveData&);
    void SetGLUID(const gaia::GLUID&);
    void SetSeshatFileKey(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    gaia::BaseServiceManager::Credentials GetCredentialUsedForUpload();
    void SetCredentialUsedForUpload(gaia::BaseServiceManager::Credentials);
    long GetTimeStamp();
    savemanager::SaveData &GetData();
    gaia::GLUID GetGLUID();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetSeshatFileKey();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetSaveVersion();
    const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > &GetAllSeshatFileKeys();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetDescription();
    unsigned GetDataLength();
    savemanager::CloudSave &operator=(const savemanager::CloudSave&);
private:
    long m_timeStamp;  // 0x0
    gaia::GLUID m_GLUID;  // 0x4
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_seshatFileKeys;  // 0x34
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_description;  // 0x48
    savemanager::SaveData m_saveData;  // 0x68
    long m_saveDataIdx;  // 0x70
    int m_noOfBufferFromSaveFile;  // 0x74
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_saveVersion;  // 0x78
    gaia::BaseServiceManager::Credentials m_credential;  // 0x98
};
}

#endif

// libs/manhattan/include/dlc/assetmgrsettings.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_ASSETMGRSETTINGS_H
#define LIBS_MANHATTAN_INCLUDE_DLC_ASSETMGRSETTINGS_H

namespace manhattan::dlc {
class AssetMgrSettings {  // sizeof 0xC0
protected:
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetIndexBaseName(int);
public:
    AssetMgrSettings(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int, manhattan::dlc::Downloader&, manhattan::dlc::Downloader&);
    virtual ~AssetMgrSettings();
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > &GetFolder();
protected:
    manhattan::dlc::ManhattanInstallerPerformanceSettings m_performanceSettings;  // 0x4
    const int m_dlcRefreshPeriod;  // 0x10
    const int m_updatePeriod;  // 0x14
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_rootFolder;  // 0x18
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_platformStr;  // 0x38
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_versionStr;  // 0x58
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_definitionStr;  // 0x78
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_indexFileName;  // 0x98
    manhattan::dlc::Downloader &m_downloaderBackGround;  // 0xB8
    manhattan::dlc::Downloader &m_downloaderOnDemand;  // 0xBC
};
}

#endif

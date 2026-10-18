// libs/manhattan/include/dlc/assetmgr2.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_ASSETMGR2_H
#define LIBS_MANHATTAN_INCLUDE_DLC_ASSETMGR2_H

namespace manhattan::dlc {
class AssetMgr2 : public manhattan::dlc::DetailsHandler<manhattan::dlc::AssetMgrDetails>, public manhattan::dlc::AssetMgrSettings {  // sizeof 0x300
    void CheckAssets();
    void RemoveOldAssets();
    void ComputeDiffAssets();
    void BuildFeedback();
    manhattan::dlc::EAssetMgrState UpdateState(manhattan::dlc::EAssetMgrState, manhattan::dlc::EManhattanInstallerState);
    void CheckAsset(const std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >&, bool);
    manhattan::dlc::DetailsHandler<manhattan::dlc::InstallTaskDetails> Install(const manhattan::dlc::InstallRequestUrl&);
    manhattan::dlc::DetailsHandler<manhattan::dlc::InstallTaskDetails> Install(const manhattan::dlc::InstallRequestIrisAsset&);
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > > BuildAssetsListToDownload(const std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > >&);
    void CheckNewInstall(manhattan::dlc::ManhattanInstaller&, std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > >&, bool);
    bool CheckLzmaIntegrity(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const manhattan::dlc::AssetFeedbackProvider &GetFeedbackProvider();
    manhattan::dlc::AssetFeedback RequestNonCompressedIrisFile(manhattan::dlc::ManhattanInstaller&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    manhattan::dlc::AssetFeedback RequestAsset(manhattan::dlc::ManhattanInstaller&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    manhattan::dlc::AssetFeedback RequestUrl(manhattan::dlc::ManhattanInstaller&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool CreateRootFolder();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetFormatedAssetName(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    void ChangeState(manhattan::dlc::EAssetMgrState);
    bool IsAssetOnAssetList(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > >&);
    manhattan::dlc::AssetFeedback AcknowledgeAsInactive(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    manhattan::dlc::InstallTaskDetails BuildInstallDetails(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    AssetMgr2(const manhattan::dlc::AssetMgrSettings&);
    virtual ~AssetMgr2();
    void Update(int);
    void Update();
    bool RequestAssetOnDemand(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetFolder();
    bool IsAssetMandatory(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool IsAssetOnDemand(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool IsAssetIndexed(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    bool IsAssetActive(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    int GetIndex(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::map<int,manhattan::dlc::InstallTaskDetails,std::less<int>,std::allocator<std::pair<int const ,manhattan::dlc::InstallTaskDetails> > > &GetAllAssetDetails();
    const manhattan::dlc::InstallTaskDetails GetAssetDetails(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const manhattan::dlc::InstallTaskDetails GetAssetDetails(int);
    const manhattan::dlc::AssetFeedback GetFeedback(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    const std::vector<manhattan::dlc::AssetFeedback,std::allocator<manhattan::dlc::AssetFeedback> > GetFeedback();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetIrisName(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetFileName(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    manhattan::dlc::InstallTaskStatisticsTotal GetInstallDetails(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    manhattan::dlc::InstallTaskStatisticsTotal GetInstallDetails(const std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > >&);
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > GetMandatoryAssets();
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > GetIndexedNonMandatoryAssets();
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > GetOnDemandAssets();
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > GetRequestedOnDemandAssets();
    bool IsReadyForRequests();
private:
    bool m_removeOldAssets;  // 0xCC
    bool m_downloadIndexedAssets;  // 0xCD
    bool m_verifyHashes;  // 0xCE
    bool m_parallelizeDecompression;  // 0xCF
    unsigned __int64 m_lastUpdate;  // 0xD0
    unsigned __int64 m_lastDlcUpdate;  // 0xD8
    manhattan::dlc::AssetFeedbackProvider m_assetFeedbackProvider;  // 0xE0
    manhattan::dlc::ManhattanInstaller m_installerBackground;  // 0x108
    manhattan::dlc::ManhattanInstaller m_installerOnDemand;  // 0x170
    glwebtools::GlWebTools m_glwtInstance;  // 0x1D8
    manhattan::dlc::AssetFeedback m_indexInstall;  // 0x1DC
    manhattan::dlc::AssetFeedback m_tocInstall;  // 0x1E8
    manhattan::dlc::AssetFeedback m_hashesInstall;  // 0x1F4
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_tocFileName;  // 0x200
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_hashesFileName;  // 0x220
    Json::Value m_TOC;  // 0x240
    Json::Value m_hashes;  // 0x250
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > > m_onDemandAssetsFromTOC;  // 0x260
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > > m_onDemandAssetsToQueue;  // 0x274
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > > m_indexedAssetsFromTOC;  // 0x288
    std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,int> > > > m_indexedAssetsToQueue;  // 0x29C
    std::vector<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_onDemandAssetsEverQueued;  // 0x2B0
    std::set<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > m_requestedOnDemandAssets;  // 0x2C4
    std::map<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::less<std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> > const ,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > m_irisAssetToNameMap;  // 0x2E0
};
}

#endif

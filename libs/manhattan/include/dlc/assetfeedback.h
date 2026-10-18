// libs/manhattan/include/dlc/assetfeedback.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_ASSETFEEDBACK_H
#define LIBS_MANHATTAN_INCLUDE_DLC_ASSETFEEDBACK_H

namespace manhattan::dlc {
class AssetFeedback {  // sizeof 0xC
    const manhattan::dlc::AssetFeedbackProvider &GetFeedbackProvider();
public:
    AssetFeedback(int, const manhattan::dlc::AssetMgr2&);
    AssetFeedback(const manhattan::dlc::AssetFeedback&);
    virtual ~AssetFeedback();
    int GetId();
    manhattan::dlc::DownloadTaskStatistics &GetDownloadStatistics();
    manhattan::dlc::DecoderStatistics &GetDecoderStatistics();
    const manhattan::dlc::StateMachine<enum manhattan::dlc::ETaskState,enum manhattan::dlc::EErrorState> &GetDownloadState();
    const manhattan::dlc::StateMachine<enum manhattan::dlc::ETaskState,enum manhattan::dlc::EErrorState> &GetDecoderState();
    double GetProgress();
    bool IsExecuting();
    bool IsErrorState();
    bool IsValid();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > ToString();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetName();
    manhattan::dlc::AssetFeedback &operator=(const manhattan::dlc::AssetFeedback&);
private:
    int m_id;  // 0x4
    const manhattan::dlc::AssetMgr2 &m_assetMgr;  // 0x8
};
}

#endif

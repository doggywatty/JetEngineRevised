// libs/manhattan/include/dlc/irisassetdownloadtask.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_IRISASSETDOWNLOADTASK_H
#define LIBS_MANHATTAN_INCLUDE_DLC_IRISASSETDOWNLOADTASK_H

namespace manhattan::dlc {
class IrisAssetDownloadTask : public manhattan::dlc::FileDownloadTask {  // sizeof 0x10C
public:
    IrisAssetDownloadTask(glwebtools::GlWebTools*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
    virtual ~IrisAssetDownloadTask();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetId();
    int GetChunkStart();
    int GetChunkEnd();
    virtual void Update();
private:
    void Init();
    static void PrivateRun(void*, void*);
public:
    manhattan::dlc::SharedFile &GetSharedFile();
    virtual bool Close();
    bool HasFinished();
private:
    manhattan::dlc::SharedFile m_sharedFile;  // 0x74
    manhattan::dlc::SharedFilePointer m_sharedFilePointer;  // 0xB0
    const std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_assetName;  // 0xBC
    int m_chunkStart;  // 0xDC
    int m_chunkEnd;  // 0xE0
    manhattan::dlc::IDownloadTask::IObserver *m_observer;  // 0xE4
    int m_chunkSize;  // 0xE8
    bool m_wasInitialized;  // 0xEC
    bool m_wasDownloadStarted;  // 0xED
    bool m_wasCanceled;  // 0xEE
    int m_progress;  // 0xF0
    int m_progressFromByteZero;  // 0xF4
    int toOffset;  // 0xF8
    int fromOffset;  // 0xFC
    int numChunks;  // 0x100
    int chunkEnd;  // 0x104
    int curChunk;  // 0x108
};
}

#endif

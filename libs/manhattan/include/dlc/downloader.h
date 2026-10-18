// libs/manhattan/include/dlc/downloader.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_DOWNLOADER_H
#define LIBS_MANHATTAN_INCLUDE_DLC_DOWNLOADER_H

namespace manhattan::dlc {
class Downloader : public manhattan::dlc::IDownloader {  // sizeof 0x28
public:
    Downloader();
    virtual ~Downloader();
    virtual void RemoveCurrentTask();
    virtual void PushDownload(manhattan::dlc::IDownloadTask*);
    virtual void Update();
    virtual int GetNumPendingDownloads();
    virtual const manhattan::dlc::IDownloadTask *GetCurrentTask();
    virtual bool HasDownloadTask(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    virtual void CancelAllDownloadTasks();
private:
    glwebtools::Mutex m_mutex;  // 0x4
    std::deque<manhattan::dlc::IDownloadTask *,std::allocator<manhattan::dlc::IDownloadTask *> > m_downloadQueue;  // 0xC
};
}

#endif

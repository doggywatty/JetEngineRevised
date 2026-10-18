// libs/manhattan/include/dlc/filedownloadtask.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_FILEDOWNLOADTASK_H
#define LIBS_MANHATTAN_INCLUDE_DLC_FILEDOWNLOADTASK_H

namespace manhattan::dlc {
class FileDownloadTask : public manhattan::dlc::IDownloadTask, public manhattan::dlc::DetailsHandler<manhattan::dlc::DownloadTaskDetails> {  // sizeof 0x74
protected:
    void SetUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
public:
    FileDownloadTask(glwebtools::GlWebTools*, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, manhattan::dlc::BaseWriter*);
    virtual ~FileDownloadTask();
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetId();
    virtual void Cancel();
    virtual void Update();
    virtual bool HasFinished();
    virtual bool Close();
    void SetId(std::basic_string<char,std::char_traits<char>,std::allocator<char> >);
    void SetContentLengthAsExpectedSize();
protected:
    virtual std::vector<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > >,std::allocator<std::pair<std::basic_string<char,std::char_traits<char>,std::allocator<char> >,std::basic_string<char,std::char_traits<char>,std::allocator<char> > > > > GetHeaders();
private:
    bool StartDownload();
    manhattan::dlc::ETaskState UpdateState(manhattan::dlc::ETaskState, manhattan::dlc::ETaskState);
public:
    void SetObserver(manhattan::dlc::IDownloadTask::IObserver*);
private:
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_id;  // 0x10
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_downloadURL;  // 0x30
    manhattan::dlc::IDownloadTask::IObserver *m_observer;  // 0x50
    manhattan::dlc::BaseWriter *m_customWriter;  // 0x54
    glwebtools::GlWebTools *m_glwtInstance;  // 0x58
    glwebtools::UrlConnection m_connection;  // 0x5C
    int m_responseCode;  // 0x64
    char* m_downloadedData;  // 0x68
    unsigned m_downloadedDataSize;  // 0x6C
    bool m_wasDownloadStarted;  // 0x70
    bool m_wasCanceled;  // 0x71
};
}

#endif

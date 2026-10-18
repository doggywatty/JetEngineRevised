// source/game/online/pollers/AdServerPoller.h
#ifndef SOURCE_GAME_ONLINE_POLLERS_ADSERVERPOLLER_H
#define SOURCE_GAME_ONLINE_POLLERS_ADSERVERPOLLER_H

class AdServerPoller : public Singleton<AdServerPoller> {  // sizeof 0x4C
public:
    AdServerPoller();
    virtual ~AdServerPoller();
    void Update(int);
    void CheckForRewards();
    void Redownload();
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetUrl(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    void HandleRetrieve(int, const glwebtools::UrlResponse&);
    void HandleClear(int, const glwebtools::UrlResponse&);
    int StringToInt(const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&, int);
    void SendRequest(int, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
    static const int RETRY_TIME_MSECS;
    static const int POLLING_FREQ_MSECS;
    int m_elapsedTime;  // 0x4
    glwebtools::UrlRequest m_currentRequest;  // 0x8
    std::basic_string<char,std::char_traits<char>,std::allocator<char> > m_downloadedJson;  // 0x10
    glwebtools::GlWebTools *m_glwtInstance;  // 0x30
    static std::basic_string<char,std::char_traits<char>,std::allocator<char> > s_pollURLs[4];
    static bool s_URLInitialization;
    static bool s_mustCheckForRewards;
    bool m_mustCheckForRewards;  // 0x34
    std::vector<std::pair<int,glwebtools::UrlConnection>,std::allocator<std::pair<int,glwebtools::UrlConnection> > > m_connections;  // 0x38
};

#endif

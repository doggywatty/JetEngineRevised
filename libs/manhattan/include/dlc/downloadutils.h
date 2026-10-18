// libs/manhattan/include/dlc/downloadutils.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_DOWNLOADUTILS_H
#define LIBS_MANHATTAN_INCLUDE_DLC_DOWNLOADUTILS_H

namespace manhattan::dlc {
class DownloadUtils {  // sizeof 0x1
public:
    static int GetContentLength(glwebtools::GlWebTools*, glwebtools::UrlConnection&, const std::basic_string<char,std::char_traits<char>,std::allocator<char> >&);
private:
    DownloadUtils(const manhattan::dlc::DownloadUtils&);
    DownloadUtils();
    manhattan::dlc::DownloadUtils &operator=(const manhattan::dlc::DownloadUtils&);
};
}

#endif

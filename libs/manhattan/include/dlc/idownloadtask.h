// libs/manhattan/include/dlc/idownloadtask.h
#ifndef LIBS_MANHATTAN_INCLUDE_DLC_IDOWNLOADTASK_H
#define LIBS_MANHATTAN_INCLUDE_DLC_IDOWNLOADTASK_H

namespace manhattan::dlc {
class IDownloadTask {  // sizeof 0x4
public:
    virtual std::basic_string<char,std::char_traits<char>,std::allocator<char> > GetId() = 0;
    virtual void Cancel() = 0;
    virtual void Update() = 0;
    virtual bool HasFinished() = 0;
    virtual bool Close() = 0;
    virtual ~IDownloadTask();
};
}

#endif

// framework/include/common/utils/timeutils.h
#ifndef FRAMEWORK_INCLUDE_COMMON_UTILS_TIMEUTILS_H
#define FRAMEWORK_INCLUDE_COMMON_UTILS_TIMEUTILS_H

class TimeUtils {  // sizeof 0x1
public:
    static unsigned GetTimeInSeconds();
    static unsigned GetServerTimeInSeconds();
    static unsigned GetTimeDifference(unsigned, unsigned);
    static __int64 GetTimeInMillis();
    static void SetMarker();
    static int GetMilliFromMarker();
    static tm GetCurrentDate();
    static void Sleep(int);
    static const unsigned k_MINUTE;
    static const unsigned k_HOUR;
    static const unsigned k_DAY;
    static __int64 s_markerTime;
};

#endif

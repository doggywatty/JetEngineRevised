// libs/glsociallib/include/connectionchecker/glsociallib_connectionchecker.h
#ifndef LIBS_GLSOCIALLIB_INCLUDE_CONNECTIONCHECKER_GLSOCIALLIB_CONNECTIONCHECKER_H
#define LIBS_GLSOCIALLIB_INCLUDE_CONNECTIONCHECKER_GLSOCIALLIB_CONNECTIONCHECKER_H

namespace sociallib {
class GLSocialLib_ConnectionChecker {  // sizeof 0x1
public:
    bool isInternetConnectionPresent();
    bool isLocalWifiConnectionPresent();
};
}

#endif

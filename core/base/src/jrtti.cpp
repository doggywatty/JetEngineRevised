// core/base/src/jrtti.cpp
#include "jrtti.h"
#include <atomic>

namespace jet {
namespace rtti {

int NewClassId()
{
    static std::atomic<int> next(1);
    return next++;
}

}
}

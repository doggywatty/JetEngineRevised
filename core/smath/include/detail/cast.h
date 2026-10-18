// core/smath/include/detail/cast.h
#ifndef CORE_SMATH_INCLUDE_DETAIL_CAST_H
#define CORE_SMATH_INCLUDE_DETAIL_CAST_H

#include "quat.h"

namespace math {

template <class To, class From>
To cast(const From& f) { return To(f); }

}

#endif

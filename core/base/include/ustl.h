// core/base/include/ustl.h
//
// `ustl::vector<T>` is used across ~25 headers in source/game (Menu_Pause.h and several
// subpages/*.h at least) but was never declared anywhere -- no header for it survived the PDB
// extraction this project is based on. It was presumably a small/embedded-friendly vector
// implementation (a "micro STL", common in mobile game SDKs of this era) with std::vector-
// compatible semantics for the operations this codebase actually uses (push_back, size, indexing,
// iteration, clear). std::vector is a safe, behavior-preserving stand-in for those operations --
// this is a minimal compilation bridge, not a guess at control-flow-affecting logic.
#ifndef CORE_BASE_INCLUDE_USTL_H
#define CORE_BASE_INCLUDE_USTL_H

#include <vector>

namespace ustl {
template <class T>
using vector = std::vector<T>;
}

#endif

#pragma once

#include <cstdlib>

namespace th06 {
namespace ZunMemory {
inline void *Alloc(size_t size) { return std::malloc(size); }

inline void Free(void *ptr) { std::free(ptr); }
}; // namespace ZunMemory
}; // namespace th06

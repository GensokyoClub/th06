#pragma once

#include <cstdlib>

namespace ZunMemory {
    inline void *Alloc(size_t size) {
        return std::malloc(size);
    }

    inline void Free(void *ptr) {
        std::free(ptr);
    }
}; // namespace ZunMemory

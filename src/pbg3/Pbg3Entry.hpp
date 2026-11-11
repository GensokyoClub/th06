#pragma once
#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
struct Pbg3Entry
{
    FILETIME lastWriteTime;
    u32 uncompressedSize;
    u32 dataOffset;
    u32 checksum;
    char filename[256];
};
ZUN_ASSERT_SIZE(Pbg3Entry, 0x114);
} // namespace th06

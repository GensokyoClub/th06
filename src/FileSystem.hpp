#pragma once

#include "inttypes.hpp"

namespace th06
{
namespace FileSystem
{
u8 *OpenPath(const char *filepath);
int WriteDataToFile(const char *path, void *data, std::size_t size);
} // namespace FileSystem

extern u32 g_LastFileSize;
}; // namespace th06

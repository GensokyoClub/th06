#pragma once

#include "ZunResult.hpp"
#include "inttypes.hpp"
#include <cstdio>

namespace th06
{
namespace FileSystem
{
// Wrapper to open a UTF-8 path string on systems that might not have first-class UTF-8 support (WINDOWS!!!)
// This should probably be moved to proper filesystem abstraction interface if / when
//   std::filesystem needs to be replaced for portability to systems that might not have
//   proper C++ standard library support
FILE *FopenUTF8(const char *filepath, const char *mode);
u8 *OpenPath(const char *filepath, int isExternalResource);
int WriteDataToFile(const char *path, void *data, std::size_t size);
} // namespace FileSystem
extern u32 g_LastFileSize;
}; // namespace th06

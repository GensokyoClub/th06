#pragma once
#include "decomp.hpp"
#include "pbg3/Pbg3Reader.hpp"

namespace th06
{
struct Pbg3Entry
{
    u32 unk1;
    u32 unk2;
    u32 uncompressedSize;
    u32 dataOffset;
    u32 checksum;
    char filename[256];
};
ZUN_ASSERT_SIZE(Pbg3Entry, 0x114);

class Pbg3Archive
{
  public:
    BOOL Release();

    BOOL Load(const char *path);
    BOOL LoadR(HMODULE module, const char *name);
    BOOL ParseHeader();
    i32 FindEntry(const char *path);
    u32 GetEntrySize(u32 entryIdx);
    u32 GetEntrySize(const char *path);
    u8 *ReadEntryRaw(u32 *outSize, u32 *outChecksum, i32 entryIdx);
    u8 *ReadDecompressEntry(u32 entryIdx, const char *filename);

    Pbg3Archive();
    ~Pbg3Archive();

  private:
    IPbg3Reader *reader;
    void *resData;
    u32 numOfEntries;
    u32 fileTableOffset;
    Pbg3Entry *entries;
};
ZUN_ASSERT_SIZE(Pbg3Archive, 0x14);
} // namespace th06

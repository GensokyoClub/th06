#pragma once
#include "decomp.hpp"
#include "pbg3/Pbg3Entry.hpp"
#include "pbg3/Pbg3Reader.hpp"

namespace th06
{
class Pbg3Archive
{
  public:
    BOOL Release();

    BOOL Load(const char *path);
    BOOL LoadR(HMODULE module, const char *name);
    BOOL ParseHeader();
    i32 FindEntry(const char *path);
    u32 GetEntrySize(u32 entryIdx);
    u32 GetEntrySize(const char *path)
    {
        return this->GetEntrySize(this->FindEntry(path));
    }
    u32 GetNumFiles() const
    {
        return this->numOfEntries;
    }
    const char *GetFileID(u32 entryIdx) const
    {
        if (entryIdx >= this->numOfEntries)
        {
            return NULL;
        }
        return this->entries[entryIdx].filename;
    }
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

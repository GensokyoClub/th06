#pragma once
#include "AnmVm.hpp"
#include "Chain.hpp"

#include "ZunResult.hpp"
#include "decomp.hpp"

namespace th06
{
struct TrackDescriptor
{
    TrackDescriptor()
    {
        memset(this, 0, sizeof(TrackDescriptor));
    }

    char path[64];
    char title[34];
    char description[8][66];
};
ZUN_ASSERT_TYPE(TrackDescriptor, 0x272, 1);

struct MusicRoom
{
    MusicRoom()
    {
        memset(this, 0, sizeof(MusicRoom));
    }

    BOOL ProcessInput();
    ZunResult CheckInputEnable();

    ChainElem *calcChain;
    ChainElem *drawChain;
    i32 waitFramesCount;
    BOOL enableInput;
    i32 cursor;
    i32 selectedSongIndex;
    i32 listingOffset;
    i32 numDescriptors;
    TrackDescriptor *trackDescriptors;
    AnmVm mainVm[1];        // music00.anm
    AnmVm titleSprites[32]; // music01.anm + music02.anm
    AnmVm descriptionSprites[16];
};
ZUN_ASSERT_TYPE(MusicRoom, 0x3434, 4);

ZunResult MusicRoom_RegisterChain();

} // namespace th06

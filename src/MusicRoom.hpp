#pragma once
#include "AnmVm.hpp"
#include "Chain.hpp"
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

namespace th06
{
ZunResult MusicRoom_RegisterChain();
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

    ZunBool ProcessInput();
    ZunResult CheckInputEnable();

    ChainElem *calc_chain;
    ChainElem *draw_chain;
    i32 waitFramesCount;
    ZunBool enableInput;
    i32 cursor;
    i32 selectedSongIndex;
    i32 listingOffset;
    i32 numDescriptors;
    TrackDescriptor *trackDescriptors;
    AnmVm mainVm[1];
    AnmVm titleSprites[32];
    AnmVm descriptionSprites[16];
};
ZUN_ASSERT_TYPE(MusicRoom, 0x3434, 4);

} // namespace th06

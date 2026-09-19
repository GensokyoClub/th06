#pragma once

#include "AnmVm.hpp"
#include "Chain.hpp"
#include "ZunResult.hpp"
#include "inttypes.hpp"

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
ZUN_ASSERT_SIZE(TrackDescriptor, 0x272);

struct MusicRoom
{
    MusicRoom()
    {
        memset(this, 0, sizeof(MusicRoom));
    }

    static ZunResult AddedCallback(MusicRoom *musicRoom);
    static ZunResult DeletedCallback(MusicRoom *musicRoom);
    ZunBool ProcessInput();
    ZunResult CheckInputEnable();
    static ChainCallbackResult OnDraw(MusicRoom *musicRoom);
    static ChainCallbackResult OnUpdate(MusicRoom *musicRoom);
    static ZunResult RegisterChain();

    ChainElem *calc_chain;
    ChainElem *draw_chain;
    i32 waitFramesCount;
    i32 enableInput;
    i32 cursor;
    i32 selectedSongIndex;
    i32 listingOffset;
    i32 numDescriptors;
    TrackDescriptor *trackDescriptors;
    AnmVm mainVm[1];
    AnmVm titleSprites[32];
    AnmVm descriptionSprites[16];
};
ZUN_ASSERT_SIZE(MusicRoom, 0x3434);
}; // namespace th06

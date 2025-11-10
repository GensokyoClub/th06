#pragma once

#include "AnmVm.hpp"
#include "Chain.hpp"
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

// Force constructor to generate size optimized code when it's placed in the binary

struct MusicRoom
{
    MusicRoom()
    {
        memset(this, 0, sizeof(MusicRoom));
    }

    static bool AddedCallback(MusicRoom *musicRoom);
    static bool DeletedCallback(MusicRoom *musicRoom);
    bool ProcessInput();
    bool CheckInputEnable();
    static ChainCallbackResult OnDraw(MusicRoom *musicRoom);
    static ChainCallbackResult OnUpdate(MusicRoom *musicRoom);
    static bool RegisterChain();

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

#pragma once
#include "decomp.hpp"

namespace th06
{
struct ReplayDataInput
{
    i32 frameNum;
    u16 inputKey;
    alignment_padding(0x2);
};
ZUN_ASSERT_TYPE(ReplayDataInput, 0x8, 4);

struct StageReplayData
{
    i32 score;
    i16 randomSeed;
    i16 pointItemsCollected;
    u8 power;
    i8 livesRemaining;
    i8 bombsRemaining;
    u8 rank;
    i8 fullPowerScoreIdx;
    alignment_padding(0x3);
    ReplayDataInput replayInputs[];
};
ZUN_ASSERT_TYPE(StageReplayData, 0x10, 4);

struct ReplayData
{
    char magic[4];
    u16 version;
    u8 shottypeChara;
    u8 difficulty;
    i32 checksum;
    u8 rngValue1;
    u8 rngValue2;
    i8 key;
    i8 rngValue3;
    char date[9];
    char name[8];
    alignment_padding(0x3);
    i32 score;
    f32 slowdownRate2;
    f32 slowdownRate;
    f32 slowdownRate3;
    StageReplayData *stageReplayData[7];
};
ZUN_ASSERT_TYPE(ReplayData, 0x50, 4);
} // namespace th06

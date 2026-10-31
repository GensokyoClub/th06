#pragma once
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "ReplayData.hpp"
#include "ZunBool.hpp"
#include "decomp.hpp"

namespace th06
{
#define REPLAY_MAGIC "T6RP"

ZunResult ReplayManager_RegisterChain(ZunBool isDemo, const char *replayFile);

void StopRecordingReplay();
void SaveReplay(const char *replayPath, const char *replayName);
ZunResult ValidateReplayData(ReplayData *data, i32 fileSize);

// Recording reserves this many bytes; saved stages contain only the used input records.
#define STAGE_REPLAY_BUFFER_SIZE 0x69780

struct ReplayManager
{
    ReplayManager()
    {
    }

    ZunBool IsDemo()
    {
        return this->isDemo;
    }

    i32 frameId;
    ReplayData *replayData;
    ZunBool isDemo;
    const char *replayFile;
    unreferenced_fields(0x34);
    u16 unk44;
    alignment_padding(0x2);
    ReplayDataInput *replayInputs;
    ReplayDataInput *replayInputStageBookmarks[7];
    ChainElem *calcChain;
    ChainElem *drawChain;
    ChainElem *calcChainDemoHighPrio;
};
ZUN_ASSERT_TYPE(ReplayManager, 0x74, 4);

} // namespace th06

#pragma once

#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "ReplayData.hpp"
#include "inttypes.hpp"

namespace th06
{
struct ReplayManager
{
    static bool RegisterChain(i32 isDemo, const char *replayFile);
    static ChainCallbackResult OnUpdate(ReplayManager *mgr);
    static ChainCallbackResult OnUpdateDemoHighPrio(ReplayManager *mgr);
    static ChainCallbackResult OnUpdateDemoLowPrio(ReplayManager *mgr);
    static ChainCallbackResult OnDraw(ReplayManager *mgr);
    static bool AddedCallback(ReplayManager *mgr);
    static bool AddedCallbackDemo(ReplayManager *mgr);
    static bool DeletedCallback(ReplayManager *mgr);
    static void StopRecording();
    static void SaveReplay(char *replay_path, char *param_2);
    static bool ValidateReplayData(ReplayHeader *data, i32 fileSize);

    ReplayManager()
    {
    }

    i32 IsDemo()
    {
        return this->isDemo;
    }

    i32 frameId;
    ReplayData *replayData;
    i32 isDemo;
    const char *replayFile;
    u8 unk10[52];
    u16 unk44;
    ReplayDataInput *replayInputs;
    ReplayDataInput *replayInputStageBookmarks[7];
    ChainElem *calcChain;
    ChainElem *drawChain;
    ChainElem *calcChainDemoHighPrio;
};
}; // namespace th06

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "GameManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "ReplayManager.hpp"
#include "Supervisor.hpp"
#include "ZunTimer.hpp"

namespace th06
{
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

AUTO_BSS_SORT(P1);
ReplayManager *g_ReplayManager;

#define TH_BUTTON_REPLAY_CAPTURE                                                                                       \
    (TH_BUTTON_SHOOT | TH_BUTTON_BOMB | TH_BUTTON_FOCUS | TH_BUTTON_SKIP | TH_BUTTON_DIRECTION)

static ChainCallbackResult ReplayManager_OnUpdate(ReplayManager *mgr)
{
    if (!g_GameManager.isInMenu)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    u16 inputs = IS_PRESSED(TH_BUTTON_REPLAY_CAPTURE);
    if (inputs != mgr->replayInputs->inputKey)
    {
        mgr->replayInputs++;
        mgr->replayInputStageBookmarks[g_GameManager.currentStage - 1] = mgr->replayInputs + 1;
        mgr->replayInputs->frameNum = mgr->frameId;
        mgr->replayInputs->inputKey = inputs;
    }
    mgr->frameId++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ReplayManager_OnUpdateDemoLowPrio(ReplayManager *mgr)
{
    if (!g_GameManager.isInMenu)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (g_Gui.HasCurrentMsgIdx() && g_Gui.IsDialogueSkippable() && mgr->frameId % 3 != 2)
    {
        return CHAIN_CALLBACK_RESULT_RESTART_FROM_FIRST_JOB;
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ReplayManager_OnUpdateDemoHighPrio(ReplayManager *mgr)
{
    if (!g_GameManager.isInMenu)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    while (mgr->frameId >= mgr->replayInputs[1].frameNum)
    {
        mgr->replayInputs++;
    }
    g_CurFrameInput = IS_PRESSED(0xFFFFFFFF & ~TH_BUTTON_REPLAY_CAPTURE) | mgr->replayInputs->inputKey;
    g_IsEigthFrameOfHeldInput = false;
    if (g_LastFrameInput == g_CurFrameInput)
    {
        if (g_NumOfFramesInputsWereHeld >= 30)
        {
            if (g_NumOfFramesInputsWereHeld % 8 == 0)
            {
                g_IsEigthFrameOfHeldInput = true;
            }
            if (g_NumOfFramesInputsWereHeld >= 38)
            {
                g_NumOfFramesInputsWereHeld = 30;
            }
        }
        g_NumOfFramesInputsWereHeld++;
    }
    else
    {
        g_NumOfFramesInputsWereHeld = 0;
    }
    mgr->frameId++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult ReplayManager_OnDraw(ReplayManager *mgr)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(idx, decryptedData, obfOffset, obfuscateCursor, checksum, checksumCursor)
ZunResult ValidateReplayData(ReplayData *data, i32 fileSize)
{
    u8 *checksumCursor;
    u32 checksum;
    u8 *obfuscateCursor;
    u8 obfOffset;
    i32 idx;

    ReplayData *decryptedData = data;

    if (decryptedData == NULL)
    {
        return ZUN_ERROR;
    }

    if (*(u32 *)decryptedData->magic != *(u32 *)REPLAY_MAGIC)
    {
        return ZUN_ERROR;
    }

    /* Deobfuscate the replay decryptedData */
    obfuscateCursor = (u8 *)&decryptedData->rngValue3;
    obfOffset = decryptedData->key;
    for (idx = 0; idx < fileSize - (i32)offsetof(ReplayData, rngValue3); idx++, obfuscateCursor++)
    {
        *obfuscateCursor -= obfOffset;
        obfOffset += 7;
    }

    /* Calculate the checksum */
    /* (0x3f000318 + key + sum(c for c in decryptedData)) % (2 ** 32) */
    checksumCursor = (u8 *)&decryptedData->key;
    checksum = 0x3f000318;
    for (idx = 0; idx < fileSize - (i32)offsetof(ReplayData, key); idx++, checksumCursor++)
    {
        checksum += *checksumCursor;
    }

    if (checksum != decryptedData->checksum)
    {
        return ZUN_ERROR;
    }

    if (decryptedData->version != REPLAY_VERSION)
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

static ZunResult ReplayManager_AddedCallback(ReplayManager *mgr)
{
    mgr->frameId = 0;
    if (mgr->replayData == NULL)
    {
        mgr->replayData = ZUN_NEW(ReplayData); // BUG: allocated with new, cleaned up with free
        memcpy(mgr->replayData->magic, REPLAY_MAGIC, 4);
        mgr->replayData->shottypeChara = g_GameManager.character * SHOTTYPES_PER_CHARACTER + g_GameManager.shotType;
        mgr->replayData->version = REPLAY_VERSION;
        mgr->replayData->difficulty = g_GameManager.difficulty;
        memcpy(mgr->replayData->name, "NO NAME", 4); // why is this 4
        for (i32 idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); idx++)
        {
            mgr->replayData->stageReplayData[idx] = NULL;
        }
    }
    else
    {
        StageReplayData *oldStageReplayData = mgr->replayData->stageReplayData[g_GameManager.currentStage - 2];
        if (oldStageReplayData == NULL)
        {
            return ZUN_ERROR;
        }
        oldStageReplayData->score = g_GameManager.score;
    }
    if (mgr->replayData->stageReplayData[g_GameManager.currentStage - 1] != NULL)
    {
        DebugPrint("error : replay.cpp");
    }
    mgr->replayData->stageReplayData[g_GameManager.currentStage - 1] =
        (StageReplayData *)ZUN_ALLOC(STAGE_REPLAY_BUFFER_SIZE);
    StageReplayData *stageReplayData = mgr->replayData->stageReplayData[g_GameManager.currentStage - 1];
    stageReplayData->bombsRemaining = g_GameManager.bombsRemaining;
    stageReplayData->livesRemaining = g_GameManager.livesRemaining;
    stageReplayData->power = g_GameManager.currentPower;
    stageReplayData->rank = g_GameManager.rank;
    stageReplayData->pointItemsCollected = g_GameManager.pointItemsCollected;
    stageReplayData->randomSeed = g_GameManager.randomSeed;
    stageReplayData->powerItemCountForScore = g_GameManager.powerItemCountForScore;
    mgr->replayInputs = stageReplayData->replayInputs;
    mgr->replayInputs->frameNum = 0;
    mgr->replayInputs->inputKey = 0;
    mgr->unk44 = 0;
    return ZUN_SUCCESS;
}

static ZunResult ReplayManager_AddedCallbackDemo(ReplayManager *mgr)
{
    i32 idx;
    StageReplayData *replayData;

    mgr->frameId = 0;
    if (mgr->replayData == NULL)
    {
        mgr->replayData = (ReplayData *)FileSystem::OpenPath(mgr->replayFile, !g_GameManager.demoMode);
        if (ValidateReplayData(mgr->replayData, g_LastFileSize) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); idx++)
        {
            if (mgr->replayData->stageReplayData[idx] != NULL)
            {
                mgr->replayData->stageReplayData[idx] =
                    (StageReplayData *)((i32)mgr->replayData->stageReplayData[idx] + (i32)mgr->replayData);
            }
        }
    }
    if (mgr->replayData->stageReplayData[g_GameManager.currentStage - 1] == NULL)
    {
        return ZUN_ERROR;
    }
    replayData = mgr->replayData->stageReplayData[g_GameManager.currentStage - 1];
    g_GameManager.character = mgr->replayData->shottypeChara / 2;
    g_GameManager.shotType = mgr->replayData->shottypeChara % 2;
    g_GameManager.difficulty = (Difficulty)mgr->replayData->difficulty;
    g_GameManager.pointItemsCollected = replayData->pointItemsCollected;
    g_Rng.Initialize(replayData->randomSeed);
    g_GameManager.rank = replayData->rank;
    g_GameManager.livesRemaining = replayData->livesRemaining;
    g_GameManager.bombsRemaining = replayData->bombsRemaining;
    g_GameManager.currentPower = replayData->power;
    mgr->replayInputs = replayData->replayInputs;
    g_GameManager.powerItemCountForScore = replayData->powerItemCountForScore;
    if (g_GameManager.currentStage >= 2 && mgr->replayData->stageReplayData[g_GameManager.currentStage - 2] != NULL)
    {
        g_GameManager.score = mgr->replayData->stageReplayData[g_GameManager.currentStage - 2]->score;
        g_GameManager.guiScore = g_GameManager.score;
    }
    return ZUN_SUCCESS;
}

static ZunResult ReplayManager_DeletedCallback(ReplayManager *mgr)
{
    g_Chain.Cut(mgr->drawChain);
    mgr->drawChain = NULL;
    if (mgr->calcChainDemoHighPrio != NULL)
    {
        g_Chain.Cut(mgr->calcChainDemoHighPrio);
        mgr->calcChainDemoHighPrio = NULL;
    }
    ZUN_FREE(g_ReplayManager->replayData);
    ZUN_DELETE(g_ReplayManager);
    g_ReplayManager = NULL;
    return ZUN_SUCCESS;
}

ZunResult ReplayManager_RegisterChain(ZunBool isDemo, const char *replayFile)
{
    if (g_Supervisor.framerateMultiplier < 0.99f && !isDemo)
    {
        return ZUN_SUCCESS;
    }
    g_Supervisor.framerateMultiplier = 1.0f;
    if (g_ReplayManager == NULL)
    {
        ReplayManager *replayMgr = ZUN_NEW(ReplayManager);
        g_ReplayManager = replayMgr;
        replayMgr->replayData = NULL;
        replayMgr->isDemo = isDemo;
        replayMgr->replayFile = replayFile;
        switch (isDemo)
        {
        case false:
            replayMgr->calcChain = g_Chain.CreateElem((ChainCallback)ReplayManager_OnUpdate);
            replayMgr->calcChain->addedCallback = (ChainAddedCallback)ReplayManager_AddedCallback;
            replayMgr->calcChain->deletedCallback = (ChainDeletedCallback)ReplayManager_DeletedCallback;
            replayMgr->drawChain = g_Chain.CreateElem((ChainCallback)ReplayManager_OnDraw);
            replayMgr->calcChain->arg = replayMgr;
            if (g_Chain.AddToCalcChain(replayMgr->calcChain, TH_CHAIN_PRIO_CALC_REPLAYMANAGER) != ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            replayMgr->calcChainDemoHighPrio = NULL;
            break;
        case true:
            replayMgr->calcChain = g_Chain.CreateElem((ChainCallback)ReplayManager_OnUpdateDemoHighPrio);
            replayMgr->calcChain->addedCallback = (ChainAddedCallback)ReplayManager_AddedCallbackDemo;
            replayMgr->calcChain->deletedCallback = (ChainDeletedCallback)ReplayManager_DeletedCallback;
            replayMgr->drawChain = g_Chain.CreateElem((ChainCallback)ReplayManager_OnDraw);
            replayMgr->calcChain->arg = replayMgr;
            if (g_Chain.AddToCalcChain(replayMgr->calcChain, TH_CHAIN_PRIO_CALC_LOW_PRIO_REPLAYMANAGER_DEMO) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            replayMgr->calcChainDemoHighPrio = g_Chain.CreateElem((ChainCallback)ReplayManager_OnUpdateDemoLowPrio);
            replayMgr->calcChainDemoHighPrio->arg = replayMgr;
            g_Chain.AddToCalcChain(replayMgr->calcChainDemoHighPrio, TH_CHAIN_PRIO_CALC_HIGH_PRIO_REPLAYMANAGER_DEMO);
            break;
        }
        replayMgr->drawChain->arg = replayMgr;
        g_Chain.AddToDrawChain(replayMgr->drawChain, TH_CHAIN_PRIO_DRAW_REPLAYMANAGER);
    }
    else
    {
        switch (isDemo)
        {
        case false:
            ReplayManager_AddedCallback(g_ReplayManager);
            break;
        case true:
#if !TRIALBUILD
            return ReplayManager_AddedCallbackDemo(g_ReplayManager);
#else
            ReplayManager_AddedCallbackDemo(g_ReplayManager);
#endif
            break;
        }
    }
    return ZUN_SUCCESS;
}

void StopRecordingReplay()
{
    ReplayManager *mgr = g_ReplayManager;
    if (mgr != NULL)
    {
        mgr->replayInputs++;
        mgr->replayInputs->frameNum = mgr->frameId;
        mgr->replayInputs->inputKey = 0;
        mgr->replayInputs++;
        mgr->replayInputs->frameNum = 9999999;
        mgr->replayInputs->inputKey = 0;
        mgr->replayInputStageBookmarks[g_GameManager.currentStage - 1] = mgr->replayInputs + 1;
    }
}

#pragma var_order(stageIdx, mgr, slowDown)
void SaveReplay(const char *replayPath, const char *replayName)
{
    ReplayManager *mgr;
    f32 slowDown;
    i32 stageIdx;

    if (g_ReplayManager != NULL)
    {
        mgr = g_ReplayManager;
        if (!mgr->IsDemo())
        {
#pragma var_order(replayCopy, stageReplayPos, file, csumStagePos, checksum, checksumCursor, obfOffset, obfStagePos,    \
                  obfuscateCursor)
            if (replayPath != NULL)
            {
                FILE *file;
                u8 *checksumCursor;
                u8 *obfuscateCursor;
                i32 obfStagePos;
                u8 obfOffset;
                u32 checksum;
                i32 csumStagePos;
                size_t stageReplayPos;
                ReplayData replayCopy = *mgr->replayData;
                StopRecordingReplay();
                stageReplayPos = sizeof(ReplayData);
                for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(g_ReplayManager->replayData->stageReplayData);
                     stageIdx++)
                {
                    if (mgr->replayData->stageReplayData[stageIdx] != NULL)
                    {
                        replayCopy.stageReplayData[stageIdx] = (StageReplayData *)stageReplayPos;
                        stageReplayPos += (size_t)mgr->replayInputStageBookmarks[stageIdx] -
                                          (size_t)mgr->replayData->stageReplayData[stageIdx];
                    }
                }
                DebugPrint("%s write ...\n", replayPath);
                replayCopy.score = g_GameManager.guiScore;
                slowDown = (g_Supervisor.unk1b4 / g_Supervisor.unk1b8 - 0.5f) * 2.0f;
                if (slowDown < 0.0f)
                {
                    slowDown = 0.0f;
                }
                else if (slowDown >= 1.0f)
                {
                    slowDown = 1.0f;
                }
                replayCopy.slowdownRate = (1.0f - slowDown) * 100.0f;
                replayCopy.slowdownRate2 = replayCopy.slowdownRate + 1.12f;
                replayCopy.slowdownRate3 = replayCopy.slowdownRate + 2.34f;
                mgr->replayData->stageReplayData[g_GameManager.currentStage - 1]->score = g_GameManager.score;
                strcpy(replayCopy.name, replayName);
                _strdate(replayCopy.date);
                replayCopy.key = g_Rng.GetRandomU16InRange(128) + 64;
                replayCopy.rngValue3 = g_Rng.GetRandomU16InRange(256);
                replayCopy.rngValue1 = g_Rng.GetRandomU16InRange(256);
                replayCopy.rngValue2 = g_Rng.GetRandomU16InRange(256);

                // Calculate the checksum.
                checksumCursor = (u8 *)&replayCopy.key;
                checksum = 0x3f000318;
                for (stageIdx = 0; stageIdx < sizeof(ReplayData) - offsetof(ReplayData, key);
                     stageIdx++, checksumCursor++)
                {
                    checksum += *checksumCursor;
                }
                for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); stageIdx++)
                {
                    if (mgr->replayData->stageReplayData[stageIdx] != NULL)
                    {
                        checksumCursor = (u8 *)mgr->replayData->stageReplayData[stageIdx];
                        for (csumStagePos = 0; csumStagePos < (i32)mgr->replayInputStageBookmarks[stageIdx] -
                                                                  (i32)mgr->replayData->stageReplayData[stageIdx];
                             csumStagePos++, checksumCursor++)
                        {
                            checksum += *checksumCursor;
                        }
                    }
                }
                replayCopy.checksum = checksum;

                // Obfuscate the data.
                obfuscateCursor = (u8 *)&replayCopy.rngValue3;
                obfOffset = replayCopy.key;
                for (stageIdx = 0; stageIdx < sizeof(ReplayData) - offsetof(ReplayData, rngValue3);
                     stageIdx++, obfuscateCursor++)
                {
                    *obfuscateCursor += obfOffset;
                    obfOffset += 7;
                }
                for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); stageIdx++)
                {
                    if (mgr->replayData->stageReplayData[stageIdx] != NULL)
                    {
                        obfuscateCursor = (u8 *)mgr->replayData->stageReplayData[stageIdx];
                        for (obfStagePos = 0; obfStagePos < (i32)mgr->replayInputStageBookmarks[stageIdx] -
                                                                (i32)mgr->replayData->stageReplayData[stageIdx];
                             obfStagePos++, obfuscateCursor++)
                        {
                            *obfuscateCursor += obfOffset;
                            obfOffset += 7;
                        }
                    }
                }

                // Write the data to the replay file.
                file = fopen(replayPath, "wb");
                fwrite(&replayCopy, sizeof(ReplayData), 1, file);
                for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); stageIdx++)
                {
                    if (mgr->replayData->stageReplayData[stageIdx] != NULL)
                    {
                        fwrite(mgr->replayData->stageReplayData[stageIdx], 1,
                               (i32)mgr->replayInputStageBookmarks[stageIdx] -
                                   (i32)mgr->replayData->stageReplayData[stageIdx],
                               file);
                    }
                }
                fclose(file);
            }
            for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); stageIdx++)
            {
                if (g_ReplayManager->replayData->stageReplayData[stageIdx] != NULL)
                {
                    DebugPrint("Replay Size %d\n", (i32)mgr->replayInputStageBookmarks[stageIdx] -
                                                       (i32)mgr->replayData->stageReplayData[stageIdx]);
                    ZUN_FREE(g_ReplayManager->replayData->stageReplayData[stageIdx]);
                }
            }
        }
        g_Chain.Cut(g_ReplayManager->calcChain);
    }
}
} // namespace th06

#include <stddef.h>
#include <stdio.h>
#include <time.h>

#include "Controller.hpp"
#include "FileSystem.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "ReplayManager.hpp"
#include "Rng.hpp"
#include "Supervisor.hpp"
#include "utils.hpp"

namespace th06
{
DIFFABLE_STATIC(ReplayManager *, g_ReplayManager)

#pragma var_order(idx, decryptedData, obfOffset, obfuscateCursor, checksum, checksumCursor)
ZunResult ReplayManager::ValidateReplayData(ReplayData *data, i32 fileSize)
{
    u8 *checksumCursor;
    u32 checksum;
    u8 *obfuscateCursor;
    u8 obfOffset;
    i32 idx;
    ReplayData *decryptedData;

    decryptedData = data;

    if (decryptedData == NULL)
    {
        return ZUN_ERROR;
    }

    /* "T6RP" magic bytes */
    if (*(i32 *)decryptedData->magic != *(i32 *)"T6RP")
    {
        return ZUN_ERROR;
    }

    /* Deobfuscate the replay decryptedData */
    obfuscateCursor = (u8 *)&decryptedData->rngValue3;
    obfOffset = decryptedData->key;
    for (idx = 0; idx < fileSize - (i32)offsetof(ReplayData, rngValue3); idx += 1, obfuscateCursor += 1)
    {
        *obfuscateCursor -= obfOffset;
        obfOffset += 7;
    }

    /* Calculate the checksum */
    /* (0x3f000318 + key + sum(c for c in decryptedData)) % (2 ** 32) */
    checksumCursor = (u8 *)&decryptedData->key;
    checksum = 0x3f000318;
    for (idx = 0; idx < fileSize - (i32)offsetof(ReplayData, key); idx += 1, checksumCursor += 1)
    {
        checksum += *checksumCursor;
    }

    if (checksum != decryptedData->checksum)
    {
        return ZUN_ERROR;
    }

    if (decryptedData->version != GAME_VERSION)
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

ZunResult ReplayManager::RegisterChain(i32 isDemo, char *replayFile)
{
    ReplayManager *replayMgr;

    if (g_Supervisor.framerateMultiplier < 0.99f && !isDemo)
    {
        return ZUN_SUCCESS;
    }
    g_Supervisor.framerateMultiplier = 1.0f;
    if (g_ReplayManager == NULL)
    {
        replayMgr = new ReplayManager();
        g_ReplayManager = replayMgr;
        replayMgr->replayData = NULL;
        replayMgr->isDemo = isDemo;
        replayMgr->replayFile = replayFile;
        switch (isDemo)
        {
        case false:
            replayMgr->calcChain = g_Chain.CreateElem((ChainCallback)ReplayManager::OnUpdate);
            replayMgr->calcChain->addedCallback = (ChainAddedCallback)AddedCallback;
            replayMgr->calcChain->deletedCallback = (ChainDeletedCallback)DeletedCallback;
            replayMgr->drawChain = g_Chain.CreateElem((ChainCallback)ReplayManager::OnDraw);
            replayMgr->calcChain->arg = replayMgr;
            if (g_Chain.AddToCalcChain(replayMgr->calcChain, TH_CHAIN_PRIO_CALC_REPLAYMANAGER))
            {
                return ZUN_ERROR;
            }
            replayMgr->calcChainDemoHighPrio = NULL;
            break;
        case true:
            replayMgr->calcChain = g_Chain.CreateElem((ChainCallback)ReplayManager::OnUpdateDemoHighPrio);
            replayMgr->calcChain->addedCallback = (ChainAddedCallback)AddedCallbackDemo;
            replayMgr->calcChain->deletedCallback = (ChainDeletedCallback)DeletedCallback;
            replayMgr->drawChain = g_Chain.CreateElem((ChainCallback)ReplayManager::OnDraw);
            replayMgr->calcChain->arg = replayMgr;
            if (g_Chain.AddToCalcChain(replayMgr->calcChain, TH_CHAIN_PRIO_CALC_LOW_PRIO_REPLAYMANAGER_DEMO))
            {
                return ZUN_ERROR;
            }
            replayMgr->calcChainDemoHighPrio = g_Chain.CreateElem((ChainCallback)ReplayManager::OnUpdateDemoLowPrio);
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
            AddedCallback(g_ReplayManager);
            break;
        case true:
            return AddedCallbackDemo(g_ReplayManager);
            break;
        }
    }
    return ZUN_SUCCESS;
}

#define TH_BUTTON_REPLAY_CAPTURE                                                                                       \
    (TH_BUTTON_SHOOT | TH_BUTTON_BOMB | TH_BUTTON_FOCUS | TH_BUTTON_SKIP | TH_BUTTON_DIRECTION | TH_BUTTON_SHOOT2 |    \
     TH_BUTTON_BOMB2 | TH_BUTTON_FOCUS2 | TH_BUTTON_DIRECTION2)

ChainCallbackResult ReplayManager::OnUpdate(ReplayManager *mgr)
{
    u16 inputs;

    if (!g_GameManager.isInMenu)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    inputs = IS_PRESSED(TH_BUTTON_REPLAY_CAPTURE);
    if (inputs != mgr->replayInputs->inputKey)
    {
        mgr->replayInputs += 1;
        mgr->replayInputStageBookmarks[g_GameManager.currentStage - 1] = mgr->replayInputs + 1;
        mgr->replayInputs->frameNum = mgr->frameId;
        mgr->replayInputs->inputKey = inputs;
    }
    mgr->frameId += 1;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ReplayManager::OnUpdateDemoLowPrio(ReplayManager *mgr)
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

ChainCallbackResult ReplayManager::OnUpdateDemoHighPrio(ReplayManager *mgr)
{
    if (!g_GameManager.isInMenu)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    while (mgr->frameId >= mgr->replayInputs[1].frameNum)
    {
        mgr->replayInputs += 1;
    }
    g_CurFrameInput = IS_PRESSED(0xFFFFFFFF & ~TH_BUTTON_REPLAY_CAPTURE) | mgr->replayInputs->inputKey;
    g_IsEigthFrameOfHeldInput = 0;
    if (g_LastFrameInput == g_CurFrameInput)
    {
        if (30 <= g_NumOfFramesInputsWereHeld)
        {
            if (g_NumOfFramesInputsWereHeld % 8 == 0)
            {
                g_IsEigthFrameOfHeldInput = 1;
            }
            if (38 <= g_NumOfFramesInputsWereHeld)
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
    mgr->frameId += 1;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult ReplayManager::OnDraw(ReplayManager *mgr)
{
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

__inline StageReplayData *AllocateStageReplayData(i32 size)
{
    return (StageReplayData *)malloc(size);
}

__inline void ReleaseReplayData(void *data)
{
    return free(data);
}

__inline void ReleaseStageReplayData(void *data)
{
    return free(data);
}

#pragma var_order(stageReplayData, idx, oldStageReplayData)
ZunResult ReplayManager::AddedCallback(ReplayManager *mgr)
{
    StageReplayData *stageReplayData;
    StageReplayData *oldStageReplayData;
    i32 idx;

    mgr->frameId = 0;
    if (mgr->replayData == NULL)
    {
        mgr->replayData = new ReplayData();
        memcpy(&mgr->replayData->magic[0], "T6RP", 4);
        mgr->replayData->shottypeChara = g_GameManager.character * 2 + g_GameManager.shotType;
        mgr->replayData->shottypeChara2 = g_GameManager.character2 * 2 + g_GameManager.shotType2;
        mgr->replayData->version = 0x102;
        mgr->replayData->difficulty = g_GameManager.difficulty;
        memcpy(&mgr->replayData->name, "NO NAME", 4);
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); idx += 1)
        {
            mgr->replayData->stageReplayData[idx] = NULL;
        }
    }
    else
    {
        oldStageReplayData = mgr->replayData->stageReplayData[g_GameManager.currentStage - 2];
        if (oldStageReplayData == NULL)
        {
            return ZUN_ERROR;
        }
        oldStageReplayData->score = g_GameManager.score;
    }
    if (mgr->replayData->stageReplayData[g_GameManager.currentStage - 1] != NULL)
    {
        utils::DebugPrint2("error : replay.cpp");
    }
    mgr->replayData->stageReplayData[g_GameManager.currentStage - 1] = AllocateStageReplayData(sizeof(StageReplayData));
    stageReplayData = mgr->replayData->stageReplayData[g_GameManager.currentStage - 1];
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

ZunResult ReplayManager::AddedCallbackDemo(ReplayManager *mgr)
{
    i32 idx;
    StageReplayData *replayData;

    mgr->frameId = 0;
    if (mgr->replayData == NULL)
    {
        mgr->replayData = (ReplayData *)FileSystem::OpenPath(mgr->replayFile, g_GameManager.demoMode == 0);
        if (ValidateReplayData(mgr->replayData, g_LastFileSize) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); idx += 1)
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
    g_GameManager.character2 = mgr->replayData->shottypeChara2 / 2;
    g_GameManager.shotType2 = mgr->replayData->shottypeChara2 % 2;
    g_GameManager.difficulty = (Difficulty)mgr->replayData->difficulty;
    g_GameManager.pointItemsCollected = replayData->pointItemsCollected;
    g_Rng.Initialize(replayData->randomSeed);
    g_GameManager.rank = replayData->rank;
    g_GameManager.livesRemaining = replayData->livesRemaining;
    g_GameManager.bombsRemaining = replayData->bombsRemaining;
    g_GameManager.currentPower = replayData->power;
    mgr->replayInputs = replayData->replayInputs;
    g_GameManager.powerItemCountForScore = replayData->powerItemCountForScore;
    if (2 <= g_GameManager.currentStage && mgr->replayData->stageReplayData[g_GameManager.currentStage - 2] != NULL)
    {
        g_GameManager.guiScore = mgr->replayData->stageReplayData[g_GameManager.currentStage - 2]->score;
        g_GameManager.score = g_GameManager.guiScore;
    }
    return ZUN_SUCCESS;
}

ZunResult ReplayManager::DeletedCallback(ReplayManager *mgr)
{
    g_Chain.Cut(mgr->drawChain);
    mgr->drawChain = NULL;
    if (mgr->calcChainDemoHighPrio != NULL)
    {
        g_Chain.Cut(mgr->calcChainDemoHighPrio);
        mgr->calcChainDemoHighPrio = NULL;
    }
    ReleaseReplayData(g_ReplayManager->replayData);
    delete g_ReplayManager;
    g_ReplayManager = NULL;
    g_ReplayManager = NULL;
    return ZUN_SUCCESS;
}

void ReplayManager::StopRecording()
{
    ReplayManager *mgr = g_ReplayManager;
    if (mgr != NULL)
    {
        mgr->replayInputs += 1;
        mgr->replayInputs->frameNum = mgr->frameId;
        mgr->replayInputs->inputKey = 0;
        mgr->replayInputs += 1;
        mgr->replayInputs->frameNum = 9999999;
        mgr->replayInputs->inputKey = 0;
        mgr->replayInputStageBookmarks[g_GameManager.currentStage - 1] = mgr->replayInputs + 1;
    }
}

#pragma var_order(stageIdx, mgr, slowDown, replayCopy, stageReplayPos, file, csumStagePos, checksum, checksumCursor,   \
                  obfOffset, obfStagePos, obfuscateCursor)
void ReplayManager::SaveReplay(char *replayPath, char *replayName)
{
    ReplayManager *mgr;
    FILE *file;
    u8 *checksumCursor;
    ReplayData replayCopy;
    u8 *obfuscateCursor;
    i32 obfStagePos;
    u8 obfOffset;
    u32 checksum;
    i32 csumStagePos;
    size_t stageReplayPos;
    f32 slowDown;
    i32 stageIdx;

    if (g_ReplayManager != NULL)
    {
        mgr = g_ReplayManager;
        if (!mgr->IsDemo())
        {
            if (replayPath != NULL)
            {
                replayCopy = *mgr->replayData;
                ReplayManager::StopRecording();
                stageReplayPos = sizeof(ReplayData);
                for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(g_ReplayManager->replayData->stageReplayData);
                     stageIdx += 1)
                {
                    if (mgr->replayData->stageReplayData[stageIdx] != NULL)
                    {
                        replayCopy.stageReplayData[stageIdx] = (StageReplayData *)stageReplayPos;
                        stageReplayPos += (size_t)mgr->replayInputStageBookmarks[stageIdx] -
                                          (size_t)mgr->replayData->stageReplayData[stageIdx];
                    }
                }
                utils::DebugPrint2("%s write ...\n", replayPath);
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
                     stageIdx += 1, checksumCursor += 1)
                {
                    checksum += *checksumCursor;
                }
                for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); stageIdx += 1)
                {
                    if (mgr->replayData->stageReplayData[stageIdx] != NULL)
                    {
                        checksumCursor = (u8 *)mgr->replayData->stageReplayData[stageIdx];
                        for (csumStagePos = 0; csumStagePos < (i32)mgr->replayInputStageBookmarks[stageIdx] -
                                                                  (i32)mgr->replayData->stageReplayData[stageIdx];
                             csumStagePos += 1, checksumCursor += 1)
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
                     stageIdx += 1, obfuscateCursor += 1)
                {
                    *obfuscateCursor += obfOffset;
                    obfOffset += 7;
                }
                for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); stageIdx += 1)
                {
                    if (mgr->replayData->stageReplayData[stageIdx] != NULL)
                    {
                        obfuscateCursor = (u8 *)mgr->replayData->stageReplayData[stageIdx];
                        for (obfStagePos = 0; obfStagePos < (i32)mgr->replayInputStageBookmarks[stageIdx] -
                                                                (i32)mgr->replayData->stageReplayData[stageIdx];
                             obfStagePos += 1, obfuscateCursor += 1)
                        {
                            *obfuscateCursor += obfOffset;
                            obfOffset += 7;
                        }
                    }
                }

                // Write the data to the replay file.
                file = fopen(replayPath, "wb");
                fwrite(&replayCopy, sizeof(ReplayData), 1, file);
                for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); stageIdx += 1)
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
            for (stageIdx = 0; stageIdx < ARRAY_SIZE_SIGNED(mgr->replayData->stageReplayData); stageIdx += 1)
            {
                if (g_ReplayManager->replayData->stageReplayData[stageIdx] != NULL)
                {
                    utils::DebugPrint2("Replay Size %d\n", (i32)mgr->replayInputStageBookmarks[stageIdx] -
                                                               (i32)mgr->replayData->stageReplayData[stageIdx]);
                    ReleaseStageReplayData(g_ReplayManager->replayData->stageReplayData[stageIdx]);
                }
            }
        }
        g_Chain.Cut(g_ReplayManager->calcChain);
    }
    return;
}
}; // namespace th06

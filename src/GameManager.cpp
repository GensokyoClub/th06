#include "stdafx.h"

#include "GameManager.hpp"
#include "AsciiManager.hpp"
#include "ChainPriorities.hpp"
#include "EffectManager.hpp"
#include "EnemyManager.hpp"
#include "Gui.hpp"
#include "ReplayManager.hpp"
#include "ScreenEffect.hpp"
#include "Stage.hpp"

#include <d3d8types.h>
#include "BulletManager.hpp"
#include "SoundPlayer.hpp"

namespace th06
{

static u32 g_ExtraLivesScores[] = {10000000, 20000000, 40000000, 60000000, 1900000000};

struct DifficultyInfo
{
    u32 rank;
    u32 minRank;
    u32 maxRank;
};
ZUN_ASSERT_TYPE(DifficultyInfo, 0xc, 4);

GameManager g_GameManager;
ChainElem g_GameMgrCalcChainElem;
ChainElem g_GameManagerOnDrawChain;

#define MAX_SCORE 999999999

#define DEMO_FADEOUT_FRAMES 3600
#define DEMO_FRAMES 3720

#define GUI_SCORE_STEP 78910

#define MAX_LIVES 8

ZunBool GameManager::IsInBounds(f32 x, f32 y, f32 width, f32 height)
{
    if (width / 2.0f + x < GAME_REGION_LEFT)
    {
        return false;
    }
    if ((x - width / 2.0f) > /*GAME_REGION_LEFT +*/ g_GameManager.gameRegionSize.x)
    {
        return false;
    }
    if (height / 2.0f + y < GAME_REGION_TOP)
    {
        return false;
    }
    if (y - height / 2.0f > /*GAME_REGION_TOP +*/ g_GameManager.gameRegionSize.y)
    {
        return false;
    }

    return true;
}

ChainCallbackResult GameManager_OnUpdate(GameManager *gameManager)
{
    if (gameManager->demoMode)
    {
        if (WAS_PRESSED(TH_BUTTON_ANY))
        {
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        }
        gameManager->demoFrames++;
        if (gameManager->demoFrames == DEMO_FADEOUT_FRAMES)
        {
            ScreenEffect_RegisterChain(SCREEN_EFFECT_FADE_OUT, 120, 0x000000, 0, 0);
        }
        if (gameManager->demoFrames >= DEMO_FRAMES)
        {
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        }
    }
    if (!gameManager->isInRetryMenu && !gameManager->isInGameMenu && !gameManager->demoMode &&
        WAS_PRESSED(TH_BUTTON_MENU))
    {
        gameManager->isInGameMenu = 1;
        g_GameManager.gameRegionScreenPos.x = GAME_REGION_POS_X;
        g_GameManager.gameRegionScreenPos.y = GAME_REGION_POS_Y;
        g_GameManager.gameRegionSize.x = GAME_REGION_WIDTH;
        g_GameManager.gameRegionSize.y = GAME_REGION_HEIGHT;
#if BUILT_SINCE(102g)
        g_Supervisor.forceRedrawFrames = 3;
#endif
    }

    gameManager->isInMenu = (ZunBool)(!gameManager->isInRetryMenu && !gameManager->isInGameMenu);

    g_Supervisor.viewport.X = gameManager->gameRegionScreenPos.x;
    g_Supervisor.viewport.Y = gameManager->gameRegionScreenPos.y;
    g_Supervisor.viewport.Width = gameManager->gameRegionSize.x;
    g_Supervisor.viewport.Height = gameManager->gameRegionSize.y;
    g_Supervisor.viewport.MinZ = 0.5f;
    g_Supervisor.viewport.MaxZ = 1.0f;

    SetupCamera(0.0f);

    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, g_Stage.skyFog.color, 1.0f, 0);

    // Seems like gameManager->isInGameMenu was supposed to have 3 states, but all the times it ends up checking both
    if (gameManager->isInGameMenu == 1 || gameManager->isInGameMenu == 2 || gameManager->isInRetryMenu)
    {
        return CHAIN_CALLBACK_RESULT_BREAK;
    }

    if (gameManager->score >= MAX_SCORE + 1)
    {
        gameManager->score = MAX_SCORE - 9;
    }
    if (gameManager->guiScore != gameManager->score)
    {
        if (gameManager->score < gameManager->guiScore)
        {
            gameManager->score = gameManager->guiScore;
        }

        u32 scoreIncrement = (gameManager->score - gameManager->guiScore) >> 5;
        if (scoreIncrement >= GUI_SCORE_STEP)
        {
            scoreIncrement = GUI_SCORE_STEP;
        }
        else if (scoreIncrement < 10)
        {
            scoreIncrement = 10;
        }
        scoreIncrement -= scoreIncrement % 10;

        if (gameManager->nextScoreIncrement < scoreIncrement)
        {
            gameManager->nextScoreIncrement = scoreIncrement;
        }
        if (gameManager->guiScore + gameManager->nextScoreIncrement > gameManager->score)
        {
            gameManager->nextScoreIncrement = gameManager->score - gameManager->guiScore;
        }

        gameManager->guiScore += gameManager->nextScoreIncrement;
        if (gameManager->guiScore >= gameManager->score)
        {
            gameManager->nextScoreIncrement = 0;
            gameManager->guiScore = gameManager->score;
        }
        if (gameManager->livesEarned >= 0 && g_ExtraLivesScores[gameManager->livesEarned] <= gameManager->guiScore)
        {
            if (gameManager->livesRemaining < MAX_LIVES)
            {
                gameManager->livesRemaining++;
                g_SoundPlayer.PlaySoundByIdx(SOUND_1UP);
            }
            g_Gui.flags.flag0 = 2;
            gameManager->livesEarned++;
            g_GameManager.IncreaseSubrank(200);
        }
        if (gameManager->highScore < gameManager->guiScore)
        {
            gameManager->highScore = gameManager->guiScore;
        }
    }
    gameManager->gameFrames++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult GameManager_OnDraw(GameManager *gameManager)
{
    if (gameManager->isInGameMenu)
    {
        gameManager->isInGameMenu = 2;
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ZunResult GameManager_AddedCallback(GameManager *mgr)
{
    static const char *g_EclFiles[] = {"dummy",
                                       "data/ecldata1.ecl",
                                       "data/ecldata2.ecl",
                                       "data/ecldata3.ecl",
                                       "data/ecldata4.ecl",
                                       "data/ecldata5.ecl",
                                       "data/ecldata6.ecl",
                                       "data/ecldata7.ecl"};
    static const char *g_AnmStageFiles[][2] = {
        {"dummy", "dummy"},
        {"data/stg1enm.anm", "data/stg1enm2.anm"},
        {"data/stg2enm.anm", "data/stg2enm2.anm"},
        {"data/stg3enm.anm", NULL},
        {"data/stg4enm.anm", NULL},
        {"data/stg5enm.anm", "data/stg5enm2.anm"},
        {"data/stg6enm.anm", "data/stg6enm2.anm"},
        {"data/stg7enm.anm", "data/stg7enm2.anm"},
    };
    static DifficultyInfo g_DifficultyInfoForReplay[] = {
        // rank, minRank, maxRank
        /* EASY    */ {16, 12, 20},
        /* NORMAL  */ {16, 10, 32},
        /* HARD    */ {16, 10, 32},
        /* LUNATIC */ {16, 10, 32},
        /* EXTRA   */ {16, 14, 18},
    };
    static DifficultyInfo g_DifficultyInfo[] = {
        // rank, minRank, maxRank
        /* EASY    */ {16, 12, 20},
        /* NORMAL  */ {16, 10, 32},
        /* HARD    */ {16, 10, 32},
        /* LUNATIC */ {16, 10, 32},
        /* EXTRA   */ {16, 14, 18},
    };

#if BUILT_SINCE(102g)
    ZunBool failedToLoadReplay = false;
#endif
    g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);

    if (g_Supervisor.curState != SUPERVISOR_STATE_NEXT_STAGE)
    {
#if BUILT_SINCE(102g)
        g_Supervisor.defaultConfig.bombCount = g_GameManager.bombsRemaining;
        g_Supervisor.defaultConfig.lifeCount = g_GameManager.livesRemaining;
#endif
        mgr->gameRegionScreenPos.x = GAME_REGION_POS_X;
        mgr->gameRegionScreenPos.y = GAME_REGION_POS_Y;
        mgr->gameRegionSize.x = GAME_REGION_WIDTH;
        mgr->gameRegionSize.y = GAME_REGION_HEIGHT;
        mgr->playerMovementAreaTopLeftPos.x = GAME_REGION_LEFT + 8.0f;
        mgr->playerMovementAreaTopLeftPos.y = GAME_REGION_TOP + 16.0f;
        mgr->playerMovementAreaSize.x = GAME_REGION_WIDTH - 8.0f * 2.0f;
        mgr->playerMovementAreaSize.y = GAME_REGION_HEIGHT - 16.0f * 2.0f;
        mgr->counat = 0;
        mgr->guiScore = 0;
        mgr->score = 0;
        mgr->nextScoreIncrement = 0;
        mgr->highScore = 100000;
        mgr->currentPower = 0;
        mgr->continuesUsed = 0;
        if (mgr->currentStage >= STAGE_INDEX(ExtraStage))
        {
            mgr->difficulty = EXTRA;
        }
        if (mgr->difficulty < EXTRA)
        {
            mgr->livesEarned = 0;
        }
        else
        {
            // Disable earning more lives
            mgr->livesEarned = ARRAY_SIZE(g_ExtraLivesScores) - 1;
        }
        g_GameManager.powerItemCountForScore = 0;
        mgr->rank = 8;
        mgr->grazeInTotal = 0;
        mgr->pointItemsCollected = 0;

        Catk *catk = mgr->catk;
        for (i32 i = 0; i < CATK_COUNT; i++, catk++)
        {
            // Randomize catk content.
            for (u32 j = 0; j < sizeof(Catk) / sizeof(u16); j++)
            {
                ((u16 *)catk)[j] = g_Rng.GetRandomU16();
            }
            catk->base.magic = CATK_MAGIC;
            catk->base.unkLen = sizeof(Catk);
            catk->base.th6kLen = sizeof(Catk);
            catk->base.version = TH6K_VERSION;
            catk->idx = i;
            catk->numAttempts = 0;
            catk->numSuccess = 0;
        }
        {
            ScoreDat *scoredat = OpenScore("score.dat");
            g_GameManager.highScore = GetHighScore(scoredat, NULL, CURRENT_SHOTTYPE(), g_GameManager.difficulty);
            ParseCatk(scoredat, mgr->catk);
            ParseClrd(scoredat, mgr->clrd);
            ParsePscr(scoredat, (Pscr *)mgr->pscr);
            if (mgr->isInPracticeMode)
            {
                g_GameManager.highScore =
                    mgr->pscr[CURRENT_SHOTTYPE()][g_GameManager.currentStage][g_GameManager.difficulty].score;
            }
            ReleaseScoreDat(scoredat);
        }
        mgr->rank = g_DifficultyInfo[g_GameManager.difficulty].rank;
        mgr->minRank = g_DifficultyInfo[g_GameManager.difficulty].minRank;
        mgr->maxRank = g_DifficultyInfo[g_GameManager.difficulty].maxRank;
        mgr->deaths = 0;
        mgr->bombsUsed = 0;
        mgr->spellcardsCaptured = 0;
    }
    else
    {
        mgr->guiScore = mgr->score;
        mgr->nextScoreIncrement = 0;
    }
    mgr->subRank = 0;
    mgr->pointItemsCollectedInStage = 0;
    mgr->grazeInStage = 0;
    mgr->isInGameMenu = 0;
    mgr->currentStage++; // NOTE: this really should've been after the if
    if (!g_GameManager.isInReplay)
    {
        i32 shottype = CURRENT_SHOTTYPE();
        if (mgr->continuesUsed == 0 &&
            mgr->clrd[shottype].stagesClearedWithoutContinues[g_GameManager.difficulty] < mgr->currentStage - 1)
        {
            mgr->clrd[shottype].stagesClearedWithoutContinues[g_GameManager.difficulty] = mgr->currentStage - 1;
        }
        if (mgr->clrd[shottype].stagesCleared[g_GameManager.difficulty] < mgr->currentStage - 1)
        {
            mgr->clrd[shottype].stagesCleared[g_GameManager.difficulty] = mgr->currentStage - 1;
        }
    }
    if (mgr->isInPracticeMode)
    {
        switch (mgr->currentStage)
        {
        case Stage1:
            break;
        case Stage2:
            mgr->currentPower = MAX_POWER / 2;
            break;
        default:
            mgr->currentPower = MAX_POWER;
        }
    }
    g_Supervisor.LoadPbg3(CM_PBG3_INDEX, TH_CM_DAT_FILE);
    g_Supervisor.LoadPbg3(ST_PBG3_INDEX, TH_ST_DAT_FILE);
    if (g_GameManager.isInReplay == TRUE)
    {
#if BUILT_SINCE(102g)
        if (ReplayManager_RegisterChain(true, g_GameManager.replayFile) != ZUN_SUCCESS)
        {
            failedToLoadReplay = true;
        }
#else
        ReplayManager_RegisterChain(true, g_GameManager.replayFile);
#endif
        while (g_ExtraLivesScores[mgr->livesEarned] <= mgr->guiScore)
        {
            mgr->livesEarned++;
        }
        mgr->minRank = g_DifficultyInfoForReplay[g_GameManager.difficulty].minRank;
        mgr->maxRank = g_DifficultyInfoForReplay[g_GameManager.difficulty].maxRank;
    }
    g_Rng.ResetGenerationCount();
    mgr->randomSeed = g_Rng.GetSeed();
    if (Stage_RegisterChain(mgr->currentStage) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_STAGE);
        return ZUN_ERROR;
    }

    if (Player_RegisterChain(0) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_PLAYER);
        return ZUN_ERROR;
    }
    if (BulletManager_RegisterChain("data/etama.anm") != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_BULLETMANAGER);
        return ZUN_ERROR;
    }
    if (EnemyManager_RegisterChain(g_AnmStageFiles[mgr->currentStage][0], g_AnmStageFiles[mgr->currentStage][1]) !=
        ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ENEMYMANAGER);
        return ZUN_ERROR;
    }
    if (g_EclManager.Load(g_EclFiles[mgr->currentStage]) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ECLMANAGER);
        return ZUN_ERROR;
    }
    if (EffectManager_RegisterChain() != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_EFFECTMANAGER);
        return ZUN_ERROR;
    }
    if (Gui_RegisterChain() != ZUN_SUCCESS)
    {
        g_GameErrorContext.Log(TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_GUI);
        return ZUN_ERROR;
    }
    if (!g_GameManager.isInReplay)
    {
        ReplayManager_RegisterChain(false, "replay/th6_00.rpy");
    }
    if (!g_GameManager.demoMode)
    {
        // Read boss battle, and store it for use when boss is started.
        g_Supervisor.ReadMidiFile(1, g_Stage.stdData->songPaths[1]);
        // Immediately start playing this level's theme.
        g_Supervisor.PlayAudio(g_Stage.stdData->songPaths[0]);
    }
    mgr->isInRetryMenu = false;
    mgr->isInMenu = true;
    if (g_Supervisor.curState != SUPERVISOR_STATE_NEXT_STAGE)
    {
        g_Supervisor.unk1b4 = 0.0f;
        g_Supervisor.unk1b8 = 0.0f;
    }
    mgr->isTimeStopped = false;
    mgr->score = 0;
    mgr->isGameCompleted = false;
    g_AsciiManager.InitializeVms();
#if BUILT_SINCE(102g)
    if (failedToLoadReplay)
    {
        g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
    }
    g_Supervisor.forceRedrawFrames = 3;
#endif
    return ZUN_SUCCESS;
}

static ZunResult GameManager_DeletedCallback(GameManager *mgr)
{
    g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
    if (!g_GameManager.demoMode)
    {
        g_Supervisor.StopAudio();
    }
    Stage_CutChain();
    BulletManager_CutChain();
    Player_CutChain();
    EnemyManager_CutChain();
    g_EclManager.Unload();
    EffectManager_CutChain();
    Gui_CutChain();
    StopRecordingReplay();
    mgr->isInMenu = false;
    g_AsciiManager.InitializeVms();
    return ZUN_SUCCESS;
}

ZunResult GameManager_RegisterChain()
{
    GameManager *mgr = &g_GameManager;

    g_GameMgrCalcChainElem.SetCallback((ChainCallback)GameManager_OnUpdate);
    g_GameMgrCalcChainElem.addedCallback = (ChainAddedCallback)GameManager_AddedCallback;
    g_GameMgrCalcChainElem.deletedCallback = (ChainDeletedCallback)GameManager_DeletedCallback;
    g_GameMgrCalcChainElem.arg = mgr;

    mgr->gameFrames = 0;

    if (g_FuncChain.AddToCalcChain(&g_GameMgrCalcChainElem, TH_CHAIN_PRIO_CALC_GAMEMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    g_GameManagerOnDrawChain.SetCallback((ChainCallback)GameManager_OnDraw);
    g_GameManagerOnDrawChain.arg = mgr;
    g_FuncChain.AddToDrawChain(&g_GameManagerOnDrawChain, TH_CHAIN_PRIO_DRAW_GAMEMANAGER);
    return ZUN_SUCCESS;
}

void GameManager_CutChain()
{
    g_FuncChain.Cut(&g_GameMgrCalcChainElem);
    g_FuncChain.Cut(&g_GameManagerOnDrawChain);
}

#pragma var_order(cameraDistance, viewportMiddleHeight, viewportMiddleWidth, aspectRatio, fov)
void SetupCameraStageBackground(f32 extraRenderDistance)
{
    f32 viewportMiddleWidth = g_Supervisor.viewport.Width / 2.0f;
    f32 viewportMiddleHeight = g_Supervisor.viewport.Height / 2.0f;
    f32 aspectRatio = (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height;
    f32 fov = D3DXToRadian(30.0f);
    f32 cameraDistance = viewportMiddleHeight / (f32)tan(fov / 2.0f);
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix,
                       &D3DXVECTOR3(viewportMiddleWidth, -viewportMiddleHeight, -cameraDistance),
                       &D3DXVECTOR3(viewportMiddleWidth, -viewportMiddleHeight, 0.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    g_GameManager.cameraDistance = fabsf(cameraDistance);
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, fov, aspectRatio, 100.0f,
                               10000.0f + extraRenderDistance);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
}

#pragma var_order(cameraDistance, viewportMiddleHeight, viewportMiddleWidth, aspectRatio, fov)
void SetupCamera(f32 extraRenderDistance)
{
    f32 viewportMiddleWidth = g_Supervisor.viewport.Width / 2.0f;
    f32 viewportMiddleHeight = g_Supervisor.viewport.Height / 2.0f;
    f32 aspectRatio = (f32)g_Supervisor.viewport.Width / (f32)g_Supervisor.viewport.Height;
    f32 fov = D3DXToRadian(30.0f);
    f32 cameraDistance = viewportMiddleHeight / (f32)tan(fov / 2.0f);
    D3DXMatrixLookAtLH(&g_Supervisor.viewMatrix,
                       &D3DXVECTOR3(viewportMiddleWidth, -viewportMiddleHeight,
                                    -cameraDistance * (f32)g_GameManager.stageCameraFacingDir.z),
                       &D3DXVECTOR3(viewportMiddleWidth + (f32)g_GameManager.stageCameraFacingDir.x,
                                    -viewportMiddleHeight + (f32)g_GameManager.stageCameraFacingDir.y, 0.0f),
                       &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    g_GameManager.cameraDistance = fabsf(cameraDistance);
    D3DXMatrixPerspectiveFovLH(&g_Supervisor.projectionMatrix, fov, aspectRatio, 100.0f,
                               10000.0f + extraRenderDistance);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_VIEW, &g_Supervisor.viewMatrix);
    g_Supervisor.d3dDevice->SetTransform(D3DTS_PROJECTION, &g_Supervisor.projectionMatrix);
}

void GameManager::IncreaseSubrank(i32 amount)
{
    this->subRank += amount;
    while (this->subRank >= 100)
    {
        this->rank++;
        this->subRank -= 100;
    }
    if (this->rank > this->maxRank)
    {
        this->rank = this->maxRank;
    }
}

void GameManager::DecreaseSubrank(i32 amount)
{
    this->subRank -= amount;
    while (this->subRank < 0)
    {
        this->rank--;
        this->subRank += 100;
    }
    if (this->rank < this->minRank)
    {
        this->rank = this->minRank;
    }
}

GameManager::GameManager()
{
    memset(this, 0, sizeof(GameManager));

    this->gameRegionScreenPos.x = GAME_REGION_POS_X;
    this->gameRegionScreenPos.y = GAME_REGION_POS_Y;
    this->gameRegionSize.x = GAME_REGION_WIDTH;
    this->gameRegionSize.y = GAME_REGION_HEIGHT;
}
} // namespace th06

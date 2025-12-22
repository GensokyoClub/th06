#include "GameManager.hpp"
#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "ChainPriorities.hpp"
#include "EclManager.hpp"
#include "EffectManager.hpp"
#include "EnemyManager.hpp"
#include "GameWindow.hpp"
#include "Gui.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "ResultScreen.hpp"
#include "Rng.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "utils.hpp"

// #include <d3d8types.h>
// #include <d3dx8math.h>

u32 g_ExtraLivesScores[5] = {
    10000000,
    20000000,
    40000000,
    60000000,
    1900000000
};

const char *g_EclFiles[9] = { "dummy",
                              "data/ecldata1.ecl",
                              "data/ecldata2.ecl",
                              "data/ecldata3.ecl",
                              "data/ecldata4.ecl",
                              "data/ecldata5.ecl",
                              "data/ecldata6.ecl",
                              "data/ecldata7.ecl",
                              NULL };

struct AnmStageFiles {
    const char *file1;
    const char *file2;
};

AnmStageFiles g_AnmStageFiles[8] = {
    { "dummy", "dummy" },
    { "data/stg1enm.anm", "data/stg1enm2.anm" },
    { "data/stg2enm.anm", "data/stg2enm2.anm" },
    { "data/stg3enm.anm", NULL },
    { "data/stg4enm.anm", NULL },
    { "data/stg5enm.anm", "data/stg5enm2.anm" },
    { "data/stg6enm.anm", "data/stg6enm2.anm" },
    { "data/stg7enm.anm", "data/stg7enm2.anm" },
};
struct DifficultyInfo {
    u32 rank;
    u32 minRank;
    u32 maxRank;
};

DifficultyInfo g_DifficultyInfoForReplay[5] = {
    // rank, minRank, maxRank
    /* EASY    */ { 16, 12, 20 },
    /* NORMAL  */ { 16, 10, 32 },
    /* HARD    */ { 16, 10, 32 },
    /* LUNATIC */ { 16, 10, 32 },
    /* EXTRA   */ { 16, 14, 18 },
};

DifficultyInfo g_DifficultyInfo[5] = {
    // rank, minRank, maxRank
    /* EASY    */ { 16, 12, 20 },
    /* NORMAL  */ { 16, 10, 32 },
    /* HARD    */ { 16, 10, 32 },
    /* LUNATIC */ { 16, 10, 32 },
    /* EXTRA   */ { 16, 14, 18 },
};

// These are either on Supervisor.cpp or somewhere else
GameManager g_GameManager;

ChainElem g_GameManagerCalcChain;
ChainElem g_GameManagerDrawChain;

#define MAX_SCORE 999999999
#define MAX_CLEARS 99

#define DEMO_FADEOUT_FRAMES 3600
#define DEMO_FRAMES 3720

#define GUI_SCORE_STEP 78910

#define MAX_LIVES 8

i32 GameManager::IsInBounds(f32 x, f32 y, f32 width, f32 height) {
    if (width / 2.0f + x < 0.0f) {
        return false;
    }
    if ((x - width / 2.0f) > g_GameManager.arcadeRegionSize.x) {
        return false;
    }
    if (height / 2.0f + y < 0.0f) {
        return false;
    }
    if (y - height / 2.0f > g_GameManager.arcadeRegionSize.y) {
        return false;
    }

    return true;
}

ChainCallbackResult GameManager::OnUpdate(GameManager *gameManager) {
    u32 isInMenu;
    u32 scoreIncrement;

    if (gameManager->demoMode) {
        if (WAS_PRESSED(TH_BUTTON_ANY)) {
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        }
        gameManager->demoFrames++;
        if (gameManager->demoFrames == DEMO_FADEOUT_FRAMES) {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_FADE_OUT, 120, 0x000000,
                                        0, 0);
        }
        if (gameManager->demoFrames >= DEMO_FRAMES) {
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        }
    }
    if (!gameManager->isInRetryMenu && !gameManager->isInGameMenu &&
        !gameManager->demoMode && WAS_PRESSED(TH_BUTTON_MENU)) {
        gameManager->isInGameMenu = 1;
        g_GameManager.arcadeRegionTopLeftPos.x = GAME_REGION_LEFT;
        g_GameManager.arcadeRegionTopLeftPos.y = GAME_REGION_TOP;
        g_GameManager.arcadeRegionSize.x = GAME_REGION_WIDTH;
        g_GameManager.arcadeRegionSize.y = GAME_REGION_HEIGHT;
        g_Supervisor.unk198 = 3;
    }

    if (!gameManager->isInRetryMenu && !gameManager->isInGameMenu) {
        isInMenu = 1;
    } else {
        isInMenu = 0;
    }

    gameManager->isInMenu = isInMenu;

    g_Supervisor.viewport.x = gameManager->arcadeRegionTopLeftPos.x;
    g_Supervisor.viewport.y = gameManager->arcadeRegionTopLeftPos.y;
    g_Supervisor.viewport.width = gameManager->arcadeRegionSize.x;
    g_Supervisor.viewport.height = gameManager->arcadeRegionSize.y;
    g_Supervisor.viewport.minZ = 0.5;
    g_Supervisor.viewport.maxZ = 1.0;

    SetupCamera(0);

    g_Supervisor.viewport.Set();
    g_glFuncTable.glClearDepthf(1.0f);
    g_glFuncTable.glClear(GL_DEPTH_BUFFER_BIT);

    //    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    //    g_Supervisor.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER,
    //    g_Stage.skyFog.color, 1.0, 0);

    // Seems like gameManager->isInGameMenu was supposed to have 3 states, but
    // all the times it ends up checking both
    if (gameManager->isInGameMenu == 1 || gameManager->isInGameMenu == 2 ||
        gameManager->isInRetryMenu) {
        return CHAIN_CALLBACK_RESULT_BREAK;
    }

    if (gameManager->score >= MAX_SCORE + 1) {
        gameManager->score = MAX_SCORE - 9;
    }
    if (gameManager->guiScore != gameManager->score) {
        if (gameManager->score < gameManager->guiScore) {
            gameManager->score = gameManager->guiScore;
        }

        scoreIncrement = (gameManager->score - gameManager->guiScore) >> 5;
        if (scoreIncrement >= GUI_SCORE_STEP) {
            scoreIncrement = GUI_SCORE_STEP;
        } else if (scoreIncrement < 10) {
            scoreIncrement = 10;
        }
        scoreIncrement = scoreIncrement - scoreIncrement % 10;

        if (gameManager->nextScoreIncrement < scoreIncrement) {
            gameManager->nextScoreIncrement = scoreIncrement;
        }
        if (gameManager->guiScore + gameManager->nextScoreIncrement >
            gameManager->score) {
            gameManager->nextScoreIncrement =
                gameManager->score - gameManager->guiScore;
        }

        gameManager->guiScore += gameManager->nextScoreIncrement;
        if (gameManager->guiScore >= gameManager->score) {
            gameManager->nextScoreIncrement = 0;
            gameManager->guiScore = gameManager->score;
        }
        if (gameManager->extraLives >= 0 &&
            g_ExtraLivesScores[gameManager->extraLives] <=
                gameManager->guiScore) {
            if (gameManager->livesRemaining < MAX_LIVES) {
                gameManager->livesRemaining++;
                g_SoundPlayer.PlaySoundByIdx(SOUND_1UP);
            }
            g_Gui.flags.flag0 = 2;
            gameManager->extraLives++;
            g_GameManager.IncreaseSubrank(200);
        }
        if (gameManager->highScore < gameManager->guiScore) {
            gameManager->highScore = gameManager->guiScore;
        }
    }
    gameManager->gameFrames++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult GameManager::OnDraw(GameManager *gameManager) {
    if (gameManager->isInGameMenu) {
        gameManager->isInGameMenu = 2;
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

bool GameManager::RegisterChain() {
    GameManager *mgr = &g_GameManager;

    g_GameManagerCalcChain.callback = (ChainCallback)GameManager::OnUpdate;
    g_GameManagerCalcChain.addedCallback = NULL;
    g_GameManagerCalcChain.deletedCallback = NULL;
    g_GameManagerCalcChain.addedCallback =
        (ChainAddedCallback)GameManager::AddedCallback;
    g_GameManagerCalcChain.deletedCallback =
        (ChainDeletedCallback)GameManager::DeletedCallback;
    g_GameManagerCalcChain.arg = mgr;

    mgr->gameFrames = 0;

    if (!g_Chain.AddToCalcChain(&g_GameManagerCalcChain,
                                TH_CHAIN_PRIO_CALC_GAMEMANAGER)) {
        return false;
    }
    g_GameManagerDrawChain.callback = (ChainCallback)GameManager::OnDraw;
    g_GameManagerDrawChain.addedCallback = NULL;
    g_GameManagerDrawChain.deletedCallback = NULL;
    g_GameManagerDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_GameManagerDrawChain,
                           TH_CHAIN_PRIO_DRAW_GAMEMANAGER);
    return true;
}

bool GameManager::AddedCallback(GameManager *mgr) {
    ScoreDat *scoredat;
    u32 clrdIdx;
    u32 catkCursor;
    i32 i;
    Catk *catk;
    bool failedToLoadReplay;

    failedToLoadReplay = false;
    if (g_Supervisor.curState != SUPERVISOR_STATE_GAMEMANAGER_REINIT) {
        g_Supervisor.defaultConfig.bombCount = g_GameManager.bombsRemaining;
        g_Supervisor.defaultConfig.lifeCount = g_GameManager.livesRemaining;
        mgr->arcadeRegionTopLeftPos.x = 32.0;
        mgr->arcadeRegionTopLeftPos.y = 16.0;
        mgr->arcadeRegionSize.x = 384.0;
        mgr->arcadeRegionSize.y = 448.0;
        mgr->playerMovementAreaTopLeftPos.x = 8.0;
        mgr->playerMovementAreaTopLeftPos.y = 16.0;
        mgr->playerMovementAreaSize.x = 368.0;
        mgr->playerMovementAreaSize.y = 416.0;
        mgr->counat = 0;
        mgr->guiScore = 0;
        mgr->score = 0;
        mgr->nextScoreIncrement = 0;
        mgr->highScore = 100000;
        mgr->currentPower = 0;
        mgr->numRetries = 0;
        if (6 <= mgr->currentStage) {
            mgr->difficulty = EXTRA;
        }
        if (mgr->difficulty < EXTRA) {
            mgr->extraLives = 0;
        } else {
            mgr->extraLives = 4;
        }
        g_GameManager.powerItemCountForScore = 0;
        mgr->rank = 8;
        mgr->grazeInTotal = 0;
        mgr->pointItemsCollected = 0;
        for (catk = mgr->catk, i = 0; i < ARRAY_SIZE_SIGNED(mgr->catk);
             i++, catk++) {
            // Randomize catk content.
            for (catkCursor = 0; catkCursor < sizeof(Catk) / sizeof(u16);
                 catkCursor++) {
                ((u16 *)catk)[catkCursor] = g_Rng.GetRandomU16();
            }
            catk->base.magic = CATK_MAGIC;
            catk->base.unkLen = sizeof(Catk);
            catk->base.th6kLen = sizeof(Catk);
            catk->base.version = TH6K_VERSION;
            catk->idx = i;
            catk->numAttempts = 0;
            catk->numSuccess = 0;
        }
#ifdef __EMSCRIPTEN__
        scoredat = ResultScreen::OpenScore("/persistent/score.dat");
#else
        scoredat = ResultScreen::OpenScore("score.dat");
#endif
        g_GameManager.highScore = ResultScreen::GetHighScore(
            scoredat, NULL, g_GameManager.CharacterShotType(),
            g_GameManager.difficulty);
        ResultScreen::ParseCatk(scoredat, mgr->catk);
        ResultScreen::ParseClrd(scoredat, mgr->clrd);
        ResultScreen::ParsePscr(scoredat, (Pscr *)mgr->pscr);
        if (mgr->isInPracticeMode != 0) {
            g_GameManager.highScore =
                mgr->pscr[g_GameManager.CharacterShotType()]
                         [g_GameManager.currentStage][g_GameManager.difficulty]
                             .score;
        }
        ResultScreen::ReleaseScoreDat(scoredat);
        mgr->rank = g_DifficultyInfo[g_GameManager.difficulty].rank;
        mgr->minRank = g_DifficultyInfo[g_GameManager.difficulty].minRank;
        mgr->maxRank = g_DifficultyInfo[g_GameManager.difficulty].maxRank;
        mgr->deaths = 0;
        mgr->bombsUsed = 0;
        mgr->spellcardsCaptured = 0;
    } else {
        mgr->guiScore = mgr->score;
        mgr->nextScoreIncrement = 0;
    }
    mgr->subRank = 0;
    mgr->pointItemsCollectedInStage = 0;
    mgr->grazeInStage = 0;
    mgr->isInGameMenu = 0;
    mgr->currentStage = mgr->currentStage + 1;
    if (g_GameManager.isInReplay == 0) {
        clrdIdx = g_GameManager.CharacterShotType();
        if (mgr->numRetries == 0 &&
            mgr->clrd[clrdIdx]
                    .difficultyClearedWithRetries[g_GameManager.difficulty] <
                mgr->currentStage - 1) {
            mgr->clrd[clrdIdx]
                .difficultyClearedWithRetries[g_GameManager.difficulty] =
                mgr->currentStage - 1;
        }
        if (mgr->clrd[clrdIdx]
                .difficultyClearedWithoutRetries[g_GameManager.difficulty] <
            mgr->currentStage - 1) {
            mgr->clrd[clrdIdx]
                .difficultyClearedWithoutRetries[g_GameManager.difficulty] =
                mgr->currentStage - 1;
        }
    }
    if (mgr->isInPracticeMode != 0) {
        switch (mgr->currentStage) {
        case STAGE2:
            break;
        case STAGE3:
            mgr->currentPower = 64;
            break;
        default:
            mgr->currentPower = 128;
        }
    }
    if (g_GameManager.isInReplay == 1) {
        if (!ReplayManager::RegisterChain(1,
                                          (char *)g_GameManager.replayFile)) {
            failedToLoadReplay = true;
        }
        while (g_ExtraLivesScores[mgr->extraLives] <= mgr->guiScore) {
            mgr->extraLives++;
        }
        mgr->minRank =
            g_DifficultyInfoForReplay[g_GameManager.difficulty].minRank;
        mgr->maxRank =
            g_DifficultyInfoForReplay[g_GameManager.difficulty].maxRank;
    }
    g_Rng.generationCount = 0;
    mgr->randomSeed = g_Rng.seed;
    if (!Stage::RegisterChain(mgr->currentStage)) {
        GameErrorContext::Log(&g_GameErrorContext,
                              TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_STAGE);
        return false;
    }

    if (!Player::RegisterChain(0)) {
        GameErrorContext::Log(&g_GameErrorContext,
                              TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_PLAYER);
        return false;
    }
    if (!BulletManager::RegisterChain("data/etama.anm")) {
        GameErrorContext::Log(
            &g_GameErrorContext,
            TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_BULLETMANAGER);
        return false;
    }
    if (!EnemyManager::RegisterChain(
            g_AnmStageFiles[mgr->currentStage].file1,
            g_AnmStageFiles[mgr->currentStage].file2)) {
        GameErrorContext::Log(
            &g_GameErrorContext,
            TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ENEMYMANAGER);
        return false;
    }
    if (!g_EclManager.Load(g_EclFiles[mgr->currentStage])) {
        GameErrorContext::Log(
            &g_GameErrorContext,
            TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_ECLMANAGER);
        return false;
    }
    if (!EffectManager::RegisterChain()) {
        GameErrorContext::Log(
            &g_GameErrorContext,
            TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_EFFECTMANAGER);
        return false;
    }
    if (!Gui::RegisterChain()) {
        GameErrorContext::Log(&g_GameErrorContext,
                              TH_ERR_GAMEMANAGER_FAILED_TO_INITIALIZE_GUI);
        return false;
    }
    if (g_GameManager.isInReplay == 0) {
        ReplayManager::RegisterChain(0, "replay/th6_00.rpy");
    }
    if (g_GameManager.demoMode == 0) {
        // Read boss battle, and store it for use when boss is started.
        g_Supervisor.ReadMidiFile(1, g_Stage.stdData->songPaths[1]);
        // Immediately start playing this level's theme.
        g_Supervisor.PlayAudio(g_Stage.stdData->songPaths[0]);
    }
    mgr->isInRetryMenu = 0;
    mgr->isInMenu = 1;
    if (g_Supervisor.curState != SUPERVISOR_STATE_GAMEMANAGER_REINIT) {
        g_Supervisor.unk1b4 = 0.0;
        g_Supervisor.unk1b8 = 0.0;
    }
    mgr->isTimeStopped = false;
    mgr->score = 0;
    mgr->isGameCompleted = 0;
    g_AsciiManager.InitializeVms();
    if (failedToLoadReplay) {
        g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
    }
    g_Supervisor.unk198 = 3;
    return true;
}

bool GameManager::DeletedCallback(GameManager *mgr) {
    if (!g_GameManager.demoMode) {
        g_Supervisor.StopAudio();
    }
    Stage::CutChain();
    BulletManager::CutChain();
    Player::CutChain();
    EnemyManager::CutChain();
    g_EclManager.Unload();
    EffectManager::CutChain();
    Gui::CutChain();
    ReplayManager::StopRecording();
    mgr->isInMenu = 0;
    g_AsciiManager.InitializeVms();
    return true;
}

void GameManager::CutChain() {
    g_Chain.Cut(&g_GameManagerCalcChain);
    g_Chain.Cut(&g_GameManagerDrawChain);
}

void GameManager::SetupCameraStageBackground(f32 extraRenderDistance) {
    ZunVec3 eyeVec;
    ZunVec3 atVec;
    ZunVec3 upVec;
    f32 fov;
    f32 aspectRatio;
    f32 viewportMiddleWidth;
    f32 viewportMiddleHeight;
    f32 cameraDistance;

    g_AnmManager->SetProjectionMode(PROJECTION_MODE_PERSPECTIVE);

    viewportMiddleWidth = g_Supervisor.viewport.width / 2.0f;
    viewportMiddleHeight = g_Supervisor.viewport.height / 2.0f;
    aspectRatio =
        (f32)g_Supervisor.viewport.width / (f32)g_Supervisor.viewport.height;
    fov = ZUN_PI * (30.0f / 180.0f);
    cameraDistance = viewportMiddleHeight / ZUN_TANF(fov / 2);
    upVec.x = 0.0f;
    upVec.y = 1.0f;
    upVec.z = 0.0f;
    atVec.x = viewportMiddleWidth;
    atVec.y = -viewportMiddleHeight;
    atVec.z = 0.0f;
    eyeVec.x = viewportMiddleWidth;
    eyeVec.y = -viewportMiddleHeight;
    eyeVec.z = -cameraDistance;
    ZunMatrix viewMatrix = createViewMatrix(eyeVec, atVec, upVec);
    g_AnmManager->SetTransformMatrix(MATRIX_VIEW, viewMatrix);
    g_Supervisor.viewMatrix = viewMatrix;

    g_GameManager.cameraDistance = ZUN_FABSF(cameraDistance);

    ZunMatrix perspectiveMatrix = perspectiveMatrixFromFOV(
        fov, aspectRatio, 100.0f, 10000.0f + extraRenderDistance);
    g_AnmManager->SetTransformMatrix(MATRIX_PROJECTION, perspectiveMatrix);
    g_Supervisor.projectionMatrix = perspectiveMatrix;
    return;
}

void GameManager::SetupCamera(f32 extraRenderDistance) {
    ZunVec3 eyeVec;
    ZunVec3 atVec;
    ZunVec3 upVec;
    f32 fov;
    f32 aspectRatio;
    f32 viewportMiddleWidth;
    f32 viewportMiddleHeight;
    f32 cameraDistance;

    f32 atVecY;
    f32 atVecX;
    f32 eyeVecZ;

    g_AnmManager->SetProjectionMode(PROJECTION_MODE_PERSPECTIVE);

    viewportMiddleWidth = g_Supervisor.viewport.width / 2.0f;
    viewportMiddleHeight = g_Supervisor.viewport.height / 2.0f;
    aspectRatio =
        (f32)g_Supervisor.viewport.width / (f32)g_Supervisor.viewport.height;
    fov = ZUN_PI * (30.0f / 180.0f);
    cameraDistance = viewportMiddleHeight / ZUN_TANF(fov / 2);
    upVec.x = 0.0f;
    upVec.y = 1.0f;
    upVec.z = 0.0f;
    atVecY = -viewportMiddleHeight + (f32)g_GameManager.stageCameraFacingDir.y;
    atVecX = viewportMiddleWidth + (f32)g_GameManager.stageCameraFacingDir.x;
    atVec.x = atVecX;
    atVec.y = atVecY;
    atVec.z = 0;
    eyeVecZ = -cameraDistance * (f32)g_GameManager.stageCameraFacingDir.z;
    eyeVec.x = viewportMiddleWidth;
    eyeVec.y = -viewportMiddleHeight;
    eyeVec.z = eyeVecZ;

    ZunMatrix viewMatrix = createViewMatrix(eyeVec, atVec, upVec);
    g_AnmManager->SetTransformMatrix(MATRIX_VIEW, viewMatrix);
    g_Supervisor.viewMatrix = viewMatrix;

    g_GameManager.cameraDistance = ZUN_FABSF(cameraDistance);

    ZunMatrix perspectiveMatrix = perspectiveMatrixFromFOV(
        fov, aspectRatio, 100.0f, 10000.0f + extraRenderDistance);
    g_AnmManager->SetTransformMatrix(MATRIX_PROJECTION, perspectiveMatrix);
    g_Supervisor.projectionMatrix = perspectiveMatrix;

    return;
}

void GameManager::IncreaseSubrank(i32 amount) {
    this->subRank = this->subRank + amount;
    while (this->subRank >= 100) {
        this->rank++;
        this->subRank -= 100;
    }
    if (this->rank > this->maxRank) {
        this->rank = this->maxRank;
    }
}

void GameManager::DecreaseSubrank(i32 amount) {
    this->subRank = this->subRank - amount;
    while (this->subRank < 0) {
        this->rank--;
        this->subRank += 100;
    }
    if (this->rank < this->minRank) {
        this->rank = this->minRank;
    }
}

GameManager::GameManager() {

    memset(this, 0, sizeof(GameManager));

    (this->arcadeRegionTopLeftPos).x = GAME_REGION_LEFT;
    (this->arcadeRegionTopLeftPos).y = GAME_REGION_TOP;
    (this->arcadeRegionSize).x = GAME_REGION_WIDTH;
    (this->arcadeRegionSize).y = GAME_REGION_HEIGHT;
}

i32 GameManager::HasReachedMaxClears(i32 character, i32 shottype) {
    return (
        this->clrd[shottype + character * 2].difficultyClearedWithRetries[1] ==
            MAX_CLEARS ||
        this->clrd[shottype + character * 2].difficultyClearedWithRetries[2] ==
            MAX_CLEARS ||
        this->clrd[shottype + character * 2].difficultyClearedWithRetries[3] ==
            MAX_CLEARS);
}

#pragma once
#include <Windows.h>
#include <d3d8.h>
#include <d3dx8math.h>

#include "Chain.hpp"
#include "Global.hpp"
#include "ResultScreen.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

namespace th06
{
enum Difficulty
{
    EASY,
    NORMAL,
    HARD,
    LUNATIC,
    EXTRA,
};

enum StageNumber
{
    DummyStage,
    Stage1,
    Stage2,
    Stage3,
    Stage4,
    Stage5,
    Stage6,
    ExtraStage,
};

#define STAGE_COUNT 7
#define MAIN_GAME_STAGE_COUNT (MAIN_GAME_STAGE_COUNT - 1)

#define STAGE_INDEX(num) ((num) - 1)

#define MAX_POWER 128

#define MAX_CONTINUE_CREDITS 3

struct GameManager;

extern GameManager g_GameManager;
struct GameManager
{
    GameManager();

    ZunBool HasExtraUnlocked(i32 character, i32 shottype)
    {
        return this->clrd[shottype + character * SHOTTYPES_PER_CHARACTER].stagesClearedWithoutContinues[NORMAL] ==
                   ALL_CLEARED ||
               this->clrd[shottype + character * SHOTTYPES_PER_CHARACTER].stagesClearedWithoutContinues[HARD] ==
                   ALL_CLEARED ||
               this->clrd[shottype + character * SHOTTYPES_PER_CHARACTER].stagesClearedWithoutContinues[LUNATIC] ==
                   ALL_CLEARED;
    }
    i32 GetLives()
    {
        return this->livesRemaining;
    }

    i32 GetBombs()
    {
        return this->bombsRemaining;
    }

    void IncreaseSubrank(i32 amount);
    void DecreaseSubrank(i32 amount);
    ZunBool IsInBounds(f32 x, f32 y, f32 width, f32 height);

    void AddScore(i32 points)
    {
        this->score += points;
    }

    i32 RankLerpInt(i32 minVal, i32 maxVal)
    {
        return this->rank * (maxVal - minVal) / 32 + minVal;
    }

    float RankLerpFloat(float minVal, float maxVal)
    {
        return this->rank * (maxVal - minVal) / 32.0f + minVal;
    }

    u32 guiScore;
    u32 score;
    u32 nextScoreIncrement;
    u32 highScore;
    i32 difficulty;
    i32 grazeInStage;
    i32 grazeInTotal;
    ZunBool isInReplay;
    i32 deaths;
    i32 bombsUsed;
    i32 spellcardsCaptured;
    i8 isTimeStopped;
    alignment_padding(0x3);
    Catk catk[CATK_COUNT];
    Clrd clrd[SHOTTYPE_COUNT];
    Pscr pscr[SHOTTYPE_COUNT][PSCR_NUM_STAGES][PSCR_NUM_DIFFICULTIES];
    u16 currentPower;
    unreferenced_fields(0x2);
    u16 pointItemsCollectedInStage;
    u16 pointItemsCollected;
    u8 continuesUsed;
    i8 powerItemCountForScore;
    i8 livesRemaining;
    i8 bombsRemaining;
    i8 livesEarned;
    u8 character;
    u8 shotType;
    u8 isInGameMenu;
    u8 isInRetryMenu;
    u8 isInMenu;
    u8 isGameCompleted;
    u8 isInPracticeMode;
    u8 demoMode;
    alignment_padding(0x3);
    i32 demoFrames;
    char replayFile[256];
    unused_array_field(char, 256);
    u16 randomSeed;
    u32 gameFrames;
    i32 currentStage;
    u32 menuCursorBackup;
    ZunVec2 gameRegionScreenPos;
    ZunVec2 gameRegionSize;
    ZunVec2 playerMovementAreaTopLeftPos;
    ZunVec2 playerMovementAreaSize;
    f32 cameraDistance;
    D3DXVECTOR3 stageCameraFacingDir;
    i32 gameplayTicks;
    i32 rank;
    i32 maxRank;
    i32 minRank;
    i32 subRank;
};

#define CURRENT_SHOTTYPE() SHOTTYPE_ID(g_GameManager.character, g_GameManager.shotType)

ZunResult GameManager_RegisterChain();
void GameManager_CutChain();
void SetupCamera(f32 extraRenderDistance);
void SetupCameraStageBackground(f32 extraRenderDistance);

ZUN_ASSERT_TYPE(GameManager, 0x1a80, 4);
} // namespace th06

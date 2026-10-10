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
    STAGE1,
    STAGE2,
    STAGE3,
    STAGE4,
    STAGE5,
    FINAL_STAGE,
    EXTRA_STAGE,
};

#define GAME_REGION_POS_X 32.0f
#define GAME_REGION_POS_Y 16.0f

#define GAME_REGION_WIDTH 384.0f
#define GAME_REGION_HEIGHT 448.0f

#define GAME_REGION_POS_RIGHT (GAME_REGION_POS_X + GAME_REGION_WIDTH)
#define GAME_REGION_POS_BOTTOM (GAME_REGION_POS_Y + GAME_REGION_HEIGHT)

#define GAME_REGION_LEFT 0.0f
#define GAME_REGION_TOP 0.0f

#define GAME_REGION_RIGHT (GAME_REGION_LEFT + GAME_REGION_WIDTH)
#define GAME_REGION_BOTTOM (GAME_REGION_TOP + GAME_REGION_HEIGHT)

#define MAX_CLEARS 99

#define MAX_POWER 128

struct GameManager;

extern GameManager g_GameManager;
struct GameManager
{
    GameManager();

    ZunBool HasReachedMaxClears(i32 character, i32 shottype)
    {
        return this->clrd[shottype + character * SHOTTYPES_PER_CHARACTER].difficultyClearedWithRetries[1] ==
                   MAX_CLEARS ||
               this->clrd[shottype + character * SHOTTYPES_PER_CHARACTER].difficultyClearedWithRetries[2] ==
                   MAX_CLEARS ||
               this->clrd[shottype + character * SHOTTYPES_PER_CHARACTER].difficultyClearedWithRetries[3] == MAX_CLEARS;
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
    Difficulty difficulty;
    i32 grazeInStage;
    i32 grazeInTotal;
    ZunBool isInReplay;
    i32 deaths;
    i32 bombsUsed;
    i32 spellcardsCaptured;
    i8 isTimeStopped;
    Catk catk[CATK_NUM_CAPTURES];
    Clrd clrd[SHOTTYPE_COUNT];
    Pscr pscr[SHOTTYPE_COUNT][PSCR_NUM_STAGES][PSCR_NUM_DIFFICULTIES];
    u16 currentPower;
    unreferenced_fields(0x2);
    u16 pointItemsCollectedInStage;
    u16 pointItemsCollected;
    u8 numRetries;
    i8 powerItemCountForScore;
    i8 livesRemaining;
    i8 bombsRemaining;
    i8 extraLives;
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
    i32 counat;
    i32 rank;
    i32 maxRank;
    i32 minRank;
    i32 subRank;
};
ZunResult GameManager_RegisterChain();
void GameManager_CutChain();
void GameManager_SetupCamera(f32 extraRenderDistance);
void GameManager_SetupCameraStageBackground(f32 extraRenderDistance);

inline i32 GameManager_CharacterShotType()
{
    return g_GameManager.shotType + g_GameManager.character * SHOTTYPES_PER_CHARACTER;
}

ZUN_ASSERT_TYPE(GameManager, 0x1a80, 4);
} // namespace th06

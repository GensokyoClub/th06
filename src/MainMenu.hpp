#pragma once
#include "AnmVm.hpp"
#include "ReplayData.hpp"
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

#define REPLAYS_PER_PAGE 15
#define NORMAL_REPLAY_COUNT REPLAYS_PER_PAGE
#define USER_REPLAY_PAGES 3
#define USER_REPLAY_COUNT (USER_REPLAY_PAGES * REPLAYS_PER_PAGE)
#define TOTAL_REPLAY_COUNT (NORMAL_REPLAY_COUNT + USER_REPLAY_COUNT)

namespace th06
{
ZunResult MainMenu_RegisterChain(ZunBool isDemo);
enum GameState
{
    STATE_STARTUP,
    STATE_PRE_INPUT,
    STATE_MAIN_MENU,
    STATE_OPTIONS,
    STATE_QUIT,
    STATE_KEYCONFIG,
    STATE_DIFFICULTY_LOAD,
    STATE_DIFFICULTY_SELECT,
    STATE_CHARACTER_LOAD,
    STATE_CHARACTER_SELECT,
    STATE_SCORE,
    STATE_SHOT_SELECT,
    STATE_REPLAY_LOAD,
    STATE_REPLAY_ANIM,
    STATE_REPLAY_UNLOAD,
    STATE_REPLAY_SELECT,
    STATE_MUSIC_ROOM,
    STATE_PRACTICE_LVL_SELECT,
};

enum CursorMovement
{
    CURSOR_MOVE_UP = -1,
    CURSOR_DONT_MOVE = 0,
    CURSOR_MOVE_DOWN = 1,
};

enum OptionsCursorPosition
{
    CURSOR_OPTIONS_POS_LIFECOUNT,
    CURSOR_OPTIONS_POS_BOMBCOUNT,
    CURSOR_OPTIONS_POS_COLORMODE,
    CURSOR_OPTIONS_POS_MUSICMODE,
    CURSOR_OPTIONS_POS_PLAYSOUNDS,
    CURSOR_OPTIONS_POS_SCREENMODE,
    CURSOR_OPTIONS_POS_SETDEFAULT,
    CURSOR_OPTIONS_POS_KEYCONFIG,
    CURSOR_OPTIONS_POS_EXIT,
};

struct MainMenu
{
    ZunResult BeginStartup();
    ZunResult DrawStartMenu();
    u32 OnUpdateOptionsMenu();
    ZunResult DrawReplayMenu();
    ZunResult ChoosePracticeLevel();
    ZunBool WeirdSecondInputCheck();
    void ColorMenuItem(AnmVm *, i32, i32, i32);

    i32 ReplayHandling();

#if BUILT_SINCE(102)
    AnmVm vm[122];
#else
    AnmVm vm[118];
#endif
    i32 cursor;
    unreferenced_fields(0x40);
    u32 unk_81e4;
    i32 chosenReplay;
    i32 replayFilesNum;
    GameState gameState;
    i32 stateTimer;
    i32 idleFrames;
    D3DCOLOR minimumOpacity;
    D3DCOLOR menuTextColor;
    D3DCOLOR color2;
    D3DCOLOR color1;
    i32 numFramesSinceActive;
    u32 framesActive;
    u32 framesInactive;
    unreferenced_fields(0x4);
    ControllerMapping controlMapping;
    unreferenced_fields(sizeof(ControllerButton));
    u8 colorMode16bit;
    u8 windowed;
    u8 frameskipConfig;
    alignment_padding(0x1);
    ChainElem *chainCalc;
    ChainElem *chainDraw;
    char replayFilePaths[TOTAL_REPLAY_COUNT][512];
    char replayFileName[TOTAL_REPLAY_COUNT][8];
    ReplayData replayFileData[TOTAL_REPLAY_COUNT];
    ReplayData *currentReplay;
    i32 timeRelatedArrSize;
    f32 timeRelatedArr[16];
    unreferenced_fields(0x4);
    u32 unk_10f28;
    i32 frameCountForRefreshRateCalc;
    u32 lastFrameTime;
};
#if BUILT_SINCE(102)
ZUN_ASSERT_TYPE(MainMenu, 0x10f34, 4);
#else
ZUN_ASSERT_TYPE(MainMenu, 0x10b04, 4);
#endif

} // namespace th06

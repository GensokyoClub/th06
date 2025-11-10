#pragma once

// #include <D3D8.h>

#include "AnmVm.hpp"
#include "Chain.hpp"
#include "ReplayData.hpp"
#include "ZunColor.hpp"
#include "inttypes.hpp"

namespace th06
{
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
    MainMenu();
    bool BeginStartup();
    bool DrawStartMenu();
    u32 OnUpdateOptionsMenu();
    bool DrawReplayMenu();
    bool ChoosePracticeLevel();
    bool WeirdSecondInputCheck();
    void ColorMenuItem(AnmVm *, i32, i32, i32);

    static bool LoadTitleAnm(MainMenu *menu);
    static CursorMovement MoveCursor(MainMenu *menu, i32 menuLength);
    static void DrawMenuItem(AnmVm *vm, i32 itemNumber, i32 cursor, ZunColor activeItemColor,
                             ZunColor inactiveItemColor, i32 spriteIdx /* I think*/);
    static void SwapMapping(MainMenu *menu, i16 btnPressed, i16 oldMapping, bool unk);

    i32 ReplayHandling();
    static bool LoadReplayMenu(MainMenu *menu);

    static bool RegisterChain(u32 isDemo);
    static ChainCallbackResult OnUpdate(MainMenu *s);
    static ChainCallbackResult OnDraw(MainMenu *s);
    static bool AddedCallback(MainMenu *s);
    static bool DeletedCallback(MainMenu *s);
    static bool LoadDiffCharSelect(MainMenu *s);

    static void ReleaseTitleAnm();

    AnmVm vm[122];
    i32 cursor;
    // i8 padding[0x40];
    u32 unk_81e4;
    i32 chosenReplay;
    i32 replayFilesNum;
    GameState gameState;
    i32 stateTimer;
    i32 idleFrames;
    ZunColor minimumOpacity;
    ZunColor menuTextColor;
    ZunColor color2;
    ZunColor color1;
    i32 numFramesSinceActive;
    u32 framesActive;
    u32 framesInactive;
    i8 padding2[4];
    i16 controlMapping[9];
    i8 padding3[2];
    u8 colorMode16bit;
    u8 windowed;
    u8 frameskipConfig;
    i8 padding4;
    ChainElem *chainCalc;
    ChainElem *chainDraw;
    char replayFilePaths[60][512];
    char replayFileName[60][8];
    ReplayData replayFileData[60];
    ReplayData *currentReplay;
    i32 timeRelatedArrSize;
    f32 timeRelatedArr[16];
    u32 unk_10f24;
    u32 unk_10f28;
    i32 frameCountForRefreshRateCalc;
    u32 lastFrameTime;
};
ZUN_ASSERT_SIZE(MainMenu, 0x10f34);

extern MainMenu g_MainMenu;
}; // namespace th06

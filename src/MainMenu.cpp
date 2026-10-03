#include <D3DX8.h>
#include <direct.h>
#include <stdio.h>
#include <windows.h>

#include "MainMenu.hpp"

#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "ChainPriorities.hpp"
#include "GameManager.hpp"
#include "GameWindow.hpp"
#include "Global.hpp"
#include "Player.hpp"
#include "ReplayData.hpp"
#include "ReplayManager.hpp"
#include "ResultScreen.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Supervisor.hpp"
#include "ZunColor.hpp"
#include "ZunTimer.hpp"
#include "i18n.hpp"

namespace th06
{
// This is the final section, so force it into the same group as
// all the library stuff to prevent an extra 16 bytes of padding
FILE_BSS_SORT(zzzzzzzzzz);

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

    static ZunResult LoadTitleAnm(MainMenu *menu);

    i32 ReplayHandling();
    static ZunResult LoadReplayMenu(MainMenu *menu);

    static ChainCallbackResult OnUpdate(MainMenu *s);
    static ChainCallbackResult OnDraw(MainMenu *s);
    static ZunResult AddedCallback(MainMenu *s);
    static ZunResult DeletedCallback(MainMenu *s);
    static ZunResult LoadDiffCharSelect(MainMenu *s);

    static void ReleaseTitleAnm();

    AnmVm vm[122];
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
    unreferenced_fields(0x2);
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
ZUN_ASSERT_TYPE(MainMenu, 0x10f34, 4);

static CursorMovement MoveCursor(MainMenu *menu, i32 menuLength);
static void SwapMapping(MainMenu *menu, i16 btnPressed, i16 oldMapping, ZunBool unk);
static void DrawMenuItem(AnmVm *vm, i32 itemNumber, i32 cursor, D3DCOLOR activeItemColor, D3DCOLOR inactiveItemColor,
                         i32 spriteIdx /* I think*/);

DIFFABLE_STATIC(MainMenu, g_MainMenu);

DIFFABLE_STATIC_ASSIGN(i16, g_LastJoystickInput) = TH_BUTTON_DOWN; // why???

#define MENU_VMS_DIFFICULTY_SELECT 81
#define MENU_VMS_CHARACTER_SELECT 86
#define MENU_VMS_SHOTTYPE_SELECT 92

#pragma var_order(i, vmList, time, deltaTime, deltaTimeAsFrames, deltaTimeAsMs, mapping, startedUp)
ChainCallbackResult MainMenu::OnUpdate(MainMenu *menu)
{
    i32 i;
    AnmVm *vmList;
    DWORD time;
    i32 deltaTime;
    f32 deltaTimeAsFrames;
    f32 deltaTimeAsMs;
    i16 mapping;
    ZunResult startedUp;

    if (menu->timeRelatedArrSize < ARRAY_SIZE_SIGNED(menu->timeRelatedArr))
    {
        timeBeginPeriod(1);
        if (menu->lastFrameTime == 0)
        {
            menu->lastFrameTime = timeGetTime();
        }
        time = timeGetTime();
        timeEndPeriod(1);
        menu->frameCountForRefreshRateCalc++;
        deltaTime = time - menu->lastFrameTime;
        if (deltaTime >= 700)
        {
            menu->lastFrameTime = time;
            menu->frameCountForRefreshRateCalc = 0;
        }
        else
        {
            if (deltaTime >= 500)
            {
                deltaTimeAsMs = deltaTime / 1000.0f;
                deltaTimeAsFrames = menu->frameCountForRefreshRateCalc * 1000.0f / deltaTime;
                if (deltaTimeAsFrames >= 57.0f)
                {
                    menu->timeRelatedArr[menu->timeRelatedArrSize] = deltaTimeAsFrames;
                    menu->timeRelatedArrSize++;
                }
                menu->lastFrameTime = time;
                menu->frameCountForRefreshRateCalc = 0;
            }
        }
    }
    switch (menu->gameState)
    {
    case STATE_STARTUP:
        startedUp = menu->BeginStartup();
        if (startedUp == ZUN_ERROR)
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        // no break
    case STATE_PRE_INPUT:
#if !TRIALBUILD
        menu->idleFrames++;
        if ((g_CurFrameInput & 0xffff) != 0)
        {
            menu->idleFrames = 0;
        }
        if (menu->idleFrames >= 720)
        {
            goto load_menu_rpy;
        }
#endif
        if (menu->WeirdSecondInputCheck())
            break;
        menu->idleFrames = 0;
    case STATE_MAIN_MENU:
        menu->DrawStartMenu();
#if !TRIALBUILD
        if ((g_CurFrameInput & 0xffff) != 0)
        {
            menu->idleFrames = 0;
        }
        menu->idleFrames++;
        if (menu->idleFrames >= 720)
        {
        load_menu_rpy:
            g_GameManager.isInReplay = true;
            g_GameManager.demoMode = true;
            g_GameManager.demoFrames = 0;
            g_Supervisor.framerateMultiplier = 1.0f;
            strcpy(g_GameManager.replayFile, "data/demo/demo00.rpy");
            g_GameManager.currentStage = 3;
            g_GameManager.difficulty = LUNATIC;
            g_Supervisor.curState = SUPERVISOR_STATE_GAMEMANAGER;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
#endif
        break;
    case STATE_REPLAY_LOAD:
    case STATE_REPLAY_ANIM:
    case STATE_REPLAY_UNLOAD:
    case STATE_REPLAY_SELECT:
        if (menu->ReplayHandling() != 0)
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    case STATE_OPTIONS:
        if (menu->OnUpdateOptionsMenu() != 0)
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    case STATE_KEYCONFIG:
        MoveCursor(menu, 11);
        vmList = &menu->vm[34];
        for (i = 0; i < 11; i++, vmList++)
        {
            DrawMenuItem(vmList, i, menu->cursor, menu->color2, menu->color1, 115);
        }
        for (i = 0; i < 9; i++, vmList++)
        {
            if (((i16 *)&menu->controlMapping)[i] < 0)
            {
                vmList->flags.isVisibleOverride = false;
                continue;
            }
            vmList->flags.isVisibleOverride = true;
            DrawMenuItem(vmList, i, menu->cursor, menu->color2, menu->color1, 115);
        }
        for (i = 0; i < 18; i++, vmList++)
        {
            if (((i16 *)&menu->controlMapping)[i / 2] < 0)
            {
                vmList->flags.isVisibleOverride = false;
                continue;
            }
            vmList->flags.isVisibleOverride = true;
            mapping = ((i16 *)&menu->controlMapping)[i / 2];
            if (i % 2 == 0)
            {
                g_AnmManager->SetActiveSprite(vmList, mapping / 10 + ANM_SPRITE_TITLE01_START);
            }
            else
            {
                g_AnmManager->SetActiveSprite(vmList, mapping % 10 + ANM_SPRITE_TITLE01_START);
            }
            vmList->baseSpriteIndex = vmList->activeSpriteIndex;
            DrawMenuItem(vmList, i / 2, menu->cursor, menu->color2, menu->color1, ARRAY_SIZE_SIGNED(menu->vm));
        }
        if (menu->stateTimer >= 32)
#pragma var_order(idx, controllerData)
        {
            i16 idx;
            u8 *controllerData = Controller::GetControllerState();
            for (idx = 0; idx < 32; idx++)
            {
                if (controllerData[idx] & 0x80)
                    break;
            }
            if (idx < 32 && g_LastJoystickInput != idx)
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                switch (menu->cursor)
                {
                case 0:
                    SwapMapping(menu, idx, menu->controlMapping.shootButton, true);
                    menu->controlMapping.shootButton = idx;
                    break;
                case 1:
                    SwapMapping(menu, idx, menu->controlMapping.bombButton, false);
                    menu->controlMapping.bombButton = idx;
                    break;
                case 2:
                    SwapMapping(menu, idx, menu->controlMapping.focusButton, true);
                    menu->controlMapping.focusButton = idx;
                    break;
                case 3:
                    SwapMapping(menu, idx, menu->controlMapping.menuButton, false);
                    menu->controlMapping.menuButton = idx;
                    break;
                case 4:
                    SwapMapping(menu, idx, menu->controlMapping.upButton, false);
                    menu->controlMapping.upButton = idx;
                    break;
                case 5:
                    SwapMapping(menu, idx, menu->controlMapping.downButton, false);
                    menu->controlMapping.downButton = idx;
                    break;
                case 6:
                    SwapMapping(menu, idx, menu->controlMapping.leftButton, false);
                    menu->controlMapping.leftButton = idx;
                    break;
                case 7:
                    SwapMapping(menu, idx, menu->controlMapping.rightButton, false);
                    menu->controlMapping.rightButton = idx;
                    break;
                case 8:
                    SwapMapping(menu, idx, menu->controlMapping.skipButton, false);
                    menu->controlMapping.skipButton = idx;
                }
            }
            g_LastJoystickInput = idx;
            if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
            {
                switch (menu->cursor)
                {
                case 9: {
                    ControllerMapping mappingData;
                    mappingData.shootButton = 0;
                    mappingData.bombButton = 1;
                    mappingData.focusButton = 0;
                    mappingData.menuButton = 0xffff;
                    mappingData.upButton = 0xffff;
                    mappingData.downButton = 0xffff;
                    mappingData.leftButton = 0xffff;
                    mappingData.rightButton = 0xffff;
                    mappingData.skipButton = 0xffff;
                    menu->controlMapping = mappingData;
                    break;
                }
                case 10:
                    menu->gameState = STATE_OPTIONS;
                    menu->stateTimer = 0;
                    for (idx = 0; idx < ARRAY_SIZE_SIGNED(menu->vm); idx++)
                    {
                        menu->vm[idx].pendingInterrupt = 3;
                    }
                    menu->cursor = 7;
                    g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
                    g_ControllerMapping = menu->controlMapping;
                    g_Supervisor.cfg.controllerMapping = menu->controlMapping;
                    break;
                }
            }
        }
        break;
    case STATE_DIFFICULTY_LOAD:
        if (menu->stateTimer == 60)
        {
            if (LoadDiffCharSelect(menu) != ZUN_SUCCESS)
            {
                g_GameErrorContext.Log(TH_ERR_MAINMENU_LOAD_SELECT_SCREEN_FAILED);
                g_Supervisor.curState = SUPERVISOR_STATE_EXITSUCCESS;
                return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
            }
            menu->gameState = STATE_DIFFICULTY_SELECT;
            menu->minimumOpacity = 0;
            menu->framesInactive = menu->framesActive;
            menu->framesActive = 0;
            if (g_GameManager.difficulty < EXTRA)
            {
                for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
                {
                    menu->vm[i].pendingInterrupt = 6;
                }
                menu->cursor = g_Supervisor.cfg.defaultDifficulty;
            }
            else
            {
                for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
                {
                    menu->vm[i].pendingInterrupt = 18;
                }
                menu->cursor = 0;
            }
        }
        else
        {
            break;
        }
    case STATE_CHARACTER_LOAD:
        if (menu->stateTimer == 36)
        {
            menu->gameState = STATE_STARTUP;
            menu->stateTimer = 0;
        }
        break;
    case STATE_DIFFICULTY_SELECT:
        vmList = &menu->vm[MENU_VMS_DIFFICULTY_SELECT];
        if (g_GameManager.difficulty < EXTRA)
        {
            MoveCursor(menu, 4);
            for (i = 0; i < 4; i++, vmList++)
            {
                if (i != menu->cursor)
                {
                    if (!g_Supervisor.IsHardwareBlendingDisabled())
                    {
                        vmList->color = 0x60000000;
                    }
                    else
                    {
                        vmList->color = 0x60ffffff;
                    }
                    vmList->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                    vmList->alphaInterpEndTime = 0;
                }
                else
                {
                    if (!g_Supervisor.IsHardwareBlendingDisabled())
                    {
                        vmList->color = COLOR_BLACK;
                    }
                    else
                    {
                        vmList->color = COLOR_WHITE;
                    }
                    vmList->posOffset = D3DXVECTOR3(-6.0f, -6.0f, 0.0f);
                }
            }
            vmList->flags.isVisibleOverride = false;
        }
        else
        {
            for (i = 0; i < 4; i++, vmList++)
            {
                vmList->flags.isVisibleOverride = false;
            }
            for (i = 4; i < 5; i++, vmList++)
            {
                if (!g_Supervisor.IsHardwareBlendingDisabled())
                {
                    vmList->color = COLOR_BLACK;
                }
                else
                {
                    vmList->color = COLOR_WHITE;
                }
                vmList->posOffset = D3DXVECTOR3(-6.0f, -6.0f, 0.0f);
            }
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            menu->gameState = STATE_CHARACTER_LOAD;
            menu->stateTimer = 0;
            for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
            {
                menu->vm[i].pendingInterrupt = 4;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            if (g_GameManager.difficulty < EXTRA)
            {
                g_Supervisor.cfg.defaultDifficulty = menu->cursor;
                if (!g_GameManager.isInPracticeMode)
                {
                    menu->cursor = 0;
                }
                else
                {
                    menu->cursor = 2;
                }
            }
            else
            {
                menu->cursor = 1;
            }
            break;
        }
        else if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            menu->gameState = STATE_CHARACTER_SELECT;
            menu->stateTimer = 0;
            for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
            {
                menu->vm[i].pendingInterrupt = 7;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
            if (g_GameManager.difficulty < EXTRA)
            {
                vmList = &menu->vm[MENU_VMS_DIFFICULTY_SELECT + menu->cursor];
                vmList->pendingInterrupt = 8;
                g_GameManager.difficulty = (Difficulty)menu->cursor;
                menu->cursor = g_GameManager.character;
            }
            else
            {
                vmList = &menu->vm[MENU_VMS_DIFFICULTY_SELECT + EXTRA];
                vmList->pendingInterrupt = 8;
                g_GameManager.difficulty = EXTRA;
                if (g_GameManager.HasReachedMaxClears(g_GameManager.character, SHOT_TYPE_A) ||
                    g_GameManager.HasReachedMaxClears(g_GameManager.character, SHOT_TYPE_B))
                {
                    menu->cursor = g_GameManager.character;
                }
                else
                {
                    menu->cursor = 1 - g_GameManager.character;
                }
            }
            g_Supervisor.cfg.defaultDifficulty = g_GameManager.difficulty;
            vmList = &menu->vm[MENU_VMS_CHARACTER_SELECT];
            for (i = 0; i < CHARACTER_COUNT; i++, vmList += 2)
            {
                if (i != menu->cursor)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            break;
        }
        break;
    case STATE_CHARACTER_SELECT:
        if (menu->stateTimer < 30)
            break;
        if (WAS_PRESSED_REPEATING(TH_BUTTON_LEFT))
        {
            menu->cursor++;
            if (menu->cursor >= CHARACTER_COUNT)
            {
                menu->cursor -= CHARACTER_COUNT;
            }
            if (g_GameManager.difficulty == EXTRA && !g_GameManager.HasReachedMaxClears(menu->cursor, SHOT_TYPE_A) &&
                !g_GameManager.HasReachedMaxClears(menu->cursor, SHOT_TYPE_B))
            {
                menu->cursor--;
                if (menu->cursor < 0)
                {
                    menu->cursor += CHARACTER_COUNT;
                }
                goto here;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
            vmList = &menu->vm[MENU_VMS_CHARACTER_SELECT];
            for (i = 0; i < CHARACTER_COUNT; i++, vmList++)
            {
                if (i == menu->cursor)
                {
                    vmList->pendingInterrupt = 9;
                    vmList++;
                    vmList->pendingInterrupt = 9;
                }
                else
                {
                    vmList->pendingInterrupt = 12;
                    vmList++;
                    vmList->pendingInterrupt = 12;
                }
            }
        }
        if (WAS_PRESSED_REPEATING(TH_BUTTON_RIGHT))
        {
            menu->cursor--;
            if (menu->cursor < 0)
            {
                menu->cursor += CHARACTER_COUNT;
            }
            if (g_GameManager.difficulty == EXTRA && !g_GameManager.HasReachedMaxClears(menu->cursor, SHOT_TYPE_A) &&
                !g_GameManager.HasReachedMaxClears(menu->cursor, SHOT_TYPE_B))
            {
                menu->cursor++;
                if (menu->cursor >= CHARACTER_COUNT)
                {
                    menu->cursor -= CHARACTER_COUNT;
                }
            }
            else
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                vmList = &menu->vm[MENU_VMS_CHARACTER_SELECT];
                for (i = 0; i < CHARACTER_COUNT; i++, vmList++)
                {
                    if (i == menu->cursor)
                    {
                        vmList->pendingInterrupt = 10;
                        vmList++;
                        vmList->pendingInterrupt = 10;
                    }
                    else
                    {
                        vmList->pendingInterrupt = 11;
                        vmList++;
                        vmList->pendingInterrupt = 11;
                    }
                }
            }
        }
    here:
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            menu->gameState = STATE_DIFFICULTY_SELECT;
            menu->stateTimer = 0;
            if (g_GameManager.difficulty < EXTRA)
            {
                for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
                {
                    menu->vm[i].pendingInterrupt = 6;
                }
                menu->cursor = g_Supervisor.cfg.defaultDifficulty;
            }
            else
            {
                for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
                {
                    menu->vm[i].pendingInterrupt = 18;
                }
                menu->cursor = 0;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            break;
        }
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            menu->gameState = STATE_SHOT_SELECT;
            menu->stateTimer = 0;
            for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
            {
                menu->vm[i].pendingInterrupt = 13;
            }
            vmList = &menu->vm[g_GameManager.difficulty + MENU_VMS_DIFFICULTY_SELECT];
            vmList->pendingInterrupt = 0;
            vmList = &menu->vm[MENU_VMS_CHARACTER_SELECT];
            for (i = 0; i < CHARACTER_COUNT; i++, vmList += 2)
            {
                if (i != menu->cursor)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            vmList = &menu->vm[MENU_VMS_SHOTTYPE_SELECT];
            for (i = 0; i < SHOTTYPES_PER_CHARACTER; i++, vmList += 2)
            {
                if (i != menu->cursor)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            g_GameManager.character = menu->cursor;
            if (g_GameManager.difficulty < EXTRA)
            {
                menu->cursor = g_GameManager.shotType;
            }
            else
            {
                if (g_GameManager.HasReachedMaxClears(g_GameManager.character, g_GameManager.shotType))
                {
                    menu->cursor = g_GameManager.shotType;
                }
                else
                {
                    menu->cursor = 1 - g_GameManager.shotType;
                }
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
        }
        break;
    case STATE_SHOT_SELECT:
        MoveCursor(menu, SHOTTYPES_PER_CHARACTER);
        if (g_GameManager.difficulty == EXTRA &&
            !g_GameManager.HasReachedMaxClears(g_GameManager.character, menu->cursor))
        {
            menu->cursor = 1 - menu->cursor;
        }
        vmList = &menu->vm[MENU_VMS_SHOTTYPE_SELECT];
        for (i = 0; i < SHOTTYPES_PER_CHARACTER; i++, vmList += 2)
        {
            vmList[1].flags.colorOp = AnmColorOp_Add;
        }
        vmList = &menu->vm[MENU_VMS_SHOTTYPE_SELECT + g_GameManager.character * SHOTTYPES_PER_CHARACTER];
        for (i = 0; i < SHOTTYPES_PER_CHARACTER; i++, vmList++)
        {
            vmList->flags.colorOp = AnmColorOp_Add;
            vmList->flags.isVisible = true;
            if (i != menu->cursor)
            {
                if (!g_Supervisor.IsHardwareBlendingDisabled())
                {
                    vmList->color = 0xa0000000;
                }
                else
                {
                    vmList->color = 0xa0d0d0d0;
                }
                vmList->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            }
            else
            {
                if (!g_Supervisor.IsHardwareBlendingDisabled())
                {
                    vmList->color = 0xff202020;
                }
                else
                {
                    vmList->color = COLOR_WHITE;
                }
                vmList->posOffset = D3DXVECTOR3(-6.0f, -6.0f, 0.0f);
            }
        }
        if (menu->stateTimer < 30)
        {
            break;
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            menu->gameState = STATE_CHARACTER_SELECT;
            menu->stateTimer = 0;
            for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
            {
                menu->vm[i].pendingInterrupt = 7;
            }
            vmList = &menu->vm[MENU_VMS_SHOTTYPE_SELECT];
            for (i = 0; i < SHOTTYPES_PER_CHARACTER; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            vmList = &menu->vm[MENU_VMS_DIFFICULTY_SELECT + g_GameManager.difficulty];
            vmList->pendingInterrupt = 0;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            g_GameManager.shotType = menu->cursor;
            menu->cursor = g_GameManager.character;
            vmList = &menu->vm[MENU_VMS_CHARACTER_SELECT];
            for (i = 0; i < CHARACTER_COUNT; i++, vmList += 2)
            {
                if (i != menu->cursor)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            break;
        }
        else if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
#pragma var_order(refreshRate, local_48, local_4c)
        {
            float refreshRate, local_48;

            g_GameManager.shotType = menu->cursor;
            if (!g_GameManager.isInPracticeMode)
            {
                if (g_GameManager.difficulty < EXTRA)
                {
                    g_GameManager.currentStage = STAGE1;
                }
                else
                {
                    g_GameManager.currentStage = EXTRA_STAGE;
                }
            something:
                g_GameManager.livesRemaining = g_Supervisor.cfg.lifeCount;
                g_GameManager.bombsRemaining = g_Supervisor.cfg.bombCount;
                if (g_GameManager.difficulty == EXTRA || g_GameManager.isInPracticeMode)
                {
                    g_GameManager.livesRemaining = 2;
                    g_GameManager.bombsRemaining = 3;
                }
                g_Supervisor.curState = SUPERVISOR_STATE_GAMEMANAGER;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                g_GameManager.isInReplay = false;
                local_48 = 0.0f;
                if (menu->timeRelatedArrSize >= 2)
                {
                    for (i = 0; i < menu->timeRelatedArrSize; i++)
                    {
                        local_48 = local_48 + menu->timeRelatedArr[i];
                    }
                    local_48 = local_48 / i;
                }
                else
                {
                    local_48 = 60.0f;
                }

                refreshRate;
                if (local_48 >= 155.0f)
                    refreshRate = 60.0f / 160.0f;
                else if (local_48 >= 135.0f)
                    refreshRate = 60.0f / 150.0f;
                else if (local_48 >= 110.0f)
                    refreshRate = 60.0f / 120.0f;
                else if (local_48 >= 95.0f)
                    refreshRate = 60.0f / 100.0f;
                else if (local_48 >= 87.5f)
                    refreshRate = 60.0f / 90.0f;
                else if (local_48 >= 82.5f)
                    refreshRate = 60.0f / 85.0f;
                else if (local_48 >= 77.5f)
                    refreshRate = 60.0f / 80.0f;
                else if (local_48 >= 73.5f)
                    refreshRate = 60.0f / 75.0f;
                else if (local_48 >= 68.0f)
                    refreshRate = 60.0f / 70.0f;
                else
                    refreshRate = 1.0f;
                utils::DebugPrint("Reflesh Rate = %f\n", 60.0f / refreshRate);
                g_Supervisor.framerateMultiplier = refreshRate;
                g_Supervisor.StopAudio();
                return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
            }
            menu->gameState = STATE_PRACTICE_LVL_SELECT;
            menu->stateTimer = 0;
            for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
            {
                menu->vm[i].pendingInterrupt = 19;
            }
            vmList = &menu->vm[MENU_VMS_DIFFICULTY_SELECT + g_GameManager.difficulty];
            vmList->pendingInterrupt = 0;
            vmList = &menu->vm[MENU_VMS_CHARACTER_SELECT];
            for (i = 0; i < CHARACTER_COUNT; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            vmList = &menu->vm[MENU_VMS_SHOTTYPE_SELECT];
            for (i = 0; i < SHOTTYPES_PER_CHARACTER; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            menu->cursor = g_GameManager.menuCursorBackup;
            i32 local_4c = g_GameManager.clrd[g_GameManager.CharacterShotType()]
                                       .difficultyClearedWithoutRetries[g_GameManager.difficulty] > 6
                               ? 6
                               : g_GameManager.clrd[g_GameManager.CharacterShotType()]
                                     .difficultyClearedWithoutRetries[g_GameManager.difficulty];
            if (g_GameManager.difficulty == EASY && local_4c == 6)
            {
                local_4c = 5;
            }
            if (menu->cursor >= local_4c)
            {
                menu->cursor = 0;
            }
        }
        break;
    case STATE_PRACTICE_LVL_SELECT: {
        u32 chosenStage = g_GameManager.clrd[g_GameManager.CharacterShotType()]
                                      .difficultyClearedWithoutRetries[g_GameManager.difficulty] > 6
                              ? 6
                              : g_GameManager.clrd[g_GameManager.CharacterShotType()]
                                    .difficultyClearedWithoutRetries[g_GameManager.difficulty];
        if (g_GameManager.difficulty == EASY && chosenStage == 6)
        {
            chosenStage = 5;
        }
        MoveCursor(menu, chosenStage);
        if (menu->stateTimer < 30)
        {
            break;
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            menu->gameState = STATE_SHOT_SELECT;
            menu->stateTimer = 0;
            for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
            {
                menu->vm[i].pendingInterrupt = 13;
            }
            vmList = &menu->vm[MENU_VMS_DIFFICULTY_SELECT + g_GameManager.difficulty];
            vmList->pendingInterrupt = 0;
            vmList = &menu->vm[MENU_VMS_CHARACTER_SELECT];
            for (i = 0; i < CHARACTER_COUNT; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            vmList = &menu->vm[MENU_VMS_SHOTTYPE_SELECT];
            for (i = 0; i < SHOTTYPES_PER_CHARACTER; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            menu->cursor = g_GameManager.shotType;
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
            break;
        }
        else if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            g_GameManager.currentStage = menu->cursor;
            g_GameManager.menuCursorBackup = menu->cursor;
            goto something;
        }
        break;
    }
    case STATE_QUIT:
        if (menu->stateTimer >= 60)
        {
            g_Supervisor.curState = SUPERVISOR_STATE_EXITSUCCESS;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    case STATE_SCORE:
        if (menu->stateTimer >= 60)
        {
            g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    case STATE_MUSIC_ROOM:
        if (menu->stateTimer >= 60)
        {
            g_Supervisor.curState = SUPERVISOR_STATE_MUSICROOM;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    }
    menu->stateTimer++;
    for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
    {
        if (g_AnmManager->ShouldDraw(&menu->vm[i]))
        {
            g_AnmManager->ExecuteScript(&menu->vm[i]);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static CursorMovement MoveCursor(MainMenu *menu, i32 menuLength)
{
    if (WAS_PRESSED_REPEATING(TH_BUTTON_UP))
    {
        menu->cursor--;
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
        if (menu->cursor < 0)
        {
            menu->cursor = menuLength - 1;
        }
        if (menu->cursor >= menuLength)
        {
            menu->cursor = 0;
        }
        return CURSOR_MOVE_UP;
    }

    if (WAS_PRESSED_REPEATING(TH_BUTTON_DOWN))
    {
        menu->cursor++;
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
        if (menu->cursor < 0)
        {
            menu->cursor = menuLength - 1;
        }
        if (menu->cursor >= menuLength)
        {
            menu->cursor = 0;
        }
        return CURSOR_MOVE_DOWN;
    }

    return CURSOR_DONT_MOVE;
}

static void SwapMapping(MainMenu *menu, i16 btnPressed, i16 oldMapping, ZunBool unk)
{
    if (!unk && menu->controlMapping.shootButton == btnPressed)
    {
        menu->controlMapping.shootButton = oldMapping;
    }
    if (menu->controlMapping.bombButton == btnPressed)
    {
        menu->controlMapping.bombButton = oldMapping;
    }
    if (!unk && menu->controlMapping.focusButton == btnPressed)
    {
        menu->controlMapping.focusButton = oldMapping;
    }
    if (menu->controlMapping.upButton == btnPressed)
    {
        menu->controlMapping.upButton = oldMapping;
    }
    if (menu->controlMapping.downButton == btnPressed)
    {
        menu->controlMapping.downButton = oldMapping;
    }
    if (menu->controlMapping.leftButton == btnPressed)
    {
        menu->controlMapping.leftButton = oldMapping;
    }
    if (menu->controlMapping.rightButton == btnPressed)
    {
        menu->controlMapping.rightButton = oldMapping;
    }
    if (menu->controlMapping.menuButton == btnPressed)
    {
        menu->controlMapping.menuButton = oldMapping;
    }
    if (menu->controlMapping.skipButton == btnPressed)
    {
        menu->controlMapping.skipButton = oldMapping;
    }
}

static void DrawMenuItem(AnmVm *vm, int itemNumber, int cursor, D3DCOLOR currentItemColor, D3DCOLOR otherItemColor,
                         int vm_amount)
{
    if (itemNumber == cursor)
    {
        if (!g_Supervisor.IsSoftwareTexturing())
        {
            vm->color = currentItemColor;
        }
        else
        {
            g_AnmManager->SetActiveSprite(vm, vm->baseSpriteIndex + vm_amount);
            vm->color = currentItemColor & D3DCOLOR_RGBA(0x00, 0x00, 0x00, 0xff) |
                        D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0x00); // just... why?
        }
        vm->posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
    }
    else
    {
        if (!g_Supervisor.IsSoftwareTexturing())
        {
            vm->color = otherItemColor;
        }
        else
        {
            g_AnmManager->SetActiveSprite(vm, vm->baseSpriteIndex);
            vm->color = otherItemColor & D3DCOLOR_RGBA(0x00, 0x00, 0x00, 0xff) |
                        D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0x00); // again, why?
        }
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    }
}

#pragma var_order(time, i)
ZunResult MainMenu::BeginStartup()
{
    DWORD time;
    i32 i;

    if (LoadTitleAnm(this) != ZUN_SUCCESS)
    {
        g_Supervisor.curState = SUPERVISOR_STATE_EXITSUCCESS;
        return ZUN_ERROR;
    }
    if (g_Supervisor.startupTimeBeforeMenuMusic > 0)
    {
        time = timeGetTime();
        while (time - g_Supervisor.startupTimeBeforeMenuMusic >= 0 &&
               (3000 > time - g_Supervisor.startupTimeBeforeMenuMusic))
        {
            time = timeGetTime();
        }
        g_Supervisor.startupTimeBeforeMenuMusic = 0;
        g_Supervisor.PlayAudio("bgm/th06_01.mid");
    }
    for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
    {
        this->vm[i].pendingInterrupt = 1;
        this->vm[i].flags.colorOp = AnmColorOp_Add;
        if (!g_Supervisor.IsHardwareBlendingDisabled())
        {
            this->vm[i].color = COLOR_BLACK;
        }
        else
        {
            this->vm[i].color = COLOR_WHITE;
        }
        this->vm[i].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    }
    this->gameState = STATE_PRE_INPUT;
    return ZUN_SUCCESS;
}

ZunBool MainMenu::WeirdSecondInputCheck()
{
    i32 vm;

    if (this->stateTimer < 30)
    {
        return true;
    }

    if (!WAS_PRESSED_REPEATING(TH_BUTTON_SELECTMENU | TH_BUTTON_BOMB | TH_BUTTON_MENU | TH_BUTTON_Q | TH_BUTTON_S))
    {
        return true;
    }

    this->stateTimer = 0;
    this->gameState = STATE_MAIN_MENU;
    for (vm = 0; vm < ARRAY_SIZE_SIGNED(this->vm); vm++)
    {
        this->vm[vm].pendingInterrupt = 2;
    }
    if (!g_Supervisor.IsHardwareBlendingDisabled())
    {
        this->vm[this->cursor].color = COLOR_RED;
    }
    else
    {
        this->vm[this->cursor].color = COLOR_PINK;
    }
    this->vm[this->cursor].posOffset = D3DXVECTOR3(-6.0f, -6.0f, 0.0f);

    this->minimumOpacity = 0;
    this->menuTextColor = COLOR_MENU_ACTIVE_BACKGROUND;
    this->numFramesSinceActive = 0;
    this->framesActive = 60;
    return false;
}

#pragma var_order(i, drawVm)
ZunResult MainMenu::DrawStartMenu(void)
{
    i32 i;
    i = MoveCursor(this, 8);
    if (this->cursor == 1 && !g_GameManager.HasReachedMaxClears(CHARA_REIMU, SHOT_TYPE_A) &&
        !g_GameManager.HasReachedMaxClears(CHARA_REIMU, SHOT_TYPE_B) &&
        !g_GameManager.HasReachedMaxClears(CHARA_MARISA, SHOT_TYPE_A) &&
        !g_GameManager.HasReachedMaxClears(CHARA_MARISA, SHOT_TYPE_B))
    {
        this->cursor += i;
    }
    AnmVm *drawVm = this->vm;
    for (i = 0; i < 8; i++, drawVm++)
    {
        DrawMenuItem(drawVm, i, this->cursor, COLOR_RED, COLOR_START_MENU_ITEM_INACTIVE, ARRAY_SIZE_SIGNED(this->vm));
    }
    if (this->stateTimer >= 20)
    {
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            switch (this->cursor)
            {
            case 0:
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                this->gameState = STATE_DIFFICULTY_LOAD;
                g_GameManager.isInPracticeMode = false;
                if (g_GameManager.difficulty >= EXTRA)
                {
                    g_GameManager.difficulty = NORMAL;
                }
                if (g_Supervisor.cfg.defaultDifficulty >= EXTRA)
                {
                    g_Supervisor.cfg.defaultDifficulty = NORMAL;
                }
                this->stateTimer = 0;
                this->minimumOpacity = 0x40000000;
                this->menuTextColor = COLOR_BLACK;
                this->numFramesSinceActive = 0;
                this->framesActive = 60;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                break;
#if !TRIALBUILD
            case 1:
                if (!(!g_GameManager.HasReachedMaxClears(CHARA_REIMU, SHOT_TYPE_A) &&
                      !g_GameManager.HasReachedMaxClears(CHARA_REIMU, SHOT_TYPE_B) &&
                      !g_GameManager.HasReachedMaxClears(CHARA_MARISA, SHOT_TYPE_A) &&
                      !g_GameManager.HasReachedMaxClears(CHARA_MARISA, SHOT_TYPE_B)))
                {
                    for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                    {
                        this->vm[i].pendingInterrupt = 4;
                    }
                    this->gameState = STATE_DIFFICULTY_LOAD;
                    g_GameManager.isInPracticeMode = false;
                    g_GameManager.difficulty = EXTRA;
                    this->stateTimer = 0;
                    this->minimumOpacity = 0x40000000;
                    this->menuTextColor = COLOR_BLACK;
                    this->numFramesSinceActive = 0;
                    this->framesActive = 60;
                    g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                }
                else
                {
                    g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
                }
                break;
#endif
            case 2:
                g_GameManager.isInPracticeMode = true;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                this->gameState = STATE_DIFFICULTY_LOAD;
                if (g_GameManager.difficulty >= EXTRA)
                {
                    g_GameManager.difficulty = NORMAL;
                }
                if (g_Supervisor.cfg.defaultDifficulty >= EXTRA)
                {
                    g_Supervisor.cfg.defaultDifficulty = NORMAL;
                }
                this->stateTimer = 0;
                this->minimumOpacity = 0x40000000;
                this->menuTextColor = COLOR_BLACK;
                this->numFramesSinceActive = 0;
                this->framesActive = 60;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                break;
            case 3:
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                this->gameState = STATE_REPLAY_LOAD;
                g_GameManager.isInPracticeMode = false;
                this->stateTimer = 0;
                this->minimumOpacity = 0x40000000;
                this->menuTextColor = COLOR_BLACK;
                this->numFramesSinceActive = 0;
                this->framesActive = 60;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                break;
            case 4:
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                this->gameState = STATE_SCORE;
                this->stateTimer = 0;
                this->minimumOpacity = 0x40000000;
                this->menuTextColor = COLOR_BLACK;
                this->numFramesSinceActive = 0;
                this->framesActive = 60;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                break;
            case 5:
                this->gameState = STATE_MUSIC_ROOM;
                this->stateTimer = 0;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                break;
            case 6:
                this->gameState = STATE_OPTIONS;
                this->stateTimer = 0;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 3;
                }
                this->cursor = 0;
                this->colorMode16bit = g_Supervisor.cfg.colorMode16bit;
                this->windowed = g_Supervisor.cfg.windowed;
                this->frameskipConfig = g_Supervisor.cfg.frameskipConfig;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                break;
            case 7:
                this->gameState = STATE_QUIT;
                this->stateTimer = 0;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
                break;
            }
        }
        if (WAS_PRESSED(TH_BUTTON_Q))
        {
            this->gameState = STATE_QUIT;
            this->stateTimer = 0;
            for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
            {
                this->vm[i].pendingInterrupt = 4;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            this->cursor = 7;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
        }
    }
    return ZUN_SUCCESS;
}

#pragma var_order(anmVm, cur, replayFileHandle, replayFileIdx, replayData, replayFilePath, replayFileInfo)
i32 MainMenu::ReplayHandling()
{
    AnmVm *anmVm;
    i32 cur;
    HANDLE replayFileHandle;
    u32 replayFileIdx;
    ReplayData *replayData;
    char replayFilePath[64];
    WIN32_FIND_DATA replayFileInfo;

    switch (this->gameState)
    {
    case STATE_REPLAY_LOAD:
        if (this->stateTimer == 60)
        {
            if (LoadReplayMenu(this))
            {
                g_GameErrorContext.Log(TH_ERR_MAINMENU_LOAD_SELECT_SCREEN_FAILED);
                g_Supervisor.curState = SUPERVISOR_STATE_EXITSUCCESS;
                return ZUN_SUCCESS;
            }
            else
            {
                replayFileIdx = 0;
                for (cur = 0; cur < REPLAYS_PER_PAGE; cur++)
                {
                    sprintf(replayFilePath, "./replay/th6_%.2d.rpy", cur + 1);
                    replayData = (ReplayData *)FileSystem::OpenPath(replayFilePath, EXTERNAL_FILE);
                    if (replayData == NULL)
                    {
                        continue;
                    }
                    if (!ReplayManager::ValidateReplayData(replayData, g_LastFileSize))
                    {
                        this->replayFileData[replayFileIdx] = *replayData;
                        strcpy(this->replayFilePaths[replayFileIdx], replayFilePath);
                        sprintf(this->replayFileName[replayFileIdx], "No.%.2d", cur + 1);
                        replayFileIdx++;
                    }
                    ZUN_FREE(replayData);
                }
                _mkdir("./replay");
                _chdir("./replay");
                replayFileHandle = FindFirstFile("th6_ud????.rpy", &replayFileInfo);
                if (replayFileHandle != INVALID_HANDLE_VALUE)
                {
                    for (cur = 0; cur < USER_REPLAY_COUNT; cur++)
                    {
                        replayData = (ReplayData *)FileSystem::OpenPath(replayFileInfo.cFileName, EXTERNAL_FILE);
                        if (replayData == NULL)
                        {
                            continue;
                        }
                        if (!ReplayManager::ValidateReplayData(replayData, g_LastFileSize))
                        {
                            this->replayFileData[replayFileIdx] = *replayData;
                            sprintf(this->replayFilePaths[replayFileIdx], "./replay/%s", replayFileInfo.cFileName);
                            sprintf(this->replayFileName[replayFileIdx], "User ");
                            replayFileIdx++;
                        }
                        ZUN_FREE(replayData);
                        if (!FindNextFile(replayFileHandle, &replayFileInfo))
                            break;
                    }
                }
                FindClose(replayFileHandle);
                _chdir("../");
                this->replayFilesNum = replayFileIdx;
                this->minimumOpacity = 0;
                this->framesInactive = this->framesActive;
                this->framesActive = 0;
                this->gameState = STATE_REPLAY_ANIM;
                anmVm = this->vm;
                for (cur = 0; cur < ARRAY_SIZE_SIGNED(this->vm); cur++, anmVm++)
                {
                    anmVm->pendingInterrupt = 15;
                }
                this->cursor = 0;
            }
            break;
        }
        break;
    case STATE_REPLAY_UNLOAD:
        if (this->stateTimer == 36)
        {
            this->gameState = STATE_STARTUP;
            this->stateTimer = 0;
        }
        break;
    case STATE_REPLAY_ANIM:
        if (this->stateTimer < 40)
        {
            break;
        }
        if (this->replayFilesNum != NULL)
        {
            MoveCursor(this, this->replayFilesNum);
            this->chosenReplay = this->cursor;
            if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
            {
                this->gameState = STATE_REPLAY_SELECT;
                anmVm = &(this->vm[97]);
                for (cur = 0; cur < 25; cur++, anmVm++)
                {
                    anmVm->pendingInterrupt = 17;
                }
                anmVm = &this->vm[99 + this->chosenReplay];
                anmVm->pendingInterrupt = 16;
                this->stateTimer = 0;
                this->cursor = 0;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                this->currentReplay =
                    (ReplayData *)FileSystem::OpenPath(this->replayFilePaths[this->chosenReplay], EXTERNAL_FILE);
                ReplayManager::ValidateReplayData(this->currentReplay, g_LastFileSize);
                for (cur = 0; cur < ARRAY_SIZE_SIGNED(this->currentReplay->stageReplayData); cur++)
                {
                    if (this->currentReplay->stageReplayData[cur] != NULL)
                    {
                        this->currentReplay->stageReplayData[cur] =
                            (StageReplayData *)((u32)this->currentReplay +
                                                (u32)this->currentReplay->stageReplayData[cur]);
                    }
                }

                while (this->replayFileData[this->chosenReplay].stageReplayData[this->cursor] == NULL)
                {
                    this->cursor++;

                    if (this->cursor >= ARRAY_SIZE_SIGNED(this->currentReplay->stageReplayData))
                    {
                        return ZUN_SUCCESS;
                    }
                }
            }
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            this->gameState = STATE_REPLAY_UNLOAD;
            this->stateTimer = 0;
            for (cur = 0; cur < ARRAY_SIZE_SIGNED(this->vm); cur++)
            {
                this->vm[cur].pendingInterrupt = 4;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            this->cursor = 0;
            break;
        }
        break;
    case STATE_REPLAY_SELECT:
        if (this->stateTimer < 40)
        {
            break;
        }
        cur = MoveCursor(this, 7);
        if (cur < 0)
        {
            while (this->replayFileData[this->chosenReplay].stageReplayData[this->cursor] == NULL)
            {
                this->cursor--;
                if (this->cursor < 0)
                {
                    this->cursor = 6;
                }
            }
        }
        else if (cur > 0)
        {
            while (this->replayFileData[this->chosenReplay].stageReplayData[this->cursor] == NULL)
            {
                this->cursor++;
                if (this->cursor >= 7)
                {
                    this->cursor = 0;
                }
            }
        }
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU) && this->currentReplay[this->cursor].stageReplayData)
        {
            g_GameManager.isInReplay = true;
            g_Supervisor.framerateMultiplier = 1.0f;
            strcpy(g_GameManager.replayFile, this->replayFilePaths[this->chosenReplay]);
            g_GameManager.difficulty = (Difficulty)this->currentReplay->difficulty;
            g_GameManager.character = this->currentReplay->shottypeChara / 2;
            g_GameManager.shotType = this->currentReplay->shottypeChara % 2;
            cur = 0;
            while (this->currentReplay->stageReplayData[cur] == NULL)
            {
                cur++;
            }
            g_GameManager.livesRemaining = this->currentReplay->stageReplayData[cur]->livesRemaining;
            g_GameManager.bombsRemaining = this->currentReplay->stageReplayData[cur]->bombsRemaining;
            ZUN_FREE(this->currentReplay);
            this->currentReplay = NULL;
            g_GameManager.currentStage = this->cursor;
            g_Supervisor.curState = SUPERVISOR_STATE_GAMEMANAGER;
            return 1;
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            ZUN_FREE(this->currentReplay);
            this->currentReplay = NULL;
            this->gameState = STATE_REPLAY_ANIM;
            this->stateTimer = 0;
            for (cur = 0; cur < ARRAY_SIZE_SIGNED(this->vm); cur++)
            {
                this->vm[cur].pendingInterrupt = 4;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            this->gameState = STATE_REPLAY_ANIM;
            anmVm = this->vm;
            for (cur = 0; cur < ARRAY_SIZE_SIGNED(this->vm); cur++, anmVm++)
            {
                anmVm->pendingInterrupt = 15;
            }
            this->cursor = this->chosenReplay;
        }
    }
    return 0;
}

#pragma var_order(vmRef, i, replayAmount)
ZunResult MainMenu::DrawReplayMenu()
{
    static const char *g_StageList[] = {"Stage1", "Stage2", "Stage3", "Stage4", "Stage5", "Stage6", "Extra "};
    static const char *g_ShortCharacterList[] = {"ReimuA ", "ReimuB ", "MarisaA", "MarisaB"};
    static const char *g_DifficultyList[] = {"Easy   ", "Normal ", "Hard   ", "Lunatic", "Extra  "};

    i32 replayAmount;
    i32 i;
    AnmVm *vmRef;

    vmRef = &this->vm[98];
    g_AsciiManager.AddFormatText(&vmRef->pos, "No.   Name      Date     Player   Rank");

    for (i = this->chosenReplay - this->chosenReplay % REPLAYS_PER_PAGE, replayAmount = i;
         i < replayAmount + REPLAYS_PER_PAGE; i++)
    {
        if (i >= this->replayFilesNum)
        {
            break;
        }
        vmRef++;
        if (!g_Supervisor.IsSoftwareTexturing())
        {
            if (i == this->chosenReplay)
            {
                g_AsciiManager.color = COLOR_LIGHT_RED;
            }
            else
            {
                g_AsciiManager.color = COLOR_GREY;
            }
        }
        else
        {
            ZunBool isSelected = (i == this->chosenReplay);
            g_AsciiManager.isSelected = isSelected;

            if (i == this->chosenReplay)
            {
                g_AsciiManager.color = COLOR_WHITE;
            }
            else
            {
                g_AsciiManager.color = COLOR_GREY;
            }
        }

        g_AsciiManager.AddFormatText(&vmRef->pos, "%s %8s  %8s %7s  %7s", this->replayFileName[i],
                                     this->replayFileData[i].name, this->replayFileData[i].date,
                                     g_ShortCharacterList[this->replayFileData[i].shottypeChara],
                                     g_DifficultyList[this->replayFileData[i].difficulty]);
    }
    if (this->gameState == STATE_REPLAY_SELECT && this->currentReplay)
    {
        g_AsciiManager.color = COLOR_WHITE;
        g_AsciiManager.isSelected = false;

        vmRef = &this->vm[97];
        g_AsciiManager.AddFormatText(&vmRef->pos, "       %2.3f%%", this->currentReplay->slowdownRate);

        vmRef = &this->vm[114];
        g_AsciiManager.AddFormatText(&vmRef->pos, "Stage  LastScore");

        for (i = 0; i < 7; i++)
        {
            vmRef++;
            if (!g_Supervisor.IsSoftwareTexturing())
            {
                if (i == this->cursor)
                {
                    g_AsciiManager.color = COLOR_LIGHT_RED;
                }
                else
                {
                    g_AsciiManager.color = COLOR_GREY;
                }
            }
            else
            {
                ZunBool isSelected = (i == this->cursor);
                g_AsciiManager.isSelected = isSelected;
                if (i == this->cursor)
                {
                    g_AsciiManager.color = COLOR_WHITE;
                }
                else
                {
                    g_AsciiManager.color = COLOR_GREY;
                }
            }
            if (this->currentReplay->stageReplayData[i])
            {
                g_AsciiManager.AddFormatText(&vmRef->pos, "%s %9d", g_StageList[i],
                                             this->currentReplay->stageReplayData[i]->score);
            }
            else
            {
                g_AsciiManager.AddFormatText(&vmRef->pos, "%s ---------", g_StageList[i]);
            }
        }
    }
    g_AsciiManager.color = COLOR_WHITE;
    g_AsciiManager.isSelected = false;
    return ZUN_SUCCESS;
}

void MainMenu::ColorMenuItem(AnmVm *vm, i32 item, i32 subItem, i32 subItemSelected)
{
    if (subItem != subItemSelected)
    {
        if (!g_Supervisor.IsSoftwareTexturing())
        {
            vm->color = COLOR_MENU_ITEM_DEFAULT;
        }
        else
        {
            g_AnmManager->SetActiveSprite(vm, vm->baseSpriteIndex);
        }
        vm->scaleX = 1.0f;
        vm->scaleY = 1.0f;
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        if (!g_Supervisor.IsSoftwareTexturing())
        {
            vm->color = COLOR_MENU_ITEM_HIGHLIGHT;
        }
        else if (vm->baseSpriteIndex < ANM_OFFSET_TITLE04)
        {
            g_AnmManager->SetActiveSprite(vm, vm->baseSpriteIndex + (ANM_OFFSET_TITLE01S - ANM_OFFSET_TITLE01));
        }
        else
        {
            g_AnmManager->SetActiveSprite(vm, vm->baseSpriteIndex + (ANM_OFFSET_TITLE04S - ANM_OFFSET_TITLE04));
        }
        vm->posOffset = D3DXVECTOR3(-2.0f, -2.0f, 0.0f);
    }

    if (item != this->cursor)
    {
        if (!g_Supervisor.IsHardwareBlendingDisabled())
        {
            vm->color = COLOR_SET_ALPHA2(vm->color, 0x80);
        }
        else
        {
            vm->color = COLOR_SET_ALPHA2(vm->color, 0x80);
        }

        vm->posOffset += D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        if (!g_Supervisor.IsHardwareBlendingDisabled())
        {
            vm->color = COLOR_SET_ALPHA2(vm->color, 0xff);
        }
        else
        {
            vm->color = COLOR_SET_ALPHA2(vm->color, 0xff);
        }

        vm->posOffset += D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
    }
}

#pragma var_order(i, optionsVm)
u32 MainMenu::OnUpdateOptionsMenu()
{
    AnmVm *optionsVm;
    i32 i;

    MoveCursor(this, 9);
    optionsVm = &this->vm[8];
    for (i = 0; i < 9; i++)
    {
        if (i >= 5 && i <= 7)
        {
            this->ColorMenuItem(&this->vm[i + 67], i, i, this->cursor);
        }
        else
        {
            this->ColorMenuItem(optionsVm, i, i, this->cursor);
            optionsVm++;
        }
    }

    for (i = 0; i < 5; i++, optionsVm++)
    {
        this->ColorMenuItem(optionsVm, CURSOR_OPTIONS_POS_LIFECOUNT, i, g_Supervisor.cfg.lifeCount);
    }

    for (i = 0; i < 4; i++, optionsVm++)
    {
        this->ColorMenuItem(optionsVm, CURSOR_OPTIONS_POS_BOMBCOUNT, i, g_Supervisor.cfg.bombCount);
    }
    for (i = 0; i < 2; i++, optionsVm++)
    {
        this->ColorMenuItem(optionsVm, CURSOR_OPTIONS_POS_COLORMODE, i, g_Supervisor.cfg.colorMode16bit);
    }
    for (i = 0; i < 2; i++, optionsVm++)
    {
        this->ColorMenuItem(optionsVm, CURSOR_OPTIONS_POS_PLAYSOUNDS, i, g_Supervisor.cfg.playSounds);
    }
    optionsVm = &this->vm[77];

    for (i = 0; i < 3; i++, optionsVm++)
    {
        this->ColorMenuItem(optionsVm, CURSOR_OPTIONS_POS_MUSICMODE, i, g_Supervisor.cfg.musicMode);
    }
    optionsVm = &this->vm[75];
    for (i = 0; i < 2; i++, optionsVm++)
    {
        this->ColorMenuItem(optionsVm, CURSOR_OPTIONS_POS_SCREENMODE, i, this->windowed);
    }
    if (this->stateTimer >= 32)
    {
        if (WAS_PRESSED_REPEATING(TH_BUTTON_LEFT))
        {
            switch (this->cursor)
            {
            case CURSOR_OPTIONS_POS_LIFECOUNT:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                if (g_Supervisor.cfg.lifeCount <= 0)
                {
                    g_Supervisor.cfg.lifeCount = 5;
                }
                g_Supervisor.cfg.lifeCount -= 1;
                break;

            case CURSOR_OPTIONS_POS_BOMBCOUNT:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                if (g_Supervisor.cfg.bombCount <= 0)
                {
                    g_Supervisor.cfg.bombCount = 4;
                }
                g_Supervisor.cfg.bombCount -= 1;
                break;

            case CURSOR_OPTIONS_POS_COLORMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                if (g_Supervisor.cfg.colorMode16bit <= 0)
                {
                    g_Supervisor.cfg.colorMode16bit = 2;
                }
                g_Supervisor.cfg.colorMode16bit -= 1;
                break;

            case CURSOR_OPTIONS_POS_MUSICMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                g_Supervisor.StopAudio();
                if (g_Supervisor.cfg.musicMode <= OFF)
                {
                    g_Supervisor.cfg.musicMode = MIDI + 1;
                }
                g_Supervisor.cfg.musicMode -= 1;
                g_Supervisor.SetupMidiPlayback("bgm/th06_01.mid");
                g_Supervisor.PlayAudio("bgm/th06_01.mid");
                break;

            case CURSOR_OPTIONS_POS_PLAYSOUNDS:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                if (g_Supervisor.cfg.playSounds <= 0)
                {
                    g_Supervisor.cfg.playSounds = 2;
                }
                g_Supervisor.cfg.playSounds -= 1;
                break;

            case CURSOR_OPTIONS_POS_SCREENMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                if (this->windowed <= 0)
                {
                    this->windowed = 2;
                }
                this->windowed -= 1;
                break;
            }
        }
        if (WAS_PRESSED(TH_BUTTON_MENU | TH_BUTTON_BOMB))
        {
            this->cursor = CURSOR_OPTIONS_POS_EXIT;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
        }
        if (WAS_PRESSED_REPEATING(TH_BUTTON_RIGHT))
        {
            switch (this->cursor)
            {
            case CURSOR_OPTIONS_POS_LIFECOUNT:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                g_Supervisor.cfg.lifeCount += 1;
                if (g_Supervisor.cfg.lifeCount >= 5)
                {
                    g_Supervisor.cfg.lifeCount = 0;
                }
                break;
            case CURSOR_OPTIONS_POS_BOMBCOUNT:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                g_Supervisor.cfg.bombCount += 1;
                if (g_Supervisor.cfg.bombCount >= 4)
                {
                    g_Supervisor.cfg.bombCount = 0;
                }
                break;
            case CURSOR_OPTIONS_POS_COLORMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                g_Supervisor.cfg.colorMode16bit += 1;
                if (g_Supervisor.cfg.colorMode16bit >= 2)
                {
                    g_Supervisor.cfg.colorMode16bit = 0;
                }
                break;
            case CURSOR_OPTIONS_POS_MUSICMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                g_Supervisor.StopAudio();
                g_Supervisor.cfg.musicMode += 1;
                if (g_Supervisor.cfg.musicMode >= MIDI + 1)
                {
                    g_Supervisor.cfg.musicMode = OFF;
                }
                g_Supervisor.SetupMidiPlayback("bgm/th06_01.mid");
                g_Supervisor.PlayAudio("bgm/th06_01.mid");
                break;
            case CURSOR_OPTIONS_POS_PLAYSOUNDS:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                g_Supervisor.cfg.playSounds += 1;
                if (g_Supervisor.cfg.playSounds >= 2)
                {
                    g_Supervisor.cfg.playSounds = 0;
                }
                break;
            case CURSOR_OPTIONS_POS_SCREENMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
                this->windowed += 1;
                if (this->windowed >= 2)
                {
                    this->windowed = 0;
                }
                break;
            }
        }
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            switch (this->cursor)
            {
            case CURSOR_OPTIONS_POS_KEYCONFIG:

                this->gameState = STATE_KEYCONFIG;
                this->stateTimer = 0;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 5;
                }
                this->cursor = 0;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);

                this->controlMapping = g_ControllerMapping;

                g_ControllerMapping.upButton = -1;
                g_ControllerMapping.downButton = -1;
                break;

            case CURSOR_OPTIONS_POS_SETDEFAULT:

                g_Supervisor.StopAudio();
                g_Supervisor.cfg.lifeCount = 2;
                g_Supervisor.cfg.bombCount = 3;
                g_Supervisor.cfg.musicMode = WAV;
                g_Supervisor.cfg.playSounds = true;
                g_Supervisor.cfg.defaultDifficulty = NORMAL;
                g_Supervisor.cfg.windowed = false;
                g_Supervisor.cfg.frameskipConfig = 0;
                g_Supervisor.SetupMidiPlayback("bgm/th06_01.mid");
                g_Supervisor.PlayAudio("bgm/th06_01.mid");
                break;

            case CURSOR_OPTIONS_POS_EXIT:

                this->gameState = STATE_MAIN_MENU;
                this->stateTimer = 0;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 2;
                }
                // TODO: Cursor enum for the main menu
                this->cursor = 6;
                g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
                if (this->colorMode16bit != g_Supervisor.cfg.colorMode16bit ||
                    this->windowed != g_Supervisor.cfg.windowed ||
                    this->frameskipConfig != g_Supervisor.cfg.frameskipConfig)
                {
                    g_Supervisor.cfg.frameskipConfig = this->frameskipConfig;
                    g_Supervisor.cfg.windowed = this->windowed;
                    g_Supervisor.curState = SUPERVISOR_STATE_EXITERROR;
                    return 1;
                }
                break;
            }
        }
    }
    return 0;
}

#pragma var_order(stageNum, color, charShotType, selectedStage, textPos)
ZunResult MainMenu::ChoosePracticeLevel()
{
    if (this->gameState == STATE_PRACTICE_LVL_SELECT)
    {
        D3DXVECTOR3 textPos(320.0f, 200.0f, 0.0f);
        u32 color = (this->stateTimer < 30) ? this->stateTimer * 255 / 30 : 255;
        i32 charShotType = g_GameManager.character * SHOTTYPES_PER_CHARACTER + g_GameManager.shotType;
        i32 selectedStage =
            (g_GameManager.clrd[charShotType].difficultyClearedWithoutRetries[g_GameManager.difficulty] > 6)
                ? 6
                : g_GameManager.clrd[charShotType].difficultyClearedWithoutRetries[g_GameManager.difficulty];

        if (g_GameManager.difficulty == EASY && selectedStage == 6)
        {
            selectedStage = 5;
        }

        i32 stageNum;
        for (stageNum = 0; stageNum < selectedStage; stageNum++)
        {
            if (stageNum == this->cursor)
            {
                g_AsciiManager.color = color << 24 | 0x00C0F0F0;
            }
            else
            {
                g_AsciiManager.color = (color >> 1) << 24 | 0x0080C0C0;
            }
            g_AsciiManager.AddFormatText(&textPos, "STAGE %d  %.9d", stageNum + 1,
                                         g_GameManager.pscr[charShotType][stageNum][g_GameManager.difficulty].score);
            textPos.y += 24.0f;
        }
        g_AsciiManager.color = COLOR_WHITE;
    }
    return ZUN_SUCCESS;
}

#pragma var_order(targetOpacity, window, vmIdx, curVm)
ChainCallbackResult MainMenu::OnDraw(MainMenu *menu)
{
    AnmVm *curVm;
    i32 vmIdx;
    ZunRect window;
    i32 targetOpacity;

    curVm = menu->vm;
    window.left = 0.0;
    window.top = 0.0;
    window.right = GAME_WINDOW_WIDTH;
    window.bottom = GAME_WINDOW_HEIGHT;
    if (menu->gameState == STATE_STARTUP)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    g_AnmManager->SetCurrentTexture(NULL);
    g_AnmManager->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    if (menu->framesActive != 0)
    {
        // This is confusing. framesActive/framesInactive appear to be unsigned,
        // due to how they get loaded. But this comparison is signed somehow.
        // Why?
        if (menu->numFramesSinceActive < (i32)menu->framesActive)
        {
            menu->numFramesSinceActive++;
        }
        targetOpacity = COLOR_ALPHA(menu->menuTextColor) - COLOR_ALPHA(menu->minimumOpacity);
        ScreenEffect::DrawSquare(
            &window,
            COLOR_SET_ALPHA(menu->menuTextColor, targetOpacity * menu->numFramesSinceActive / menu->framesActive +
                                                     COLOR_ALPHA(menu->minimumOpacity)));
    }
    else if (menu->numFramesSinceActive != 0)
    {
        menu->numFramesSinceActive--;
        targetOpacity = COLOR_ALPHA(menu->menuTextColor) - COLOR_ALPHA(menu->minimumOpacity);
        ScreenEffect::DrawSquare(
            &window,
            COLOR_SET_ALPHA(menu->menuTextColor, targetOpacity * menu->numFramesSinceActive / menu->framesInactive +
                                                     COLOR_ALPHA(menu->minimumOpacity)));
    }
    for (vmIdx = 0; vmIdx < 98; vmIdx++, curVm++)
    {
        if (g_AnmManager->ShouldDraw(curVm))
        {
            D3DXVECTOR3 posBackup = curVm->pos;
            curVm->pos += curVm->posOffset;
            g_AnmManager->Draw(curVm);
            curVm->pos = posBackup;
        }
    }
    switch (menu->gameState)
    {
    case STATE_REPLAY_ANIM:
    case STATE_REPLAY_UNLOAD:
    case STATE_REPLAY_SELECT:
        menu->DrawReplayMenu();
    default:
        menu->ChoosePracticeLevel();
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ZunResult MainMenu::LoadTitleAnm(MainMenu *menu)
{
    i32 i;

    g_Supervisor.LoadPbg3(TL_PBG3_INDEX, TH_TL_DAT_FILE);
    for (i = ANM_FILE_SELECT01; i <= ANM_FILE_REPLAY; i++)
    {
        g_AnmManager->ReleaseAnm(i);
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_TITLE01, "data/title01.anm", ANM_OFFSET_TITLE01))
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_TITLE02, "data/title02.anm", ANM_OFFSET_TITLE02))
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_TITLE03, "data/title03.anm", ANM_OFFSET_TITLE03))
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_TITLE04, "data/title04.anm", ANM_OFFSET_TITLE04))
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_TITLE01S, "data/title01s.anm", ANM_OFFSET_TITLE01S))
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_TITLE04S, "data/title04s.anm", ANM_OFFSET_TITLE04S))
    {
        return ZUN_ERROR;
    }

    for (i = 0; i < 80; i++)
    {
        g_AnmManager->ExecuteAnmIdx(&menu->vm[i], ANM_SCRIPT_TITLE01_START + i);
        menu->vm[i].flags.isVisible = false;
        menu->vm[i].baseSpriteIndex = menu->vm[i].activeSpriteIndex;
        menu->vm[i].flags.zWriteDisable = true;
    }

    if (g_AnmManager->LoadSurface(0, "data/title/title00.jpg"))
    {
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

ZunResult MainMenu::LoadDiffCharSelect(MainMenu *menu)
{
    AnmVm *vm;
    i32 i;

    for (i = ANM_FILE_TITLE01; i <= ANM_FILE_TITLE04; i++)
    {
        g_AnmManager->ReleaseAnm(i);
    }
    if (g_AnmManager->LoadSurface(0, "data/title/select00.jpg") != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SELECT01, "data/select01.anm", ANM_OFFSET_SELECT01) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SELECT02, "data/select02.anm", ANM_OFFSET_SELECT02) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SELECT03, "data/select03.anm", ANM_OFFSET_SELECT03) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SELECT04, "data/select04.anm", ANM_OFFSET_SELECT04) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SELECT05, "data/select05.anm", ANM_OFFSET_SELECT05) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SLPL00A, "data/slpl00a.anm", ANM_OFFSET_SLPL00A) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SLPL00B, "data/slpl00b.anm", ANM_OFFSET_SLPL00B) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SLPL01A, "data/slpl01a.anm", ANM_OFFSET_SLPL01A) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_SLPL01B, "data/slpl01b.anm", ANM_OFFSET_SLPL01B) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    for (vm = &menu->vm[80], i = ANM_SCRIPT_SELECT01_START; i <= ANM_SCRIPT_SELECT01_END; i++, vm++)
    {
        g_AnmManager->ExecuteAnmIdx(vm, i);
        vm->flags.isVisible = false;
        vm->flags.colorOp = AnmColorOp_Add;
        if (!g_Supervisor.IsHardwareBlendingDisabled())
        {
            vm->color = COLOR_BLACK;
        }
        else
        {
            vm->color = COLOR_WHITE;
        }
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->baseSpriteIndex = vm->activeSpriteIndex;
        vm->flags.zWriteDisable = true;
    }
    return ZUN_SUCCESS;
}

#pragma var_order(fileIdx, vm)
ZunResult MainMenu::LoadReplayMenu(MainMenu *menu)
{
    AnmVm *vm;
    i32 fileIdx;

    for (fileIdx = ANM_FILE_TITLE01; fileIdx <= ANM_FILE_TITLE04; fileIdx++)
    {
        g_AnmManager->ReleaseAnm(fileIdx);
    }

    if (g_AnmManager->LoadSurface(0, "data/title/select00.jpg") != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    if (g_AnmManager->LoadAnm(ANM_FILE_REPLAY, "data/replay00.anm", ANM_OFFSET_REPLAY) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    vm = &menu->vm[96];
    for (fileIdx = ANM_SCRIPT_REPLAY_START; fileIdx <= ANM_SCRIPT_REPLAY_END; fileIdx++, vm++)
    {
        g_AnmManager->ExecuteAnmIdx(vm, fileIdx);
        vm->flags.isVisible = false;
        vm->flags.colorOp = AnmColorOp_Add;

        if (!g_Supervisor.IsHardwareBlendingDisabled())
        {
            vm->color = COLOR_BLACK;
        }
        else
        {
            vm->color = COLOR_WHITE;
        }
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->baseSpriteIndex = vm->activeSpriteIndex;
        vm->flags.zWriteDisable = true;
    }
    return ZUN_SUCCESS;
}

ZunResult MainMenu_RegisterChain(ZunBool isDemo)
{
    MainMenu *menu = &g_MainMenu;

    memset(menu, 0, sizeof(MainMenu));
    g_GameManager.isInGameMenu = 0;
    utils::DebugPrint(TH_DBG_MAINMENU_VRAM, g_Supervisor.d3dDevice->GetAvailableTextureMem());
    menu->gameState = isDemo ? STATE_REPLAY_LOAD : STATE_STARTUP;
    g_Supervisor.framerateMultiplier = 0.0f;
    menu->chainCalc = g_Chain.CreateElem((ChainCallback)MainMenu::OnUpdate);
    menu->chainCalc->arg = menu;
    menu->chainCalc->addedCallback = (ChainAddedCallback)MainMenu::AddedCallback;
    menu->chainCalc->deletedCallback = (ChainDeletedCallback)MainMenu::DeletedCallback;
    menu->stateTimer = 0;
    if (g_Chain.AddToCalcChain(menu->chainCalc, TH_CHAIN_PRIO_CALC_MAINMENU) != 0)
    {
        return ZUN_ERROR;
    }
    menu->chainDraw = g_Chain.CreateElem((ChainCallback)MainMenu::OnDraw);
    menu->chainDraw->arg = menu;
    g_Chain.AddToDrawChain(menu->chainDraw, TH_CHAIN_PRIO_DRAW_MAINMENU);
    menu->lastFrameTime = 0;
    menu->stateTimer = 60;
    menu->frameCountForRefreshRateCalc = 0;
    return ZUN_SUCCESS;
}

ZunResult MainMenu::AddedCallback(MainMenu *menu)
{
#if !TRIALBUILD
    if (!g_GameManager.demoMode)
#endif
    {
        g_Supervisor.SetupMidiPlayback("bgm/th06_01.mid");
    }

    // TODO: Make this an inline function?
    g_AnmManager->ClearScriptRange(ANM_OFFSET_TITLE01, ANM_OFFSET_TITLE01S - ANM_OFFSET_TITLE01);
    menu->unk_81e4 = 0;

    switch (g_Supervisor.prevState)
    {
    case SUPERVISOR_STATE_GAMEMANAGER:
    case SUPERVISOR_STATE_NEXT_STAGE:
    case SUPERVISOR_STATE_RESULTSCREEN_FROMGAME:
        menu->cursor = g_GameManager.difficulty == EXTRA;
        break;
    case SUPERVISOR_STATE_RESULTSCREEN:
        menu->cursor = 4;
        break;
    case SUPERVISOR_STATE_MUSICROOM:
        menu->cursor = 5;
        break;
    case SUPERVISOR_STATE_INIT:
    case SUPERVISOR_STATE_MAINMENU:
    default:
        menu->cursor = 0;
    }

    if (g_GameManager.isInPracticeMode)
    {
        menu->cursor = 2;
    }

    g_GameManager.isInPracticeMode = false;
    if (!g_Supervisor.IsHardwareBlendingDisabled())
    {
        menu->color1 = 0x80004000;
        menu->color2 = 0xff008000;
    }
    else
    {
        menu->color1 = 0x80ffffff;
        menu->color2 = COLOR_WHITE;
    }
    menu->minimumOpacity = 0;
    menu->menuTextColor = 0x40000000;
    menu->numFramesSinceActive = 0;
    menu->framesActive = 0;
    menu->unk_10f28 = 0x10;
    menu->currentReplay = NULL;
    ScoreDat *scoredat = OpenScore("score.dat");
    ParseClrd(scoredat, g_GameManager.clrd);
    ParsePscr(scoredat, (Pscr *)g_GameManager.pscr);
    ReleaseScoreDat(scoredat);
    if (!g_GameManager.demoMode)
    {
        if (g_Supervisor.startupTimeBeforeMenuMusic == 0)
        {
            g_Supervisor.PlayAudio("bgm/th06_01.mid");
            ScreenEffect::RegisterChain(SCREEN_EFFECT_FADE_IN, 120, 0xffffff, 0, 0);
        }
        else
        {
            ScreenEffect::RegisterChain(SCREEN_EFFECT_FADE_IN, 200, 0xffffff, 0, 0);
        }
    }
    g_GameManager.demoMode = false;
    g_GameManager.demoFrames = 0;
    return ZUN_SUCCESS;
}

ZunResult MainMenu::DeletedCallback(MainMenu *menu)
{
    g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
    MainMenu::ReleaseTitleAnm();
    for (i32 i = ANM_FILE_SELECT01; i <= ANM_FILE_REPLAY; i++)
    {
        g_AnmManager->ReleaseAnm(i);
    }
    g_AnmManager->ReleaseSurface(0);
    g_AnmManager->ClearScriptRange(ANM_OFFSET_TITLE01, ANM_OFFSET_TITLE01S - ANM_OFFSET_TITLE01);
    g_Chain.Cut(menu->chainDraw);
    menu->chainDraw = NULL;

    ZUN_FREE(menu->currentReplay);
    return ZUN_SUCCESS;
}

void MainMenu::ReleaseTitleAnm()
{
    // There's a bit of an off-by-one error here, where it frees
    // ANM_FILE_SELECT01 in addition to the titles. I'm pretty sure this is
    // unintentional.
    for (i32 i = ANM_FILE_TITLE01; i <= ANM_FILE_SELECT01; i++)
    {
        g_AnmManager->ReleaseAnm(i);
    }
}
} // namespace th06

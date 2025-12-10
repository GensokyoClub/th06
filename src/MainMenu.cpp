#include <D3DX8.h>
#include <cstdio>
#include <direct.h>
#include <windows.h>

#include "MainMenu.hpp"

#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "ChainPriorities.hpp"
#include "Filesystem.hpp"
#include "GameErrorContext.hpp"
#include "GameManager.hpp"
#include "ReplayData.hpp"
#include "ReplayManager.hpp"
#include "ResultScreen.hpp"
#include "ScreenEffect.hpp"
#include "SoundPlayer.hpp"
#include "Supervisor.hpp"
#include "ZunColor.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th06
{
DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 4, g_ShortCharacterList) = {"ReimuA ", "ReimuB ", "MarisaA", "MarisaB"};
DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 5, g_DifficultyList) = {"Easy   ", "Normal ", "Hard   ", "Lunatic", "Extra  "};
DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 7, g_StageList) = {"Stage1", "Stage2", "Stage3", "Stage4",
                                                        "Stage5", "Stage6", "Extra "};

DIFFABLE_STATIC(i16, g_LastJoystickInput)
MainMenu::MainMenu()
{
    // what?
    int waste1, waste2, waste3, waste4;
}

#pragma function(strcpy)
#pragma var_order(i, vmList, time, deltaTime, deltaTimeAsFrames, deltaTimeAsMs, mapping, startedUp, sVar1,             \
                  controllerData, mappingData, refreshRate, local_48, local_4c, chosenStage, pos1, pos2, pos3, pos4,   \
                  pos5, vm, hasLoadedSprite)
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
    i16 sVar1;
    u8 *controllerData;
    ControllerMapping mappingData;
    f32 refreshRate;
    f32 local_48;
    i32 local_4c;
    u32 chosenStage;
    D3DXVECTOR3 pos1;
    D3DXVECTOR3 pos2;
    D3DXVECTOR3 pos3;
    D3DXVECTOR3 pos4;
    D3DXVECTOR3 pos5;
    AnmVm *vm;
    u32 hasLoadedSprite;

    if (menu->timeRelatedArrSize < ARRAY_SIZE_SIGNED(menu->timeRelatedArr))
    {
        timeBeginPeriod(1);
        if (menu->lastFrameTime == 0)
        {
            menu->lastFrameTime = timeGetTime();
        }
        time = timeGetTime();
        timeEndPeriod(1);
        menu->frameCountForRefreshRateCalc = menu->frameCountForRefreshRateCalc + 1;
        deltaTime = time - menu->lastFrameTime;
        if (deltaTime >= 700)
        {
            menu->lastFrameTime = time;
            menu->frameCountForRefreshRateCalc = 0;
        }
        else
        {
            if (500 <= deltaTime)
            {
                deltaTimeAsMs = deltaTime / 1000.f;
                deltaTimeAsFrames = menu->frameCountForRefreshRateCalc * 1000.f / deltaTime;
                if (deltaTimeAsFrames >= 57.f)
                {
                    menu->timeRelatedArr[menu->timeRelatedArrSize] = deltaTimeAsFrames;
                    menu->timeRelatedArrSize = menu->timeRelatedArrSize + 1;
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
    case STATE_PRE_INPUT:
        menu->idleFrames = menu->idleFrames + 1;
        if ((g_CurFrameInput & 0xffff) != 0)
        {
            menu->idleFrames = 0;
        }
        if (720 <= menu->idleFrames)
        {
            goto load_menu_rpy;
        }
        if (menu->WeirdSecondInputCheck())
            break;
        menu->idleFrames = 0;
    case STATE_MAIN_MENU:
        menu->DrawStartMenu();
        if ((g_CurFrameInput & 0xffff) != 0)
        {
            menu->idleFrames = 0;
        }
        menu->idleFrames = menu->idleFrames + 1;
        if (720 <= menu->idleFrames)
        {
        load_menu_rpy:
            g_GameManager.isInReplay = 1;
            g_GameManager.demoMode = 1;
            g_GameManager.demoFrames = 0;
            g_Supervisor.framerateMultiplier = 1.0;
            strcpy(g_GameManager.replayFile, "data/demo/demo00.rpy");
            g_GameManager.currentStage = 3;
            g_GameManager.difficulty = LUNATIC;
            g_Supervisor.curState = SUPERVISOR_STATE_GAMEMANAGER;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
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
            DrawMenuItem(vmList, i, menu->cursor, menu->color2, menu->color1, 0x73);
        }
        for (i = 0; i < 9; i++, vmList++)
        {
            if (menu->controlMapping[i] < 0)
            {
                vmList->flags.flag1 = 0;
                continue;
            }
            vmList->flags.flag1 = 1;
            DrawMenuItem(vmList, i, menu->cursor, menu->color2, menu->color1, 0x73);
        }
        for (i = 0; i < 18; i++, vmList++)
        {
            if (menu->controlMapping[i / 2] < 0)
            {
                vmList->flags.flag1 = 0;
                continue;
            }
            vmList->flags.flag1 = 1;
            mapping = menu->controlMapping[i / 2];
            if (i % 2 == 0)
            {
                g_AnmManager->SetActiveSprite(vmList, mapping / 10 + ANM_SPRITE_TITLE01_START);
            }
            else
            {
                g_AnmManager->SetActiveSprite(vmList, mapping % 10 + ANM_SPRITE_TITLE01_START);
            }
            vmList->baseSpriteIndex = vmList->activeSpriteIndex;
            DrawMenuItem(vmList, i / 2, menu->cursor, menu->color2, menu->color1, 0x7a);
        }
        if (32 <= menu->stateTimer)
        {
            controllerData = Controller::GetControllerState();
            for (sVar1 = 0; sVar1 < 32; sVar1++)
            {
                if ((controllerData[sVar1] & 0x80) != 0)
                    break;
            }
            if (sVar1 < 32 && g_LastJoystickInput != sVar1)
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                switch (menu->cursor)
                {
                case 0:
                    SwapMapping(menu, sVar1, menu->controlMapping[0], 1);
                    menu->controlMapping[0] = sVar1;
                    break;
                case 1:
                    SwapMapping(menu, sVar1, menu->controlMapping[1], 0);
                    menu->controlMapping[1] = sVar1;
                    break;
                case 2:
                    SwapMapping(menu, sVar1, menu->controlMapping[2], 1);
                    menu->controlMapping[2] = sVar1;
                    break;
                case 3:
                    SwapMapping(menu, sVar1, menu->controlMapping[3], 0);
                    menu->controlMapping[3] = sVar1;
                    break;
                case 4:
                    SwapMapping(menu, sVar1, menu->controlMapping[4], 0);
                    menu->controlMapping[4] = sVar1;
                    break;
                case 5:
                    SwapMapping(menu, sVar1, menu->controlMapping[5], 0);
                    menu->controlMapping[5] = sVar1;
                    break;
                case 6:
                    SwapMapping(menu, sVar1, menu->controlMapping[6], 0);
                    menu->controlMapping[6] = sVar1;
                    break;
                case 7:
                    SwapMapping(menu, sVar1, menu->controlMapping[7], 0);
                    menu->controlMapping[7] = sVar1;
                    break;
                case 8:
                    SwapMapping(menu, sVar1, menu->controlMapping[8], 0);
                    menu->controlMapping[8] = sVar1;
                }
            }
            g_LastJoystickInput = sVar1;
            if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
            {
                switch (menu->cursor)
                {
                case 9:
                    mappingData.shootButton = 0;
                    mappingData.bombButton = 1;
                    mappingData.focusButton = 0;
                    mappingData.menuButton = 0xffff;
                    mappingData.upButton = 0xffff;
                    mappingData.downButton = 0xffff;
                    mappingData.leftButton = 0xffff;
                    mappingData.rightButton = 0xffff;
                    mappingData.skipButton = 0xffff;
                    memcpy(menu->controlMapping, &mappingData, sizeof(ControllerMapping));
                    break;
                case 10:
                    menu->gameState = STATE_OPTIONS;
                    menu->stateTimer = 0;
                    for (sVar1 = 0; sVar1 < ARRAY_SIZE_SIGNED(menu->vm); sVar1++)
                    {
                        menu->vm[sVar1].pendingInterrupt = 3;
                    }
                    menu->cursor = 7;
                    g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
                    memcpy(&g_ControllerMapping, menu->controlMapping, sizeof(ControllerMapping));
                    memcpy(&g_Supervisor.cfg.controllerMapping, menu->controlMapping, sizeof(ControllerMapping));
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
            if (g_GameManager.difficulty < 4)
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
        vmList = &menu->vm[81];
        if (g_GameManager.difficulty < 4)
        {
            MoveCursor(menu, 4);
            for (i = 0; i < 4; i++, vmList++)
            {
                if (i != menu->cursor)
                {
                    if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                    {
                        vmList->color = 0x60000000;
                    }
                    else
                    {
                        vmList->color = 0x60ffffff;
                    }
                    pos1.x = 0.0;
                    pos1.y = 0.0;
                    pos1.z = 0.0;
                    memcpy(vmList->posOffset, &pos1, sizeof(D3DXVECTOR3));
                    vmList->alphaInterpEndTime = 0;
                }
                else
                {
                    if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                    {
                        vmList->color = COLOR_BLACK;
                    }
                    else
                    {
                        vmList->color = COLOR_WHITE;
                    }
                    pos2.x = -6.0f;
                    pos2.y = -6.0f;
                    pos2.z = 0.0;
                    memcpy(vmList->posOffset, &pos2, sizeof(D3DXVECTOR3));
                }
            }
            vmList->flags.flag1 = 0;
        }
        else
        {
            for (i = 0; i < 4; i++, vmList++)
            {
                vmList->flags.flag1 = 0;
            }
            for (i = 4; i < 5; i++, vmList++)
            {
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vmList->color = COLOR_BLACK;
                }
                else
                {
                    vmList->color = COLOR_WHITE;
                }
                pos3.x = -6.0f;
                pos3.y = -6.0f;
                pos3.z = 0.0;
                memcpy(vmList->posOffset, &pos3, sizeof(D3DXVECTOR3));
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
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            if (g_GameManager.difficulty < 4)
            {
                g_Supervisor.cfg.defaultDifficulty = menu->cursor;
                if (g_GameManager.isInPracticeMode == 0)
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
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
            if (g_GameManager.difficulty < 4)
            {
                vmList = &menu->vm[81 + menu->cursor];
                vmList->pendingInterrupt = 8;
                g_GameManager.difficulty = (Difficulty)menu->cursor;
                menu->cursor = g_GameManager.character;
            }
            else
            {
                vmList = &menu->vm[85];
                vmList->pendingInterrupt = 8;
                g_GameManager.difficulty = EXTRA;
                if (g_GameManager.HasReachedMaxClears(g_GameManager.character, 0) ||
                    g_GameManager.HasReachedMaxClears(g_GameManager.character, 1))
                {
                    menu->cursor = g_GameManager.character;
                }
                else
                {
                    menu->cursor = 1 - g_GameManager.character;
                }
            }
            g_Supervisor.cfg.defaultDifficulty = g_GameManager.difficulty;
            vmList = &menu->vm[86];
            for (i = 0; i < 2; i++, vmList += 2)
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
        if (WAS_PRESSED_WEIRD(TH_BUTTON_LEFT))
        {
            menu->cursor = menu->cursor + 1;
            if (2 <= menu->cursor)
            {
                menu->cursor = menu->cursor - 2;
            }
            if (g_GameManager.difficulty == EXTRA && g_GameManager.HasReachedMaxClears(menu->cursor, 0) == 0 &&
                g_GameManager.HasReachedMaxClears(menu->cursor, 1) == 0)
            {
                menu->cursor = menu->cursor - 1;
                if (menu->cursor < 0)
                {
                    menu->cursor = menu->cursor + 2;
                }
                goto here;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
            vmList = &menu->vm[86];
            for (i = 0; i < 2; i++, vmList++)
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
        if (WAS_PRESSED_WEIRD(TH_BUTTON_RIGHT))
        {
            menu->cursor = menu->cursor - 1;
            if (menu->cursor < 0)
            {
                menu->cursor = menu->cursor + 2;
            }
            if (g_GameManager.difficulty == EXTRA && g_GameManager.HasReachedMaxClears(menu->cursor, 0) == 0 &&
                g_GameManager.HasReachedMaxClears(menu->cursor, 1) == 0)
            {
                menu->cursor = menu->cursor + 1;
                if (2 <= menu->cursor)
                {
                    menu->cursor = menu->cursor - 2;
                }
            }
            else
            {
                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                vmList = &menu->vm[86];
                for (i = 0; i < 2; i++, vmList++)
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
            if (g_GameManager.difficulty < 4)
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
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
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
            vmList = &menu->vm[g_GameManager.difficulty + 81];
            vmList->pendingInterrupt = 0;
            vmList = &menu->vm[86];
            for (i = 0; i < 2; i++, vmList += 2)
            {
                if (i != menu->cursor)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            vmList = &menu->vm[92];
            for (i = 0; i < 2; i++, vmList += 2)
            {
                if (i != menu->cursor)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            g_GameManager.character = menu->cursor;
            if (g_GameManager.difficulty < 4)
            {
                menu->cursor = g_GameManager.shotType;
            }
            else
            {
                if (g_GameManager.HasReachedMaxClears(g_GameManager.character, g_GameManager.shotType) != 0)
                {
                    menu->cursor = g_GameManager.shotType;
                }
                else
                {
                    menu->cursor = 1 - g_GameManager.shotType;
                }
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
        }
        break;
    case STATE_SHOT_SELECT:
        MoveCursor(menu, 2);
        if (g_GameManager.difficulty == EXTRA &&
            g_GameManager.HasReachedMaxClears(g_GameManager.character, menu->cursor) == 0)
        {
            menu->cursor = 1 - menu->cursor;
        }
        vmList = &menu->vm[92];
        for (i = 0; i < 2; i++, vmList += 2)
        {
            vmList[1].flags.colorOp = AnmVmColorOp_Add;
        }
        vmList = &menu->vm[92 + g_GameManager.character * 2];
        for (i = 0; i < 2; i++, vmList++)
        {
            vmList->flags.colorOp = AnmVmColorOp_Add;
            vmList->flags.isVisible = 1;
            if (i != menu->cursor)
            {
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vmList->color = 0xa0000000;
                }
                else
                {
                    vmList->color = 0xa0d0d0d0;
                }
                pos4.x = 0.0;
                pos4.y = 0.0;
                pos4.z = 0.0;
                memcpy(&vmList->posOffset, &pos4, sizeof(D3DXVECTOR3));
            }
            else
            {
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vmList->color = 0xff202020;
                }
                else
                {
                    vmList->color = 0xffffffff;
                }
                pos5.x = -6.f;
                pos5.y = -6.f;
                pos5.z = 0.0;
                memcpy(&vmList->posOffset, &pos5, sizeof(D3DXVECTOR3));
            }
        }
        if (30 > menu->stateTimer)
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
            vmList = &menu->vm[92];
            for (i = 0; i < 2; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            vmList = &menu->vm[81 + g_GameManager.difficulty];
            vmList->pendingInterrupt = 0;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            g_GameManager.shotType = menu->cursor;
            menu->cursor = g_GameManager.character;
            vmList = &menu->vm[86];
            for (i = 0; i < 2; i++, vmList += 2)
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
        {
            g_GameManager.shotType = menu->cursor;
            if (g_GameManager.isInPracticeMode == 0)
            {
                if (g_GameManager.difficulty < 4)
                {
                    g_GameManager.currentStage = 0;
                }
                else
                {
                    g_GameManager.currentStage = 6;
                }
            something:
                g_GameManager.livesRemaining = g_Supervisor.cfg.lifeCount;
                g_GameManager.bombsRemaining = g_Supervisor.cfg.bombCount;
                if ((g_GameManager.difficulty == EXTRA) || (g_GameManager.isInPracticeMode != 0))
                {
                    g_GameManager.livesRemaining = 2;
                    g_GameManager.bombsRemaining = 3;
                }
                g_Supervisor.curState = 2;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                g_GameManager.isInReplay = 0;
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
                    local_48 = 60.f;
                }

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
                    refreshRate = 1.0;
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
            vmList = &menu->vm[81 + g_GameManager.difficulty];
            vmList->pendingInterrupt = 0;
            vmList = &menu->vm[86];
            for (i = 0; i < 2; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            vmList = &menu->vm[92];
            for (i = 0; i < 2; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            menu->cursor = g_GameManager.menuCursorBackup;
            local_4c = g_GameManager.clrd[g_GameManager.CharacterShotType()]
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
    case STATE_PRACTICE_LVL_SELECT:
        chosenStage = g_GameManager.clrd[g_GameManager.CharacterShotType()]
                                  .difficultyClearedWithoutRetries[g_GameManager.difficulty] > 6
                          ? 6
                          : g_GameManager.clrd[g_GameManager.CharacterShotType()]
                                .difficultyClearedWithoutRetries[g_GameManager.difficulty];
        if (g_GameManager.difficulty == EASY && chosenStage == 6)
        {
            chosenStage = 5;
        }
        MoveCursor(menu, chosenStage);
        if (30 > menu->stateTimer)
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
            vmList = &menu->vm[81 + g_GameManager.difficulty];
            vmList->pendingInterrupt = 0;
            vmList = &menu->vm[86];
            for (i = 0; i < 2; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            vmList = &menu->vm[92];
            for (i = 0; i < 2; i++, vmList += 2)
            {
                if (i != g_GameManager.character)
                {
                    vmList[0].pendingInterrupt = 0;
                    vmList[1].pendingInterrupt = 0;
                }
            }
            menu->cursor = g_GameManager.shotType;
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
            break;
        }
        else if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            g_GameManager.currentStage = menu->cursor;
            g_GameManager.menuCursorBackup = menu->cursor;
            goto something;
        }
        break;
    case STATE_QUIT:
        if (60 <= menu->stateTimer)
        {
            g_Supervisor.curState = SUPERVISOR_STATE_EXITSUCCESS;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    case STATE_SCORE:
        if (60 <= menu->stateTimer)
        {
            g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    case STATE_MUSIC_ROOM:
        if (60 <= menu->stateTimer)
        {
            g_Supervisor.curState = SUPERVISOR_STATE_MUSICROOM;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        break;
    }
    menu->stateTimer = menu->stateTimer + 1;
    for (i = 0; i < ARRAY_SIZE_SIGNED(menu->vm); i++)
    {
        vm = &menu->vm[i];
        if (vm->sprite == NULL)
        {
            hasLoadedSprite = false;
        }
        else if (vm->sprite->sourceFileIndex < 0)
        {
            hasLoadedSprite = false;
        }
        else
        {
            hasLoadedSprite = g_AnmManager->textures[vm->sprite->sourceFileIndex] != NULL;
        }
        if (hasLoadedSprite)
        {
            g_AnmManager->ExecuteScript(&menu->vm[i]);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}
#pragma intrinsic(strcpy)

CursorMovement MainMenu::MoveCursor(MainMenu *menu, i32 menuLength)
{
    if (WAS_PRESSED_WEIRD(TH_BUTTON_UP))
    {
        menu->cursor--;
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
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

    if (WAS_PRESSED_WEIRD(TH_BUTTON_DOWN))
    {
        menu->cursor++;
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
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

void MainMenu::SwapMapping(MainMenu *menu, i16 btnPressed, i16 oldMapping, ZunBool unk)
{
    if (unk == 0 && menu->controlMapping[0] == btnPressed)
    {
        menu->controlMapping[0] = oldMapping;
    }
    if (menu->controlMapping[1] == btnPressed)
    {
        menu->controlMapping[1] = oldMapping;
    }
    if (unk == 0 && menu->controlMapping[2] == btnPressed)
    {
        menu->controlMapping[2] = oldMapping;
    }
    if (menu->controlMapping[4] == btnPressed)
    {
        menu->controlMapping[4] = oldMapping;
    }
    if (menu->controlMapping[5] == btnPressed)
    {
        menu->controlMapping[5] = oldMapping;
    }
    if (menu->controlMapping[6] == btnPressed)
    {
        menu->controlMapping[6] = oldMapping;
    }
    if (menu->controlMapping[7] == btnPressed)
    {
        menu->controlMapping[7] = oldMapping;
    }
    if (menu->controlMapping[3] == btnPressed)
    {
        menu->controlMapping[3] = oldMapping;
    }
    if (menu->controlMapping[8] == btnPressed)
    {
        menu->controlMapping[8] = oldMapping;
    }
}

void MainMenu::DrawMenuItem(AnmVm *vm, int itemNumber, int cursor, D3DCOLOR currentItemColor, D3DCOLOR otherItemColor,
                            int vm_amount)
{
    D3DXVECTOR3 otherItemPos;
    D3DXVECTOR3 currentItemPos;

    if (itemNumber == cursor)
    {
        if (!g_Supervisor.cfg.IsSoftwareTexturing())
        {
            vm->color = currentItemColor;
        }
        else
        {
            g_AnmManager->SetActiveSprite(vm, vm->baseSpriteIndex + vm_amount);
            vm->color = currentItemColor & D3DCOLOR_RGBA(0x00, 0x00, 0x00, 0xff) |
                        D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0x00); // just... why?
        }

        currentItemPos.x = -4.0f;
        currentItemPos.y = -4.0f;
        currentItemPos.z = 0.0f;
        vm->posOffset = currentItemPos;
    }
    else
    {
        if (!g_Supervisor.cfg.IsSoftwareTexturing())
        {
            vm->color = otherItemColor;
        }

        else
        {
            g_AnmManager->SetActiveSprite(vm, vm->baseSpriteIndex);
            vm->color = otherItemColor & D3DCOLOR_RGBA(0x00, 0x00, 0x00, 0xff) |
                        D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0x00); // again, why?
        }
        otherItemPos.x = 0.0f;
        otherItemPos.y = 0.0f;
        otherItemPos.z = 0.0f;
        vm->posOffset = otherItemPos;
    }
}

#pragma var_order(time, i, vector3Ptr)
ZunResult MainMenu::BeginStartup()
{
    D3DXVECTOR3 vector3Ptr;
    DWORD time;
    int i;

    if (LoadTitleAnm(this) != ZUN_SUCCESS)
    {
        g_Supervisor.curState = SUPERVISOR_STATE_EXITSUCCESS;
        return ZUN_ERROR;
    }
    if (g_Supervisor.startupTimeBeforeMenuMusic > 0)
    {
        time = timeGetTime();
        while ((time - g_Supervisor.startupTimeBeforeMenuMusic >= 0) &&
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
        this->vm[i].flags.colorOp = AnmVmColorOp_Add;
        if ((g_Supervisor.cfg.opts & (1 << GCOS_USE_D3D_HW_TEXTURE_BLENDING)) == 0)
        {
            this->vm[i].color = COLOR_BLACK;
        }
        else
        {
            this->vm[i].color = COLOR_WHITE;
        }
        vector3Ptr.x = 0.0;
        vector3Ptr.y = 0.0;
        vector3Ptr.z = 0.0;
        this->vm[i].posOffset = vector3Ptr;
    }
    this->gameState = STATE_PRE_INPUT;
    return ZUN_SUCCESS;
}

#pragma var_order(vm, d3dVec)
ZunBool MainMenu::WeirdSecondInputCheck()
{
    i32 vm;
    D3DXVECTOR3 d3dVec;

    if (this->stateTimer < 0x1e)
    {
        return true;
    }

    if (!WAS_PRESSED_WEIRD(TH_BUTTON_SELECTMENU | TH_BUTTON_BOMB | TH_BUTTON_MENU | TH_BUTTON_Q | TH_BUTTON_S))
    {
        return true;
    }

    this->stateTimer = 0;
    this->gameState = STATE_MAIN_MENU;
    for (vm = 0; vm < 122; vm++)
    {
        this->vm[vm].pendingInterrupt = 2;
    }
    if (!((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1))
    {
        this->vm[this->cursor].color = COLOR_RED;
    }
    else
    {
        this->vm[this->cursor].color = COLOR_PINK;
    }
    d3dVec.x = -6.0;
    d3dVec.y = -6.0;
    d3dVec.z = 0.0;
    this->vm[this->cursor].posOffset = d3dVec;

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
    if ((this->cursor == 1) && !g_GameManager.HasReachedMaxClears(0, 0) && !g_GameManager.HasReachedMaxClears(0, 1) &&
        !g_GameManager.HasReachedMaxClears(1, 0) && !g_GameManager.HasReachedMaxClears(1, 1))
    {
        this->cursor += i;
    }
    AnmVm *drawVm = this->vm;
    for (i = 0; i < 8; i++, drawVm++ /* zun why */)
    {
        DrawMenuItem(drawVm, i, this->cursor, COLOR_RED, COLOR_START_MENU_ITEM_INACTIVE, 122);
    }
    if (this->stateTimer >= 0x14)
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
                g_GameManager.isInPracticeMode = 0;
                if (EXTRA <= g_GameManager.difficulty)
                {
                    g_GameManager.difficulty = NORMAL;
                }
                if (EXTRA <= g_Supervisor.cfg.defaultDifficulty)
                {
                    g_Supervisor.cfg.defaultDifficulty = NORMAL;
                }
                this->stateTimer = 0;
                this->minimumOpacity = 0x40000000;
                this->menuTextColor = COLOR_BLACK;
                this->numFramesSinceActive = 0;
                this->framesActive = 60;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                break;
            case 1:
                if (!(!g_GameManager.HasReachedMaxClears(0, 0) && !g_GameManager.HasReachedMaxClears(0, 1) &&
                      !g_GameManager.HasReachedMaxClears(1, 0) && !g_GameManager.HasReachedMaxClears(1, 1)))
                {
                    for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                    {
                        this->vm[i].pendingInterrupt = 4;
                    }
                    this->gameState = STATE_DIFFICULTY_LOAD;
                    g_GameManager.isInPracticeMode = 0;
                    g_GameManager.difficulty = EXTRA;
                    this->stateTimer = 0;
                    this->minimumOpacity = 0x40000000;
                    this->menuTextColor = COLOR_BLACK;
                    this->numFramesSinceActive = 0;
                    this->framesActive = 60;
                    g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                }
                else
                {
                    g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
                }
                break;
            case 2:
                g_GameManager.isInPracticeMode = 1;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                this->gameState = STATE_DIFFICULTY_LOAD;
                if (EXTRA <= g_GameManager.difficulty)
                {
                    g_GameManager.difficulty = NORMAL;
                }
                if (EXTRA <= g_Supervisor.cfg.defaultDifficulty)
                {
                    g_Supervisor.cfg.defaultDifficulty = NORMAL;
                }
                this->stateTimer = 0;
                this->minimumOpacity = 0x40000000;
                this->menuTextColor = COLOR_BLACK;
                this->numFramesSinceActive = 0;
                this->framesActive = 60;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                break;
            case 3:
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                this->gameState = STATE_REPLAY_LOAD;
                g_GameManager.isInPracticeMode = 0;
                this->stateTimer = 0;
                this->minimumOpacity = 0x40000000;
                this->menuTextColor = COLOR_BLACK;
                this->numFramesSinceActive = 0;
                this->framesActive = 60;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
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
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                break;
            case 5:
                this->gameState = STATE_MUSIC_ROOM;
                this->stateTimer = 0;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
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
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                break;
            case 7:
                this->gameState = STATE_QUIT;
                this->stateTimer = 0;
                for (i = 0; i < ARRAY_SIZE_SIGNED(this->vm); i++)
                {
                    this->vm[i].pendingInterrupt = 4;
                }
                g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
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
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            this->cursor = 7;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
        }
    }
    return ZUN_SUCCESS;
}

#pragma function(strcpy)
#pragma var_order(anmVm, cur, replayFileHandle, replayFileIdx, replayData, replayFilePath, replayFileInfo, uh, uh2,    \
                  padding)
i32 MainMenu::ReplayHandling()
{
    AnmVm *anmVm;
    i32 cur;
    HANDLE replayFileHandle;
    u32 replayFileIdx;
    ReplayData *replayData;
    char replayFilePath[32];
    WIN32_FIND_DATA replayFileInfo;
    u8 padding[0x20]; // idk

    switch (this->gameState)
    {
    case STATE_REPLAY_LOAD:
        if (this->stateTimer == 60)
        {
            if (LoadReplayMenu(this))
            {
                g_GameErrorContext.Log("japanese");
                g_Supervisor.curState = SUPERVISOR_STATE_EXITSUCCESS;
                return ZUN_SUCCESS;
            }
            else
            {
                replayFileIdx = 0;
                for (cur = 0; cur < 15; cur++)
                {
                    sprintf(replayFilePath, "./replay/th6_%.2d.rpy", cur + 1);
                    replayData = (ReplayData *)FileSystem::OpenPath(replayFilePath, 1);
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
                    free(replayData);
                }
                _mkdir("./replay");
                _chdir("./replay");
                replayFileHandle = FindFirstFileA("th6_ud????.rpy", &replayFileInfo);
                if (replayFileHandle != INVALID_HANDLE_VALUE)
                {
                    for (cur = 0; cur < 0x2d; cur++)
                    {
                        replayData = (ReplayData *)FileSystem::OpenPath(replayFilePath, 1);
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
                        free(replayData);
                        if (!FindNextFileA(replayFileHandle, &replayFileInfo))
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
        if (this->stateTimer == 0x24)
        {
            this->gameState = STATE_STARTUP;
            this->stateTimer = 0;
        }
        break;
    case STATE_REPLAY_ANIM:
        if (this->stateTimer < 0x28)
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
                for (cur = 0; cur < 0x19; cur += 1, anmVm++)
                {
                    anmVm->pendingInterrupt = 0x11;
                }
                anmVm = &this->vm[99 + this->chosenReplay];
                anmVm->pendingInterrupt = 0x10;
                this->stateTimer = 0;
                this->cursor = 0;
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);
                this->currentReplay = (ReplayData *)FileSystem::OpenPath(this->replayFilePaths[this->chosenReplay], 1);
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
                    this->cursor = this->cursor + 1;

                    if ((int)this->cursor >= ARRAY_SIZE_SIGNED(this->currentReplay->stageReplayData))
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
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->cursor = 0;
            break;
        }
        break;
    case STATE_REPLAY_SELECT:
        if (this->stateTimer < 0x28)
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
            g_GameManager.isInReplay = 1;
            g_Supervisor.framerateMultiplier = 1.0;
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
            ReplayData *uh = this->currentReplay;
            free(uh);
            this->currentReplay = NULL;
            g_GameManager.currentStage = this->cursor;
            g_Supervisor.curState = SUPERVISOR_STATE_GAMEMANAGER;
            return 1;
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            ReplayData *uh2 = this->currentReplay;
            free(uh2);
            this->currentReplay = NULL;
            this->gameState = STATE_REPLAY_ANIM;
            this->stateTimer = 0;
            for (cur = 0; cur < ARRAY_SIZE_SIGNED(this->vm); cur++)
            {
                this->vm[cur].pendingInterrupt = 4;
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
            this->gameState = STATE_REPLAY_ANIM;
            anmVm = this->vm;
            for (cur = 0; cur < ARRAY_SIZE_SIGNED(this->vm); cur += 1, anmVm++)
            {
                anmVm->pendingInterrupt = 0xf;
            }
            this->cursor = this->chosenReplay;
        }
    }
    return 0;
}
#pragma intrinsic(strcpy)

#pragma var_order(vmRef, i, replayAmount, isSelected, isSelected2)
ZunResult MainMenu::DrawReplayMenu()
{
    i32 replayAmount;
    i32 i;
    AnmVm *vmRef;
    ZunBool isSelected;
    ZunBool isSelected2;

    vmRef = &this->vm[98];
    g_AsciiManager.AddFormatText(&vmRef->pos, "No.   Name      Date     Player   Rank");

    for (i = this->chosenReplay - this->chosenReplay % 15, replayAmount = i; i < replayAmount + 15; i++)
    {
        if (i >= this->replayFilesNum)
        {
            break;
        }
        vmRef++;
        if (!g_Supervisor.cfg.IsSoftwareTexturing())
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
            isSelected = (i == this->chosenReplay);
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
            if (!g_Supervisor.cfg.IsSoftwareTexturing())
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
                isSelected2 = (i == this->cursor);
                g_AsciiManager.isSelected = isSelected2;
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
        if (!g_Supervisor.cfg.IsSoftwareTexturing())
        {
            vm->color = COLOR_MENU_ITEM_DEFAULT;
        }
        else
        {
            g_AnmManager->SetActiveSprite(vm, vm->baseSpriteIndex);
        }
        vm->scaleX = 1.0;
        vm->scaleY = 1.0;
        vm->posOffset = D3DXVECTOR3(0.0, 0.0, 0.0);
    }
    else
    {
        if (!g_Supervisor.cfg.IsSoftwareTexturing())
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
        vm->posOffset = D3DXVECTOR3(-2.0, -2.0, 0.0);
    }

    if (item != this->cursor)
    {
        if ((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING & 1) == 0)
        {
            vm->color = COLOR_SET_ALPHA2(vm->color, 128);
        }
        else
        {
            vm->color = COLOR_SET_ALPHA2(vm->color, 128);
        }

        vm->posOffset += D3DXVECTOR3(0.0, 0.0, 0.0);
    }
    else
    {
        if ((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING & 1) == 0)
        {
            vm->color = COLOR_SET_ALPHA2(vm->color, 255);
        }
        else
        {
            vm->color = COLOR_SET_ALPHA2(vm->color, 255);
        }

        vm->posOffset += D3DXVECTOR3(-4.0, -4.0, 0.0);
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
        if (WAS_PRESSED_WEIRD(TH_BUTTON_LEFT))
        {
            switch (this->cursor)
            {
            case CURSOR_OPTIONS_POS_LIFECOUNT:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                if (g_Supervisor.cfg.lifeCount <= 0)
                {
                    g_Supervisor.cfg.lifeCount = 5;
                }
                g_Supervisor.cfg.lifeCount -= 1;
                break;

            case CURSOR_OPTIONS_POS_BOMBCOUNT:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                if (g_Supervisor.cfg.bombCount <= 0)
                {
                    g_Supervisor.cfg.bombCount = 4;
                }
                g_Supervisor.cfg.bombCount -= 1;
                break;

            case CURSOR_OPTIONS_POS_COLORMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                if (g_Supervisor.cfg.colorMode16bit <= 0)
                {
                    g_Supervisor.cfg.colorMode16bit = 2;
                }
                g_Supervisor.cfg.colorMode16bit -= 1;
                break;

            case CURSOR_OPTIONS_POS_MUSICMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
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

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                if (g_Supervisor.cfg.playSounds <= 0)
                {
                    g_Supervisor.cfg.playSounds = 2;
                }
                g_Supervisor.cfg.playSounds -= 1;
                break;

            case CURSOR_OPTIONS_POS_SCREENMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
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
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
        }
        if (WAS_PRESSED_WEIRD(TH_BUTTON_RIGHT))
        {
            switch (this->cursor)
            {
            case CURSOR_OPTIONS_POS_LIFECOUNT:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                g_Supervisor.cfg.lifeCount += 1;
                if (g_Supervisor.cfg.lifeCount >= 5)
                {
                    g_Supervisor.cfg.lifeCount = 0;
                }
                break;
            case CURSOR_OPTIONS_POS_BOMBCOUNT:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                g_Supervisor.cfg.bombCount += 1;
                if (g_Supervisor.cfg.bombCount >= 4)
                {
                    g_Supervisor.cfg.bombCount = 0;
                }
                break;
            case CURSOR_OPTIONS_POS_COLORMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                g_Supervisor.cfg.colorMode16bit += 1;
                if (g_Supervisor.cfg.colorMode16bit >= 2)
                {
                    g_Supervisor.cfg.colorMode16bit = 0;
                }
                break;
            case CURSOR_OPTIONS_POS_MUSICMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
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

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
                g_Supervisor.cfg.playSounds += 1;
                if (g_Supervisor.cfg.playSounds >= 2)
                {
                    g_Supervisor.cfg.playSounds = 0;
                }
                break;
            case CURSOR_OPTIONS_POS_SCREENMODE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU, 0);
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
                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT, 0);

                memcpy(this->controlMapping, &g_ControllerMapping, sizeof(ControllerMapping));

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
                g_SoundPlayer.PlaySoundByIdx(SOUND_BACK, 0);
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
        D3DXVECTOR3 textPos(320.0, 200.0, 0.0);
        u32 color = (this->stateTimer < 30) ? this->stateTimer * 0xFF / 30 : 0xff;
        i32 charShotType = (g_GameManager.character << 1) + g_GameManager.shotType;
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
                g_AsciiManager.color = color << 0x18 | 0x00C0F0F0;
            }
            else
            {
                g_AsciiManager.color = (color >> 1) << 0x18 | 0x0080C0C0;
            }
            g_AsciiManager.AddFormatText(&textPos, "STAGE %d  %.9d", stageNum + 1,
                                         g_GameManager.pscr[charShotType][stageNum][g_GameManager.difficulty].score);
            textPos.y += 24;
        }
        g_AsciiManager.color = 0xFFFFFFFF;
    }
    return ZUN_SUCCESS;
}

#pragma var_order(targetOpacity, window, vmIdx, curVm, posBackup, mgr, shouldDraw, offset, pos)
ChainCallbackResult MainMenu::OnDraw(MainMenu *menu)
{
    D3DXVECTOR3 posBackup;
    D3DXVECTOR3 *pos;
    D3DXVECTOR3 *offset;
    BOOL shouldDraw;
    AnmVm *curVm;
    i32 vmIdx;
    ZunRect window;
    i32 targetOpacity;
    AnmManager *mgr;

    curVm = menu->vm;
    window.left = 0.0;
    window.top = 0.0;
    window.right = 640.0;
    window.bottom = 480.0;
    if (menu->gameState == STATE_STARTUP)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    mgr = g_AnmManager;
    mgr->currentTexture = NULL;
    g_AnmManager->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    if (menu->framesActive != 0)
    {
        // This is confusing. framesActive/framesInactive appear to be unsigned,
        // due to how they get loaded. But this comparison is signed somehow.
        // Why?
        if (menu->numFramesSinceActive < (i32)menu->framesActive)
        {
            menu->numFramesSinceActive += 1;
        }
        targetOpacity = COLOR_ALPHA(menu->menuTextColor) - COLOR_ALPHA(menu->minimumOpacity);
        ScreenEffect::DrawSquare(
            &window,
            COLOR_SET_ALPHA(menu->menuTextColor, targetOpacity * menu->numFramesSinceActive / menu->framesActive +
                                                     COLOR_ALPHA(menu->minimumOpacity)));
    }
    else if (menu->numFramesSinceActive != 0)
    {
        menu->numFramesSinceActive -= 1;
        targetOpacity = COLOR_ALPHA(menu->menuTextColor) - COLOR_ALPHA(menu->minimumOpacity);
        ScreenEffect::DrawSquare(
            &window,
            COLOR_SET_ALPHA(menu->menuTextColor, targetOpacity * menu->numFramesSinceActive / menu->framesInactive +
                                                     COLOR_ALPHA(menu->minimumOpacity)));
    }
    for (vmIdx = 0; vmIdx < 98; vmIdx++, curVm++)
    {
        if (curVm->sprite == NULL)
        {
            shouldDraw = false;
        }
        else if (curVm->sprite->sourceFileIndex < 0)
        {
            shouldDraw = false;
        }
        else
        {
            shouldDraw = g_AnmManager->textures[curVm->sprite->sourceFileIndex] != NULL;
        }
        if (shouldDraw)
        {
            memcpy(posBackup, curVm->pos, sizeof(D3DXVECTOR3));
            offset = &curVm->posOffset;
            pos = &curVm->pos;
            pos->x += offset->x;
            pos->y += offset->y;
            pos->z += offset->z;
            g_AnmManager->Draw(curVm);
            memcpy(curVm->pos, posBackup, sizeof(D3DXVECTOR3));
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

    g_Supervisor.LoadPbg3(3, TH_TL_DAT_FILE);
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
        menu->vm[i].flags.isVisible = 0;
        menu->vm[i].baseSpriteIndex = menu->vm[i].activeSpriteIndex;
        menu->vm[i].flags.zWriteDisable = 1;
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
    for (vm = &menu->vm[0x50], i = ANM_SCRIPT_SELECT01_START; i <= ANM_SCRIPT_SELECT01_END; i++, vm++)
    {
        g_AnmManager->ExecuteAnmIdx(vm, i);
        vm->flags.isVisible = 0;
        vm->flags.colorOp = AnmVmColorOp_Add;
        if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
        {
            vm->color = COLOR_BLACK;
        }
        else
        {
            vm->color = COLOR_WHITE;
        }
        vm->posOffset = D3DXVECTOR3(0, 0, 0);
        vm->baseSpriteIndex = vm->activeSpriteIndex;
        vm->flags.zWriteDisable = 1;
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
        vm->flags.isVisible = 0;
        vm->flags.colorOp = AnmVmColorOp_Add;

        if ((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING & 1) == 0)
        {
            vm->color = COLOR_BLACK;
        }
        else
        {
            vm->color = COLOR_WHITE;
        }
        vm->posOffset = D3DXVECTOR3(0, 0, 0);
        vm->baseSpriteIndex = vm->activeSpriteIndex;
        vm->flags.zWriteDisable = 1;
    }
    return ZUN_SUCCESS;
}

#pragma function(memset)
ZunResult MainMenu::RegisterChain(u32 isDemo)
{
    MainMenu *menu = &g_MainMenu;

    memset(menu, 0, sizeof(MainMenu));
    g_GameManager.isInGameMenu = 0;
    utils::DebugPrint(TH_DBG_MAINMENU_VRAM, g_Supervisor.d3dDevice->GetAvailableTextureMem());
    menu->gameState = isDemo ? STATE_REPLAY_LOAD : STATE_STARTUP;
    g_Supervisor.framerateMultiplier = 0.0;
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
#pragma intrinsic(memset)

#pragma var_order(scoredat, i, anmmgr)
ZunResult MainMenu::AddedCallback(MainMenu *m)
{
    i32 i;
    ScoreDat *scoredat;
    AnmManager *anmmgr;

    if (g_GameManager.demoMode == 0)
    {
        g_Supervisor.SetupMidiPlayback("bgm/th06_01.mid");
    }

    anmmgr = g_AnmManager;

    for (i = 0; i < ANM_OFFSET_TITLE01S - ANM_OFFSET_TITLE01; i++)
    {
        anmmgr->scripts[i + ANM_OFFSET_TITLE01] = NULL;
    }
    m->unk_81e4 = 0;

    switch (g_Supervisor.wantedState2)
    {
    case SUPERVISOR_STATE_GAMEMANAGER:
    case SUPERVISOR_STATE_GAMEMANAGER_REINIT:
    case SUPERVISOR_STATE_RESULTSCREEN_FROMGAME:
        m->cursor = g_GameManager.difficulty == EXTRA;
        break;
    case SUPERVISOR_STATE_RESULTSCREEN:
        m->cursor = 4;
        break;
    case SUPERVISOR_STATE_MUSICROOM:
        m->cursor = 5;
        break;
    case SUPERVISOR_STATE_INIT:
    case SUPERVISOR_STATE_MAINMENU:
    default:
        m->cursor = 0;
    }

    if (g_GameManager.isInPracticeMode != 0)
    {
        m->cursor = 2;
    }

    g_GameManager.isInPracticeMode = 0;
    if ((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING & 1) == 0)
    {
        m->color1 = 0x80004000;
        m->color2 = 0xff008000;
    }
    else
    {
        m->color1 = 0x80ffffff;
        m->color2 = 0xffffffff;
    }
    m->minimumOpacity = 0;
    m->menuTextColor = 0x40000000;
    m->numFramesSinceActive = 0;
    m->framesActive = 0;
    m->unk_10f28 = 0x10;
    m->currentReplay = NULL;
    scoredat = ResultScreen::OpenScore("score.dat");
    ResultScreen::ParseClrd(scoredat, g_GameManager.clrd);
    ResultScreen::ParsePscr(scoredat, (Pscr *)g_GameManager.pscr);
    ResultScreen::ReleaseScoreDat(scoredat);
    if (g_GameManager.demoMode == 0)
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
    g_GameManager.demoMode = 0;
    g_GameManager.demoFrames = 0;
    return ZUN_SUCCESS;
}

#pragma var_order(i1, i2, mgr, replay)
ZunResult MainMenu::DeletedCallback(MainMenu *menu)
{
    AnmManager *mgr;
    void *replay;
    i32 i1, i2;

    g_Supervisor.d3dDevice->ResourceManagerDiscardBytes(0);
    MainMenu::ReleaseTitleAnm();
    for (i1 = ANM_FILE_SELECT01; i1 <= ANM_FILE_REPLAY; i1++)
    {
        g_AnmManager->ReleaseAnm(i1);
    }
    g_AnmManager->ReleaseSurface(0);

    // TODO: Inline function, but when inlining it, I lose control over the
    // stack slots, and it stops matching.
    mgr = g_AnmManager;
    for (i2 = 0; i2 < ANM_OFFSET_TITLE01S - ANM_OFFSET_TITLE01; i2++)
    {
        mgr->scripts[ANM_OFFSET_TITLE01 + i2] = NULL;
    }
    g_Chain.Cut(menu->chainDraw);
    menu->chainDraw = NULL;

    replay = menu->currentReplay;
    free(replay);
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

DIFFABLE_STATIC(MainMenu, g_MainMenu);
}; // namespace th06

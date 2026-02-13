#include "AsciiManager.hpp"
#include "StageMenu.hpp"

#include "AnmManager.hpp"
#include "ChainPriorities.hpp"
#include "Controller.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "Supervisor.hpp"
#include "utils.hpp"
#include <stdio.h>

namespace th06
{
DIFFABLE_STATIC(AsciiManager, g_AsciiManager)
DIFFABLE_STATIC(ChainElem, g_AsciiManagerCalcChain)
DIFFABLE_STATIC(ChainElem, g_AsciiManagerOnDrawMenusChain)
DIFFABLE_STATIC(ChainElem, g_AsciiManagerOnDrawPopupsChain)

AsciiManager::AsciiManager()
{
}

StageMenu::StageMenu()
{
}

ChainCallbackResult AsciiManager::OnUpdate(AsciiManager *mgr)
{
    if (!g_GameManager.isInGameMenu && !g_GameManager.isInRetryMenu)
    {
        AsciiManagerPopup *curPopup = &mgr->popups[0];
        i32 i = 0;
        for (; i < ARRAY_SIZE_SIGNED(mgr->popups); i++, curPopup++)
        {
            if (!curPopup->inUse)
            {
                continue;
            }

            curPopup->position.y -= 0.5f * g_Supervisor.effectiveFramerateMultiplier;
            curPopup->timer.Tick();
            if (curPopup->timer > 60)
            {
                curPopup->inUse = false;
            }
        }
    }
    else if (g_GameManager.isInGameMenu)
    {
        mgr->gameMenu.OnUpdateGameMenu();
    }
    if (g_GameManager.isInRetryMenu)
    {
        mgr->retryMenu.OnUpdateRetryMenu();
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult AsciiManager::OnDrawMenus(AsciiManager *mgr)
{
    mgr->DrawStrings();
    mgr->numStrings = 0;
    mgr->gameMenu.OnDrawGameMenu();
    mgr->retryMenu.OnDrawRetryMenu();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult AsciiManager::OnDrawPopups(AsciiManager *mgr)
{
    if (g_Supervisor.hasD3dHardwareVertexProcessing)
    {
        mgr->DrawPopupsWithHwVertexProcessing();
    }
    else
    {
        mgr->DrawPopupsWithoutHwVertexProcessing();
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ZunResult AsciiManager::RegisterChain()
{
    AsciiManager *mgr = &g_AsciiManager;

    g_AsciiManagerCalcChain.callback = (ChainCallback)AsciiManager::OnUpdate;
    g_AsciiManagerCalcChain.addedCallback = NULL;
    g_AsciiManagerCalcChain.deletedCallback = NULL;
    g_AsciiManagerCalcChain.addedCallback = (ChainAddedCallback)AsciiManager::AddedCallback;
    g_AsciiManagerCalcChain.deletedCallback = (ChainDeletedCallback)AsciiManager::DeletedCallback;
    g_AsciiManagerCalcChain.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_AsciiManagerCalcChain, TH_CHAIN_PRIO_CALC_ASCIIMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    g_AsciiManagerOnDrawMenusChain.callback = (ChainCallback)OnDrawMenus;
    g_AsciiManagerOnDrawMenusChain.addedCallback = NULL;
    g_AsciiManagerOnDrawMenusChain.deletedCallback = NULL;
    g_AsciiManagerOnDrawMenusChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_AsciiManagerOnDrawMenusChain, TH_CHAIN_PRIO_DRAW_ASCIIMANAGER_MENUS);

    g_AsciiManagerOnDrawPopupsChain.callback = (ChainCallback)OnDrawPopups;
    g_AsciiManagerOnDrawPopupsChain.addedCallback = NULL;
    g_AsciiManagerOnDrawPopupsChain.deletedCallback = NULL;
    g_AsciiManagerOnDrawPopupsChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_AsciiManagerOnDrawPopupsChain, TH_CHAIN_PRIO_DRAW_ASCIIMANAGER_POPUPS);

    return ZUN_SUCCESS;
}

ZunResult AsciiManager::AddedCallback(AsciiManager *s)
{
    int x, y, z;

    if (g_AnmManager->LoadAnm(ANM_FILE_ASCII, "data/ascii.anm", ANM_OFFSET_ASCII) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_ASCIIS, "data/asciis.anm", ANM_OFFSET_ASCIIS) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    if (g_AnmManager->LoadAnm(ANM_FILE_CAPTURE, "data/capture.anm", ANM_OFFSET_CAPTURE) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    s->InitializeVms();
    return ZUN_SUCCESS;
}

#pragma var_order(vm1, mgr1, mgr0)
void AsciiManager::InitializeVms()
{
    memset(this, 0, sizeof(AsciiManager));

    this->color = 0xffffffff;
    this->scale.x = 1.0;
    this->scale.y = 1.0;

    this->vm1.flags.anchor = AnmVmAnchor_TopLeft;
    AnmVm *vm1 = &this->vm1;
    AnmManager *mgr1 = g_AnmManager;
    vm1->Initialize();
    mgr1->SetActiveSprite(vm1, 0);

    AnmManager *mgr0 = g_AnmManager;
    this->vm0.Initialize();
    mgr0->SetActiveSprite(&this->vm0, 0x20);

    this->vm1.pos.z = 0.1;
    this->isSelected = 0;
}

ZunResult AsciiManager::DeletedCallback(AsciiManager *s)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_ASCII);
    g_AnmManager->ReleaseAnm(ANM_FILE_ASCIIS);
    g_AnmManager->ReleaseAnm(ANM_FILE_CAPTURE);
    return ZUN_SUCCESS;
}

void AsciiManager::CutChain()
{
    g_Chain.Cut(&g_AsciiManagerCalcChain);
    g_Chain.Cut(&g_AsciiManagerOnDrawMenusChain);
    // What about g_AsciiManagerOnDrawPopupsChain? It looks like zun forgot
    // to free it!
}

void AsciiManager::AddString(D3DXVECTOR3 *position, char *text)
{
    if (this->numStrings >= 0x100)
    {
        return;
    }

    AsciiManagerString *curString = &this->strings[this->numStrings];
    this->numStrings += 1;
    // Hello unguarded strcpy my old friend. If text is bigger than 64
    // characters, kboom.
    strcpy(curString->text, text);
    curString->position = *position;
    curString->color = this->color;
    curString->scale.x = this->scale.x;
    curString->scale.y = this->scale.y;
    curString->isGui = this->isGui;
    if (g_Supervisor.cfg.IsSoftwareTexturing())
    {
        curString->isSelected = this->isSelected;
    }
    else
    {
        curString->isSelected = 0;
    }
}

void AsciiManager::AddFormatText(D3DXVECTOR3 *position, const char *fmt, ...)
{
    char tmpBuffer[512];
    va_list args;

    va_start(args, fmt);
    vsprintf(tmpBuffer, fmt, args);
    AddString(position, tmpBuffer);

    va_end(args);
}

#pragma var_order(charWidth, i, string, text, guiString, padding_1, padding_2, padding_3)
void AsciiManager::DrawStrings(void)
{
    i32 padding_1;
    i32 padding_2;
    i32 padding_3;
    i32 i;
    BOOL guiString;
    f32 charWidth;
    AsciiManagerString *string;
    u8 *text;

    guiString = TRUE;
    string = this->strings;
    this->vm0.flags.isVisible = 1;
    this->vm0.flags.anchor = AnmVmAnchor_TopLeft;
    for (i = 0; i < this->numStrings; i++, string++)
    {
        this->vm0.pos = string->position;
        text = (u8 *)string->text;
        this->vm0.scaleX = string->scale.x;
        this->vm0.scaleY = string->scale.y;
        charWidth = 14 * string->scale.x;
        if (guiString != string->isGui)
        {
            guiString = string->isGui;
            if (guiString)
            {
                g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
                g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;
                g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
                g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;
                g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
            }
            else
            {
                g_Supervisor.viewport.X = 0;
                g_Supervisor.viewport.Y = 0;
                g_Supervisor.viewport.Width = 640;
                g_Supervisor.viewport.Height = 480;
                g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
            }
        }
        while (*text != NULL)
        {
            if (*text == '\n')
            {
                this->vm0.pos.y = 16 * string->scale.y + this->vm0.pos.y;
                this->vm0.pos.x = string->position.x;
            }
            else if (*text == ' ')
            {
                this->vm0.pos.x += charWidth;
            }
            else
            {
                if (string->isSelected == FALSE)
                {
                    this->vm0.sprite = &g_AnmManager->sprites[*text - 0x15];
                    this->vm0.color = string->color;
                }
                else
                {
                    this->vm0.sprite = &g_AnmManager->sprites[*text + 0x61];
                    this->vm0.color = 0xFFFFFFFF;
                }
                g_AnmManager->DrawNoRotation(&this->vm0);
                this->vm0.pos.x += charWidth;
            }
            text++;
        }
    }
}

void AsciiManager::CreatePopup1(D3DXVECTOR3 *position, i32 value, D3DCOLOR color)
{
    AsciiManagerPopup *popup;
    i32 characterCount;

    if (this->nextPopupIndex1 >= (ARRAY_SIZE_SIGNED(this->popups) - 3))
    {
        this->nextPopupIndex1 = 0;
    }

    popup = &this->popups[this->nextPopupIndex1];
    popup->inUse = 1;
    characterCount = 0;

    if (value >= 0)
    {
        while (value)
        {
            popup->digits[characterCount++] = (char)(value % 10);

            value /= 10;
        }
    }
    else
    {
        popup->digits[characterCount++] = '\n';
    }

    if (characterCount == 0)
    {
        popup->digits[characterCount++] = '\0';
    }

    popup->characterCount = characterCount;
    popup->color = color;
    popup->timer.InitializeForPopup();
    popup->position = *position;

    this->nextPopupIndex1++;
}

void AsciiManager::CreatePopup2(D3DXVECTOR3 *position, i32 value, D3DCOLOR color)
{
    AsciiManagerPopup *popup;
    i32 characterCount;

    if (this->nextPopupIndex2 >= 3)
    {
        this->nextPopupIndex2 = 0;
    }

    popup = &this->popups[0x200 + this->nextPopupIndex2];
    popup->inUse = 1;
    characterCount = 0;

    if (value >= 0)
    {
        while (value)
        {
            popup->digits[characterCount++] = (char)(value % 10);

            value /= 10;
        }
    }
    else
    {
        popup->digits[characterCount++] = '\n';
    }

    if (characterCount == 0)
    {
        popup->digits[characterCount++] = '\0';
    }

    popup->characterCount = characterCount;
    popup->color = color;
    popup->timer.InitializeForPopup();
    popup->position = *position;

    this->nextPopupIndex2++;
}

enum UpdateGameMenuState
{
    GAME_MENU_PAUSE_OPENING,
    GAME_MENU_PAUSE_CURSOR_UNPAUSE,
    GAME_MENU_PAUSE_CURSOR_QUIT,
    GAME_MENU_PAUSE_SELECTED_UNPAUSE,
    GAME_MENU_QUIT_CURSOR_YES,
    GAME_MENU_QUIT_CURSOR_NO,
    GAME_MENU_QUIT_SELECTED_YES,
};

#define GAME_MENU_SPRITE_TITLE_PAUSE 0
#define GAME_MENU_SPRITE_CURSOR_UNPAUSE 1
#define GAME_MENU_SPRITE_CURSOR_QUIT 2
#define GAME_MENU_SPRITE_TITLE_QUIT 3
#define GAME_MENU_SPRITE_CURSOR_YES 4
#define GAME_MENU_SPRITE_CURSOR_NO 5

#define GAME_MENU_SPRITES_START_PAUSE GAME_MENU_SPRITE_TITLE_PAUSE
#define GAME_MENU_SPRITES_COUNT_PAUSE 3
#define GAME_MENU_SPRITES_END_PAUSE (GAME_MENU_SPRITES_START_PAUSE + GAME_MENU_SPRITES_COUNT_PAUSE)
#define GAME_MENU_SPRITES_START_QUIT GAME_MENU_SPRITE_TITLE_QUIT
#define GAME_MENU_SPRITES_COUNT_QUIT 3
#define GAME_MENU_SPRITES_END_QUIT (GAME_MENU_SPRITES_START_QUIT + GAME_MENU_SPRITES_COUNT_QUIT)

i32 StageMenu::OnUpdateGameMenu()
{
    i32 vmIdx;

    if (WAS_PRESSED(TH_BUTTON_MENU))
    {
        this->curState = GAME_MENU_PAUSE_SELECTED_UNPAUSE;
        for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
        {
            if (this->menuSprites[vmIdx].flags.isVisible)
            {
                this->menuSprites[vmIdx].pendingInterrupt = 2;
            }
        }
        this->numFrames = 0;
        this->menuBackground.pendingInterrupt = 1;
    }
    if (WAS_PRESSED(TH_BUTTON_BOMB2))
    {
        this->curState = GAME_MENU_QUIT_SELECTED_YES;
        for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
        {
            if (this->menuSprites[vmIdx].flags.isVisible)
            {
                this->menuSprites[vmIdx].pendingInterrupt = 2;
            }
        }
        this->numFrames = 0;
    }
    switch (this->curState)
    {
    case GAME_MENU_PAUSE_OPENING:
        for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
        {
            g_AnmManager->SetAndExecuteScriptIdx(&this->menuSprites[vmIdx], vmIdx + 2);
        }
        for (vmIdx = GAME_MENU_SPRITES_START_PAUSE; vmIdx < GAME_MENU_SPRITES_END_PAUSE; vmIdx++)
        {
            this->menuSprites[vmIdx].pendingInterrupt = 1;
        }
        this->curState++;
        this->numFrames = 0;
        if (g_Supervisor.lockableBackbuffer)
        {
            g_AnmManager->RequestScreenshot();
            g_AnmManager->SetAndExecuteScriptIdx(&this->menuBackground, ANM_SCRIPT_CAPTURE_PAUSE_BG);
            this->menuBackground.pos.x = GAME_REGION_LEFT;
            this->menuBackground.pos.y = GAME_REGION_TOP;
            this->menuBackground.pos.z = 0.0f;
        }
    case GAME_MENU_PAUSE_CURSOR_UNPAUSE:
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_UNPAUSE].color = COLOR_LIGHT_RED;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_QUIT].color = COLOR_SET_ALPHA(COLOR_GREY, 0x80);
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_UNPAUSE].scaleY = 1.7f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_UNPAUSE].scaleX = 1.7f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_QUIT].scaleY = 1.5f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_QUIT].scaleX = 1.5f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_UNPAUSE].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_QUIT].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (4 <= this->numFrames)
        {
            if (WAS_PRESSED(TH_BUTTON_UP) || WAS_PRESSED(TH_BUTTON_DOWN))
            {
                this->curState = GAME_MENU_PAUSE_CURSOR_QUIT;
            }
            if (WAS_PRESSED(TH_BUTTON_SHOOT))
            {
                for (vmIdx = GAME_MENU_SPRITES_START_PAUSE; vmIdx < GAME_MENU_SPRITES_END_PAUSE; vmIdx++)
                {
                    this->menuSprites[vmIdx].pendingInterrupt = 2;
                }
                this->curState = GAME_MENU_PAUSE_SELECTED_UNPAUSE;
                this->numFrames = 0;
                this->menuBackground.pendingInterrupt = 1;
            }
        }
        break;
    case GAME_MENU_PAUSE_CURSOR_QUIT:
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_UNPAUSE].color = COLOR_SET_ALPHA(COLOR_GREY, 0x80);
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_QUIT].color = COLOR_LIGHT_RED;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_UNPAUSE].scaleY = 1.5f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_UNPAUSE].scaleX = 1.5f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_QUIT].scaleY = 1.7f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_QUIT].scaleX = 1.7f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_UNPAUSE].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_QUIT].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (4 <= this->numFrames)
        {
            if (WAS_PRESSED(TH_BUTTON_UP) || WAS_PRESSED(TH_BUTTON_DOWN))
            {
                this->curState = GAME_MENU_PAUSE_CURSOR_UNPAUSE;
            }
            if (WAS_PRESSED(TH_BUTTON_SHOOT))
            {
                for (vmIdx = GAME_MENU_SPRITES_START_PAUSE; vmIdx < GAME_MENU_SPRITES_END_PAUSE; vmIdx++)
                {
                    this->menuSprites[vmIdx].pendingInterrupt = 2;
                }
                for (vmIdx = GAME_MENU_SPRITES_START_QUIT; vmIdx < GAME_MENU_SPRITES_END_QUIT; vmIdx++)
                {
                    this->menuSprites[vmIdx].pendingInterrupt = 1;
                }
                this->curState = GAME_MENU_QUIT_CURSOR_NO;
                this->numFrames = 0;
            }
        }
        break;
    case GAME_MENU_PAUSE_SELECTED_UNPAUSE:
        /* Close menu, wait 20 frames for the animation? */
        if (20 <= this->numFrames)
        {
            this->curState = GAME_MENU_PAUSE_OPENING;
            g_GameManager.isInGameMenu = 0;
            for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
            {
                this->menuSprites[vmIdx].SetInvisible();
            }
        }
        break;
    case GAME_MENU_QUIT_CURSOR_YES:
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_YES].color = COLOR_LIGHT_RED;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_NO].color = COLOR_SET_ALPHA(COLOR_GREY, 0x80);
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_YES].scaleY = 1.7f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_YES].scaleX = 1.7f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_NO].scaleY = 1.5f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_NO].scaleX = 1.5f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_YES].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_NO].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (4 <= this->numFrames)
        {
            if (WAS_PRESSED(TH_BUTTON_UP) || WAS_PRESSED(TH_BUTTON_DOWN))
            {
                this->curState = GAME_MENU_QUIT_CURSOR_NO;
            }
            if (WAS_PRESSED(TH_BUTTON_SHOOT))
            {
                for (vmIdx = GAME_MENU_SPRITES_START_QUIT; vmIdx < GAME_MENU_SPRITES_END_QUIT; vmIdx++)
                {
                    this->menuSprites[vmIdx].pendingInterrupt = 2;
                }
                this->curState = GAME_MENU_QUIT_SELECTED_YES;
                this->numFrames = 0;
            }
        }
        break;
    case GAME_MENU_QUIT_CURSOR_NO:
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_YES].color = COLOR_SET_ALPHA(COLOR_GREY, 0x80);
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_NO].color = COLOR_LIGHT_RED;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_YES].scaleY = 1.5f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_YES].scaleX = 1.5f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_NO].scaleY = 1.7f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_NO].scaleX = 1.7f;
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_YES].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        this->menuSprites[GAME_MENU_SPRITE_CURSOR_NO].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        if (GAME_MENU_SPRITE_CURSOR_YES <= this->numFrames)
        {
            if (WAS_PRESSED(TH_BUTTON_UP) || WAS_PRESSED(TH_BUTTON_DOWN))
            {
                this->curState = GAME_MENU_QUIT_CURSOR_YES;
            }
            if (WAS_PRESSED(TH_BUTTON_SHOOT))
            {
                for (vmIdx = GAME_MENU_SPRITES_START_PAUSE; vmIdx < GAME_MENU_SPRITES_END_PAUSE; vmIdx++)
                {
                    this->menuSprites[vmIdx].pendingInterrupt = 1;
                }
                for (vmIdx = GAME_MENU_SPRITES_START_QUIT; vmIdx < GAME_MENU_SPRITES_END_QUIT; vmIdx++)
                {
                    this->menuSprites[vmIdx].pendingInterrupt = 2;
                }
                this->curState = GAME_MENU_PAUSE_CURSOR_QUIT;
                this->numFrames = 0;
            }
        }
        break;
    case GAME_MENU_QUIT_SELECTED_YES:
        if (20 <= this->numFrames)
        {
            this->curState = GAME_MENU_PAUSE_OPENING;
            g_GameManager.isInGameMenu = 0;
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
            for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
            {
                this->menuSprites[vmIdx].SetInvisible();
            }
        }
    }
    for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
    {
        g_AnmManager->ExecuteScript(&this->menuSprites[vmIdx]);
    }
    if (g_Supervisor.lockableBackbuffer)
    {
        g_AnmManager->ExecuteScript(&this->menuBackground);
    }
    this->numFrames++;
    return 0;
}

void StageMenu::OnDrawGameMenu()
{
    i32 vmIdx;

    if (g_GameManager.isInGameMenu)
    {
        g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
        g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;
        g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
        g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;
        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
        if (g_Supervisor.lockableBackbuffer && this->curState != GAME_MENU_PAUSE_OPENING)
        {
            AnmVm menuBackground = this->menuBackground;
            menuBackground.flags.zWriteDisable = 1;
            g_AnmManager->DrawNoRotation(&menuBackground);
        }
        for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
        {
            if (this->menuSprites[vmIdx].flags.isVisible)
            {
                g_AnmManager->DrawNoRotation(&this->menuSprites[vmIdx]);
            }
        }
    }
    return;
}

enum RetryGameMenuState
{
    RETRY_MENU_OPENING,
    RETRY_MENU_CURSOR_YES,
    RETRY_MENU_CURSOR_NO,
    RETRY_MENU_SELECTED_YES,
    RETRY_MENU_SELECTED_NO,
};

#define RETRY_MENU_SPRITE_TITLE 0
#define RETRY_MENU_SPRITE_RETRIES_LABEL 1
#define RETRY_MENU_SPRITE_YES 2
#define RETRY_MENU_SPRITE_NO 3
#define RETRY_MENU_SPRITE_RETRIES_NUMBER 4

#define RETRY_MENU_SPRITES_START RETRY_MENU_SPRITE_TITLE
#define RETRY_MENU_SPRITES_COUNT 4
#define RETRY_MENU_SPRITES_END (RETRY_MENU_SPRITES_START + RETRY_MENU_SPRITES_COUNT)

i32 StageMenu::OnUpdateRetryMenu()
{
    i32 idx;

    if (g_GameManager.isInPracticeMode)
    {
        g_GameManager.isInRetryMenu = 0;
        g_GameManager.guiScore = g_GameManager.score;
        g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
        return 1;
    }
    if (g_GameManager.isInReplay)
    {
        g_GameManager.isInRetryMenu = 0;
        g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU_REPLAY;
        g_GameManager.guiScore = g_GameManager.score;
        return 1;
    }
    if (g_GameManager.numRetries >= 3 || g_GameManager.difficulty >= EXTRA)
    {
        g_GameManager.isInRetryMenu = 0;
        g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
        g_GameManager.guiScore = g_GameManager.score;
        return 1;
    }
    switch (this->curState)
    {
    case RETRY_MENU_OPENING:
        if (this->numFrames == 0)
        {
            for (idx = RETRY_MENU_SPRITES_START; idx < RETRY_MENU_SPRITES_END; idx++)
            {
                if (idx < 2)
                {
                    g_AnmManager->SetAndExecuteScriptIdx(&this->menuSprites[idx], idx + 8);
                }
                else
                {
                    g_AnmManager->SetAndExecuteScriptIdx(&this->menuSprites[idx], idx + 4);
                }
                this->menuSprites[idx].pendingInterrupt = 1;
            }
            if (g_Supervisor.lockableBackbuffer)
            {
                g_AnmManager->RequestScreenshot();
                g_AnmManager->SetAndExecuteScriptIdx(&this->menuBackground, ANM_SCRIPT_CAPTURE_PAUSE_BG);
                this->menuBackground.pos.x = GAME_REGION_LEFT;
                this->menuBackground.pos.y = GAME_REGION_TOP;
                this->menuBackground.pos.z = 0.0f;
            }
        }
        if (this->numFrames > 8)
            break;
        this->curState += RETRY_MENU_CURSOR_NO;
        this->numFrames = 0;
    case RETRY_MENU_CURSOR_YES:
        this->menuSprites[RETRY_MENU_SPRITE_YES].color = COLOR_LIGHT_RED;
        this->menuSprites[RETRY_MENU_SPRITE_NO].color = COLOR_SET_ALPHA(COLOR_GREY, 0x80);
        this->menuSprites[RETRY_MENU_SPRITE_YES].scaleY = 1.7f;
        this->menuSprites[RETRY_MENU_SPRITE_YES].scaleX = 1.7f;
        this->menuSprites[RETRY_MENU_SPRITE_NO].scaleY = 1.5f;
        this->menuSprites[RETRY_MENU_SPRITE_NO].scaleX = 1.5f;
        this->menuSprites[RETRY_MENU_SPRITE_YES].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->menuSprites[RETRY_MENU_SPRITE_NO].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (4 <= this->numFrames)
        {
            if (WAS_PRESSED(TH_BUTTON_UP) || WAS_PRESSED(TH_BUTTON_DOWN))
            {
                this->curState = RETRY_MENU_CURSOR_NO;
            }
            if (WAS_PRESSED(TH_BUTTON_SHOOT))
            {
                for (idx = RETRY_MENU_SPRITES_START; idx < RETRY_MENU_SPRITES_END; idx++)
                {
                    this->menuSprites[idx].pendingInterrupt = 2;
                }
                this->curState = RETRY_MENU_SELECTED_YES;
                this->menuBackground.pendingInterrupt = 1;
                this->numFrames = 0;
            }
        }
        break;
    case RETRY_MENU_CURSOR_NO:
        this->menuSprites[RETRY_MENU_SPRITE_NO].color = COLOR_LIGHT_RED;
        this->menuSprites[RETRY_MENU_SPRITE_YES].color = COLOR_SET_ALPHA(COLOR_GREY, 0x80);
        this->menuSprites[RETRY_MENU_SPRITE_YES].scaleY = 1.5f;
        this->menuSprites[RETRY_MENU_SPRITE_YES].scaleX = 1.5f;
        this->menuSprites[RETRY_MENU_SPRITE_NO].scaleY = 1.7f;
        this->menuSprites[RETRY_MENU_SPRITE_NO].scaleX = 1.7f;
        this->menuSprites[RETRY_MENU_SPRITE_NO].posOffset = D3DXVECTOR3(-4.0f, -4.0f, 0.0f);
        this->menuSprites[RETRY_MENU_SPRITE_YES].posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (this->numFrames >= 30)
        {
            if (WAS_PRESSED(TH_BUTTON_UP) || WAS_PRESSED(TH_BUTTON_DOWN))
            {
                this->curState = RETRY_MENU_CURSOR_YES;
            }
            if (WAS_PRESSED(TH_BUTTON_SHOOT))
            {
                for (idx = RETRY_MENU_SPRITES_START; idx < RETRY_MENU_SPRITES_END; idx++)
                {
                    this->menuSprites[idx].pendingInterrupt = 2;
                }
                this->curState = RETRY_MENU_SELECTED_NO;
                this->numFrames = 0;
            }
        }
        break;
    case RETRY_MENU_SELECTED_NO:
        if (this->numFrames >= 20)
        {
            this->curState = 0;
            this->numFrames = 0;
            g_GameManager.isInRetryMenu = 0;
            g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
            for (idx = RETRY_MENU_SPRITES_START; idx < RETRY_MENU_SPRITES_END; idx++)
            {
                this->menuSprites[idx].SetInvisible();
            }
            g_GameManager.guiScore = g_GameManager.score;
            return 0;
        }
        break;
    case RETRY_MENU_SELECTED_YES:
        if (this->numFrames >= 30)
        {
            this->curState = 0;
            this->numFrames = 0;
            g_GameManager.isInRetryMenu = 0;
            for (idx = RETRY_MENU_SPRITES_START; idx < RETRY_MENU_SPRITES_END; idx++)
            {
                this->menuSprites[idx].SetInvisible();
            }
            g_GameManager.numRetries++;
            g_GameManager.guiScore = g_GameManager.numRetries;
            g_GameManager.nextScoreIncrement = 0;
            g_GameManager.score = g_GameManager.guiScore;
            g_GameManager.livesRemaining = g_Supervisor.defaultConfig.lifeCount;
            g_GameManager.bombsRemaining = g_Supervisor.defaultConfig.bombCount;
            g_GameManager.grazeInStage = 0;
            g_GameManager.currentPower = 0;
            g_GameManager.pointItemsCollectedInStage = 0;
            g_GameManager.extraLives = 0;
            g_Gui.flags.flag0 = 2;
            g_Gui.flags.flag1 = 2;
            g_Gui.flags.flag3 = 2;
            g_Gui.flags.flag4 = 2;
            g_Gui.flags.flag2 = 2;
            return 0;
        }
        break;
    }
    for (idx = RETRY_MENU_SPRITES_START; idx < RETRY_MENU_SPRITES_END; idx++)
    {
        g_AnmManager->ExecuteScript(&this->menuSprites[idx]);
    }
    if (g_Supervisor.lockableBackbuffer)
    {
        g_AnmManager->ExecuteScript(&this->menuBackground);
    }
    this->numFrames++;
    return 0;
}

void StageMenu::OnDrawRetryMenu()
{
    int idx;

    if (g_GameManager.isInRetryMenu)
    {
        g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
        g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;
        g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
        g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;
        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
        if (g_Supervisor.lockableBackbuffer && (this->curState != RETRY_MENU_OPENING || this->numFrames > 2))
        {
            g_AnmManager->DrawNoRotation(&this->menuBackground);
        }
        if (this->curState == RETRY_MENU_CURSOR_YES || this->curState == RETRY_MENU_CURSOR_NO)
        {
            this->menuSprites[RETRY_MENU_SPRITE_RETRIES_NUMBER] = this->menuSprites[RETRY_MENU_SPRITE_RETRIES_LABEL];
            this->menuSprites[RETRY_MENU_SPRITE_RETRIES_NUMBER].pos.x +=
                8.0f * this->menuSprites[RETRY_MENU_SPRITE_RETRIES_NUMBER].scaleX;
            this->menuSprites[RETRY_MENU_SPRITE_RETRIES_NUMBER].sprite =
                &g_AnmManager->sprites[30 - g_GameManager.numRetries];
            g_AnmManager->DrawNoRotation(&this->menuSprites[RETRY_MENU_SPRITE_RETRIES_NUMBER]);
        }
        for (idx = RETRY_MENU_SPRITES_START; idx < RETRY_MENU_SPRITES_END; idx++)
        {
            if (this->menuSprites[idx].flags.isVisible)
            {
                g_AnmManager->DrawNoRotation(&this->menuSprites[idx]);
            }
        }
    }
    return;
}

#pragma var_order(currentPopup, j, i, currentDigit, unusedVec3)
void AsciiManager::DrawPopupsWithHwVertexProcessing()
{
    u8 *currentDigit;
    AsciiManagerPopup *currentPopup;
    i32 i;
    i32 j;
    D3DXVECTOR3 unusedVec3;

    currentPopup = this->popups;
    g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
    g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;
    g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
    g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    for (i = 0; i < ARRAY_SIZE_SIGNED(this->popups); i++, currentPopup++)
    {
        if (currentPopup->inUse == 0)
        {
            continue;
        }

        this->vm1.pos.x = currentPopup->position.x - (currentPopup->characterCount * 4);
        this->vm1.pos.y = currentPopup->position.y;
        this->vm1.color = currentPopup->color;

        currentDigit = (u8 *)currentPopup->digits + currentPopup->characterCount - 1;
        for (j = currentPopup->characterCount; 0 < j; j--)
        {
            this->vm1.sprite = g_AnmManager->sprites + *currentDigit;
            if (*currentDigit >= '\n')
            {
                this->vm1.matrix.m[0][0] = 0.1875f;
                this->vm1.matrix.m[1][1] = 0.03125f;
                g_AnmManager->Draw2(&this->vm1);
                this->vm1.matrix.m[0][0] = 0.03125f;
                this->vm1.matrix.m[1][1] = 0.03125f;
            }
            else
            {
                g_AnmManager->Draw2(&this->vm1);
            }

            this->vm1.pos.x += 8.0f;
            currentDigit--;
        }
    }

    return;
}

#pragma var_order(currentPopup, j, i, currentDigit, unusedVec3)
void AsciiManager::DrawPopupsWithoutHwVertexProcessing()
{
    u8 *currentDigit;
    AsciiManagerPopup *currentPopup;
    i32 i;
    i32 j;
    D3DXVECTOR3 unusedVec3;

    currentPopup = this->popups;
    g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
    g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;
    g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
    g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    for (i = 0; i < ARRAY_SIZE_SIGNED(this->popups); i++, currentPopup++)
    {
        if (currentPopup->inUse == 0)
        {
            continue;
        }

        this->vm1.pos.x = currentPopup->position.x - (currentPopup->characterCount * 4);
        this->vm1.pos.y = currentPopup->position.y;
        this->vm1.color = currentPopup->color;

        currentDigit = (u8 *)currentPopup->digits + currentPopup->characterCount - 1;
        for (j = currentPopup->characterCount; 0 < j; j--)
        {
            this->vm1.sprite = g_AnmManager->sprites + *currentDigit;
            if (*currentDigit >= '\n')
            {
                this->vm1.matrix.m[0][0] = 0.1875f;
                this->vm1.matrix.m[1][1] = 0.03125f;
                g_AnmManager->DrawNoRotation(&this->vm1);
                this->vm1.matrix.m[0][0] = 0.03125f;
                this->vm1.matrix.m[1][1] = 0.03125f;
            }
            else
            {
                g_AnmManager->Draw2(&this->vm1);
            }

            this->vm1.pos.x += 8.0f;
            currentDigit--;
        }
    }

    return;
}
}; // namespace th06

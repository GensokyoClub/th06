#include "stdafx.h"

#include "ChainPriorities.hpp"
#include "Gui.hpp"
#include "StageMenu.hpp"

// Keep AsciiManager.hpp outside the common PCH so its inline methods are
// emitted after their first callers.
#include "AsciiManager.hpp"

// Unnecessary include, but forces vector constructor iterator
// to the top of the trial build.
#include "EffectManager.hpp"

namespace th06
{
AsciiManager g_AsciiManager;
ChainElem g_AsciiMgrCalcChain;
ChainElem g_AsciiManagerOnDrawMenusChain;
#if BUILT_SINCE(100)
ChainElem g_AsciiManagerOnDrawPopupsChainElem;
#endif

ChainCallbackResult AsciiManager_OnUpdate(AsciiManager *mgr)
{
    if (!g_GameManager.isInGameMenu && !g_GameManager.isInRetryMenu)
    {
        AsciiManagerPopup *curPopup = &mgr->popups[0];
        i32 i; // NOTE: This doesn't match if i is put in the loop?
        for (i = 0; i < ASCII_TOTAL_POPUPS_COUNT; i++, curPopup++)
        {
            if (!curPopup->inUse)
            {
                continue;
            }

            curPopup->position.y -= 0.5f * g_Supervisor.effectiveFramerateMultiplier;
            curPopup->timer++;
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

ChainCallbackResult AsciiManager_OnDrawMenus(AsciiManager *mgr)
{
#if BUILT_BEFORE(100)
    // 0.08 draws popups and menus from a single chain callback.
    if (g_Supervisor.hasD3dHardwareVertexProcessing)
    {
        mgr->DrawPopupsWithHwVertexProcessing();
    }
    else
    {
        mgr->DrawPopupsWithoutHwVertexProcessing();
    }
#endif
    mgr->DrawStrings();
    mgr->numStrings = 0;
    mgr->gameMenu.OnDrawGameMenu();
    mgr->retryMenu.OnDrawRetryMenu();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#if BUILT_SINCE(100)
ChainCallbackResult AsciiManager_OnDrawPopups(AsciiManager *mgr)
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
#endif

static ZunResult AsciiManager_AddedCallback(AsciiManager *s)
{
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

static ZunResult AsciiManager_DeletedCallback(AsciiManager *s)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_ASCII);
    g_AnmManager->ReleaseAnm(ANM_FILE_ASCIIS);
    g_AnmManager->ReleaseAnm(ANM_FILE_CAPTURE);
    return ZUN_SUCCESS;
}

ZunResult AsciiManager_RegisterChain()
{
    AsciiManager *mgr = &g_AsciiManager;

    g_AsciiMgrCalcChain.SetCallback((ChainCallback)AsciiManager_OnUpdate);
    g_AsciiMgrCalcChain.addedCallback = (ChainAddedCallback)AsciiManager_AddedCallback;
    g_AsciiMgrCalcChain.deletedCallback = (ChainDeletedCallback)AsciiManager_DeletedCallback;
    g_AsciiMgrCalcChain.arg = mgr;
    if (g_FuncChain.AddToCalcChain(&g_AsciiMgrCalcChain, TH_CHAIN_PRIO_CALC_ASCIIMANAGER) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    g_AsciiManagerOnDrawMenusChain.SetCallback((ChainCallback)AsciiManager_OnDrawMenus);
    g_AsciiManagerOnDrawMenusChain.arg = mgr;
    g_FuncChain.AddToDrawChain(&g_AsciiManagerOnDrawMenusChain, TH_CHAIN_PRIO_DRAW_ASCIIMANAGER_MENUS);

#if BUILT_SINCE(100)
    g_AsciiManagerOnDrawPopupsChainElem.SetCallback((ChainCallback)AsciiManager_OnDrawPopups);
    g_AsciiManagerOnDrawPopupsChainElem.arg = mgr;
    g_FuncChain.AddToDrawChain(&g_AsciiManagerOnDrawPopupsChainElem, TH_CHAIN_PRIO_DRAW_ASCIIMANAGER_POPUPS);
#endif

    return ZUN_SUCCESS;
}

void AsciiManager_CutChain()
{
    g_FuncChain.Cut(&g_AsciiMgrCalcChain);
    g_FuncChain.Cut(&g_AsciiManagerOnDrawMenusChain);

    // What about g_AsciiManagerOnDrawPopupsChainElem? It looks like zun forgot
    // to free it!
    // Since 1.00, this misses the separate popup chain added by the draw split.
}

void AsciiManager::AddString(D3DXVECTOR3 *position, const char *text)
{
    if (this->numStrings >= ASCII_STRING_COUNT)
    {
        return;
    }

    AsciiManagerString *curString = &this->strings[this->numStrings];
    this->numStrings++;
    // Hello unguarded strcpy my old friend. If text is bigger than 64
    // characters, kboom.
    strcpy(curString->text, text);
    curString->position = *position;
    curString->color = this->color;
    curString->scale.x = this->scale.x;
    curString->scale.y = this->scale.y;
    curString->isGui = this->isGui;
    if (g_Supervisor.IsSoftwareTexturing())
    {
        curString->isSelected = this->isSelected;
    }
    else
    {
        curString->isSelected = false;
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

#pragma var_order(charWidth, i, string, text, guiString, unusedVec3)
void AsciiManager::DrawStrings(void)
{
    f32 charWidth;
    u8 *text;
    D3DXVECTOR3 unusedVec3; // NOTE: Not padding, IN calls default constructor

    ZunBool guiString = TRUE;
    AsciiManagerString *string = this->strings;
    this->fontVm.flags.isVisible = true;
    this->fontVm.flags.anchor = AnmVmAnchor_TopLeft;

    i32 i;
    for (i = 0; i < this->numStrings; i++, string++)
    {
        this->fontVm.pos = string->position;
        text = (u8 *)string->text;
        this->fontVm.scaleX = string->scale.x;
        this->fontVm.scaleY = string->scale.y;
        charWidth = 14.0f * string->scale.x;
        if (guiString != string->isGui)
        {
            guiString = string->isGui;
            if (guiString)
            {
                g_Supervisor.viewport.X = g_GameManager.gameRegionScreenPos.x;
                g_Supervisor.viewport.Y = g_GameManager.gameRegionScreenPos.y;
                g_Supervisor.viewport.Width = g_GameManager.gameRegionSize.x;
                g_Supervisor.viewport.Height = g_GameManager.gameRegionSize.y;
                g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
            }
            else
            {
                g_Supervisor.viewport.X = 0;
                g_Supervisor.viewport.Y = 0;
                g_Supervisor.viewport.Width = GAME_WINDOW_WIDTH;
                g_Supervisor.viewport.Height = GAME_WINDOW_HEIGHT;
                g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
            }
        }
        while (*text != '\0')
        {
            if (*text == '\n')
            {
                this->fontVm.pos.y = 16 * string->scale.y + this->fontVm.pos.y;
                this->fontVm.pos.x = string->position.x;
            }
            else if (*text == ' ')
            {
                this->fontVm.pos.x += charWidth;
            }
            else
            {
                if (!string->isSelected)
                {
                    this->fontVm.sprite = g_AnmManager->GetSprite(*text + ANM_SPRITE_ASCII_FONT_SPRITE(' '));
                    this->fontVm.color = string->color;
                }
                else
                {
                    this->fontVm.sprite = g_AnmManager->GetSprite(*text + ANM_SPRITE_ASCIIS_FONT_SPRITE(' '));
                    this->fontVm.color = COLOR_WHITE;
                }
                g_AnmManager->DrawNoRotation(&this->fontVm);
                this->fontVm.pos.x += charWidth;
            }
            text++;
        }
    }
}

void AsciiManager::CreatePopup1(D3DXVECTOR3 *position, i32 value, D3DCOLOR color)
{
    if (this->nextPopupIndex1 >= ASCII_SCORE_POPUPS_COUNT)
    {
        this->nextPopupIndex1 = 0;
    }

    AsciiManagerPopup *popup = &this->popups[ASCII_SCORE_POPUPS_START + this->nextPopupIndex1];
    popup->inUse = true;
    i32 characterCount = 0;

#if BUILT_SINCE(100)
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
#else
    while (value)
    {
        popup->digits[characterCount++] = (char)(value % 10);
        value /= 10;
    }
#endif

    if (characterCount == 0)
    {
        popup->digits[characterCount++] = '\0';
    }

    popup->characterCount = characterCount;
    popup->color = color;
    popup->timer = 0;
    popup->position = *position;
#if BUILT_BEFORE(102)
    // 1.00 and 1.01 move popups into the game region when they are created, for the software vertex path.
    if (!g_Supervisor.hasD3dHardwareVertexProcessing)
    {
        popup->position.x += g_GameManager.gameRegionScreenPos.x;
        popup->position.y += g_GameManager.gameRegionScreenPos.y;
    }
#endif

    this->nextPopupIndex1++;
}

void AsciiManager::CreatePopup2(D3DXVECTOR3 *position, i32 value, D3DCOLOR color)
{
    if (this->nextPopupIndex2 >= ASCII_PLAYER_POPUPS_COUNT)
    {
        this->nextPopupIndex2 = 0;
    }

    AsciiManagerPopup *popup = &this->popups[ASCII_PLAYER_POPUPS_START + this->nextPopupIndex2];
    popup->inUse = true;
    i32 characterCount = 0;

#if BUILT_SINCE(100)
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
#else
    while (value)
    {
        popup->digits[characterCount++] = (char)(value % 10);
        value /= 10;
    }
#endif

    if (characterCount == 0)
    {
        popup->digits[characterCount++] = '\0';
    }

    popup->characterCount = characterCount;
    popup->color = color;
    popup->timer = 0;
    popup->position = *position;
#if BUILT_BEFORE(102)
    // 1.00 and 1.01 move popups into the game region when they are created, for the software vertex path.
    if (!g_Supervisor.hasD3dHardwareVertexProcessing)
    {
        popup->position.x += g_GameManager.gameRegionScreenPos.x;
        popup->position.y += g_GameManager.gameRegionScreenPos.y;
    }
#endif

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
            if (this->menuSprites[vmIdx].IsVisible())
            {
                this->menuSprites[vmIdx].pendingInterrupt = 2;
            }
        }
        this->numFrames = 0;
        this->menuBackground.pendingInterrupt = 1;
    }
    if (WAS_PRESSED(TH_BUTTON_Q))
    {
        this->curState = GAME_MENU_QUIT_SELECTED_YES;
        for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
        {
            if (this->menuSprites[vmIdx].IsVisible())
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
            this->menuBackground.pos.x = GAME_REGION_POS_X;
            this->menuBackground.pos.y = GAME_REGION_POS_Y;
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
        if (this->numFrames >= 4)
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
        if (this->numFrames >= 4)
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
        if (this->numFrames >= 20)
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
        if (this->numFrames >= 4)
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
        if (this->numFrames >= 4)
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
        if (this->numFrames >= 20)
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
        g_Supervisor.viewport.X = g_GameManager.gameRegionScreenPos.x;
        g_Supervisor.viewport.Y = g_GameManager.gameRegionScreenPos.y;
        g_Supervisor.viewport.Width = g_GameManager.gameRegionSize.x;
        g_Supervisor.viewport.Height = g_GameManager.gameRegionSize.y;
        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
        if (g_Supervisor.lockableBackbuffer && this->curState != GAME_MENU_PAUSE_OPENING)
        {
            AnmVm menuBackground = this->menuBackground;
            menuBackground.flags.zWriteDisable = true;
            g_AnmManager->DrawNoRotation(&menuBackground);
        }
        for (vmIdx = 0; vmIdx < ARRAY_SIZE_SIGNED(this->menuSprites); vmIdx++)
        {
            if (this->menuSprites[vmIdx].IsVisible())
            {
                g_AnmManager->DrawNoRotation(&this->menuSprites[vmIdx]);
            }
        }
    }
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
    i32 i;

#if BUILT_SINCE(100)
    if (g_GameManager.isInPracticeMode)
    {
        g_GameManager.isInRetryMenu = false;
        g_GameManager.guiScore = g_GameManager.score;
        g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
        return 1;
    }
#endif
    if (g_GameManager.isInReplay)
    {
        g_GameManager.isInRetryMenu = false;
        g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU_REPLAY;
        g_GameManager.guiScore = g_GameManager.score;
        return 1;
    }
    if (g_GameManager.continuesUsed >= MAX_CONTINUE_CREDITS
#if BUILT_SINCE(100)
        || g_GameManager.difficulty >= EXTRA
#endif
    )
    {
        g_GameManager.isInRetryMenu = false;
        g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
        g_GameManager.guiScore = g_GameManager.score;
        return 1;
    }
    switch (this->curState)
    {
    case RETRY_MENU_OPENING:
        if (this->numFrames == 0)
        {
            for (i = RETRY_MENU_SPRITES_START; i < RETRY_MENU_SPRITES_END; i++)
            {
                if (i < 2)
                {
                    g_AnmManager->SetAndExecuteScriptIdx(&this->menuSprites[i], i + 8);
                }
                else
                {
                    g_AnmManager->SetAndExecuteScriptIdx(&this->menuSprites[i], i + 4);
                }
                this->menuSprites[i].pendingInterrupt = 1;
            }
            if (g_Supervisor.lockableBackbuffer)
            {
                g_AnmManager->RequestScreenshot();
                g_AnmManager->SetAndExecuteScriptIdx(&this->menuBackground, ANM_SCRIPT_CAPTURE_PAUSE_BG);
                this->menuBackground.pos.x = GAME_REGION_POS_X;
                this->menuBackground.pos.y = GAME_REGION_POS_Y;
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
        if (this->numFrames >= 4)
        {
            if (WAS_PRESSED(TH_BUTTON_UP) || WAS_PRESSED(TH_BUTTON_DOWN))
            {
                this->curState = RETRY_MENU_CURSOR_NO;
            }
            if (WAS_PRESSED(TH_BUTTON_SHOOT))
            {
                for (i = RETRY_MENU_SPRITES_START; i < RETRY_MENU_SPRITES_END; i++)
                {
                    this->menuSprites[i].pendingInterrupt = 2;
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
                for (i = RETRY_MENU_SPRITES_START; i < RETRY_MENU_SPRITES_END; i++)
                {
                    this->menuSprites[i].pendingInterrupt = 2;
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
            g_GameManager.isInRetryMenu = false;
            g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
            for (i = RETRY_MENU_SPRITES_START; i < RETRY_MENU_SPRITES_END; i++)
            {
                this->menuSprites[i].SetInvisible();
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
            g_GameManager.isInRetryMenu = false;
            for (i = RETRY_MENU_SPRITES_START; i < RETRY_MENU_SPRITES_END; i++)
            {
                this->menuSprites[i].SetInvisible();
            }
            g_GameManager.continuesUsed++;
            g_GameManager.guiScore = g_GameManager.continuesUsed;
            g_GameManager.nextScoreIncrement = 0;
            g_GameManager.score = g_GameManager.guiScore;
#if BUILT_SINCE(102g)
            g_GameManager.livesRemaining = g_Supervisor.defaultConfig.lifeCount;
            g_GameManager.bombsRemaining = g_Supervisor.defaultConfig.bombCount;
#else
            g_GameManager.livesRemaining = g_Supervisor.cfg.lifeCount;
            g_GameManager.bombsRemaining = g_Supervisor.cfg.bombCount;
#endif
            g_GameManager.grazeInStage = 0;
            g_GameManager.pointItemsCollectedInStage = 0;
            g_GameManager.currentPower = 0;
            g_GameManager.livesEarned = 0;
            g_Gui.flags.flag0 = 2;
            g_Gui.flags.flag1 = 2;
            g_Gui.flags.flag3 = 2;
            g_Gui.flags.flag4 = 2;
            g_Gui.flags.flag2 = 2;
            return 0;
        }
        break;
    }
    for (i = RETRY_MENU_SPRITES_START; i < RETRY_MENU_SPRITES_END; i++)
    {
        g_AnmManager->ExecuteScript(&this->menuSprites[i]);
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
    if (g_GameManager.isInRetryMenu)
    {
        g_Supervisor.viewport.X = g_GameManager.gameRegionScreenPos.x;
        g_Supervisor.viewport.Y = g_GameManager.gameRegionScreenPos.y;
        g_Supervisor.viewport.Width = g_GameManager.gameRegionSize.x;
        g_Supervisor.viewport.Height = g_GameManager.gameRegionSize.y;
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
            this->menuSprites[RETRY_MENU_SPRITE_RETRIES_NUMBER].sprite = g_AnmManager->GetSprite(
                ANM_SPRITE_ASCII_FONT_SPRITE(' ') + '0' + MAX_CONTINUE_CREDITS - g_GameManager.continuesUsed);
            g_AnmManager->DrawNoRotation(&this->menuSprites[RETRY_MENU_SPRITE_RETRIES_NUMBER]);
        }
        for (i32 i = RETRY_MENU_SPRITES_START; i < RETRY_MENU_SPRITES_END; i++)
        {
            if (this->menuSprites[i].IsVisible())
            {
                g_AnmManager->DrawNoRotation(&this->menuSprites[i]);
            }
        }
    }
}

#pragma var_order(currentPopup, j, i, currentDigit, unusedVec3)
void AsciiManager::DrawPopupsWithHwVertexProcessing()
{
    u8 *currentDigit;
    i32 i;
    i32 j;
    D3DXVECTOR3 unusedVec3; // NOTE: Not padding

    AsciiManagerPopup *currentPopup = this->popups;
    g_Supervisor.viewport.X = g_GameManager.gameRegionScreenPos.x;
    g_Supervisor.viewport.Y = g_GameManager.gameRegionScreenPos.y;
    g_Supervisor.viewport.Width = g_GameManager.gameRegionSize.x;
    g_Supervisor.viewport.Height = g_GameManager.gameRegionSize.y;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    for (i = 0; i < ASCII_TOTAL_POPUPS_COUNT; i++, currentPopup++)
    {
        if (!currentPopup->inUse)
        {
            continue;
        }

        this->scorePopupVm.pos.x = currentPopup->position.x - (currentPopup->characterCount * 4);
        this->scorePopupVm.pos.y = currentPopup->position.y;
        this->scorePopupVm.color = currentPopup->color;

        currentDigit = (u8 *)currentPopup->digits + currentPopup->characterCount - 1;
        for (j = currentPopup->characterCount; j > 0; j--)
        {
            this->scorePopupVm.sprite = g_AnmManager->GetSprite(*currentDigit);
#if BUILT_SINCE(100)
            if (*currentDigit >= '\n')
            {
                // TODO: explain these values
                this->scorePopupVm.matrix.m[0][0] = 0.1875f;
                this->scorePopupVm.matrix.m[1][1] = 0.03125f;
                g_AnmManager->Draw2(&this->scorePopupVm);
                this->scorePopupVm.matrix.m[0][0] = 0.03125f;
                this->scorePopupVm.matrix.m[1][1] = 0.03125f;
            }
            else
            {
                g_AnmManager->Draw2(&this->scorePopupVm);
            }
#else
            g_AnmManager->Draw2(&this->scorePopupVm);
#endif

            this->scorePopupVm.pos.x += 8.0f;
            currentDigit--;
        }
    }
}

#pragma var_order(currentPopup, j, i, currentDigit, unusedVec3)
void AsciiManager::DrawPopupsWithoutHwVertexProcessing()
{
    u8 *currentDigit;
    i32 i;
    i32 j;
    D3DXVECTOR3 unusedVec3; // NOTE: Not padding

    AsciiManagerPopup *currentPopup = this->popups;
    g_Supervisor.viewport.X = g_GameManager.gameRegionScreenPos.x;
    g_Supervisor.viewport.Y = g_GameManager.gameRegionScreenPos.y;
    g_Supervisor.viewport.Width = g_GameManager.gameRegionSize.x;
    g_Supervisor.viewport.Height = g_GameManager.gameRegionSize.y;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);

    for (i = 0; i < ASCII_TOTAL_POPUPS_COUNT; i++, currentPopup++)
    {
        if (!currentPopup->inUse)
        {
            continue;
        }

        this->scorePopupVm.pos.x = currentPopup->position.x - (currentPopup->characterCount * 4);
        this->scorePopupVm.pos.y = currentPopup->position.y;
        this->scorePopupVm.color = currentPopup->color;

        currentDigit = (u8 *)currentPopup->digits + currentPopup->characterCount - 1;
        for (j = currentPopup->characterCount; j > 0; j--)
        {
            this->scorePopupVm.sprite = g_AnmManager->GetSprite(*currentDigit + ANM_SPRITE_SCORE_TEXT);
#if BUILT_SINCE(100)
            if (*currentDigit >= '\n')
            {
                // TODO: explain these values
                this->scorePopupVm.matrix.m[0][0] = 0.1875f;
                this->scorePopupVm.matrix.m[1][1] = 0.03125f;
                g_AnmManager->DrawNoRotation(&this->scorePopupVm);
                this->scorePopupVm.matrix.m[0][0] = 0.03125f;
                this->scorePopupVm.matrix.m[1][1] = 0.03125f;
            }
            else
            {
                g_AnmManager->Draw2(&this->scorePopupVm);
            }
#else
            g_AnmManager->DrawNoRotation(&this->scorePopupVm);
#endif

            this->scorePopupVm.pos.x += 8.0f;
            currentDigit--;
        }
    }
}

} // namespace th06

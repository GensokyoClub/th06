#include "stdafx.h"

#include "ChainPriorities.hpp"
#include "Ending.hpp"
#include "ScreenEffect.hpp"

namespace th06
{

i32 Ending::ReadEndFileParameter()
{
    i32 readResult = atoi(this->endFileDataPtr);
    while (*this->endFileDataPtr != '\0')
    {
        this->endFileDataPtr++;
    }
    while (*this->endFileDataPtr == '\0')
    {
        this->endFileDataPtr++;
    }
    return readResult;
}

#pragma var_order(endingRect, color)
void Ending::FadingEffect()
{
    ZunRect endingRect;
    i32 color;

    endingRect.left = 0.0f;
    endingRect.top = 0.0f;
    endingRect.right = GAME_WINDOW_WIDTH;
    endingRect.bottom = GAME_WINDOW_HEIGHT;

    switch (this->fadeType)
    {
    case ENDING_FADE_TYPE_FADE_IN_BLACK:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = ENDING_FADE_TYPE_NO_FADE;
            this->fadeColor = COLOR_TRANSPARENT;
            break;
        }
        else
        {
            color = 255 - this->fadeTimer * 255 / this->fadeDuration;
            this->fadeColor = COLOR_SET_ALPHA(COLOR_BLACK, color);
            this->fadeTimer++;
            break;
        }
    case ENDING_FADE_TYPE_FADE_OUT_BLACK:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = COLOR_BLACK;
            break;
        }
        else
        {
            color = this->fadeTimer * 255 / this->fadeDuration;
            this->fadeColor = COLOR_SET_ALPHA(COLOR_BLACK, color);
            this->fadeTimer++;
            break;
        }
    case ENDING_FADE_TYPE_FADE_IN_WHITE:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeType = ENDING_FADE_TYPE_NO_FADE;
            this->fadeColor = COLOR_TRANSPARENT;
            break;
        }
        else
        {
            color = 255 - this->fadeTimer * 255 / this->fadeDuration;
            this->fadeColor = COLOR_SET_ALPHA(COLOR_WHITE, color);
            this->fadeTimer++;
            break;
        }
    case ENDING_FADE_TYPE_FADE_OUT_WHITE:
        if (this->fadeTimer >= this->fadeDuration)
        {
            this->fadeColor = COLOR_WHITE;
            break;
        }
        else
        {
            color = this->fadeTimer * 255 / this->fadeDuration;
            this->fadeColor = COLOR_SET_ALPHA(COLOR_WHITE, color);
            this->fadeTimer++;
            break;
        }
    case ENDING_FADE_TYPE_NO_FADE:
        this->fadeColor = COLOR_TRANSPARENT;
        break;
    }
    if ((this->fadeColor & COLOR_ALPHA_MASK) != COLOR_TRANSPARENT)
    {
        ScreenEffect_DrawSquare(&endingRect, this->fadeColor);
    }
}

#pragma var_order(firstLineDisplayed, textBuffer, charactersRead)
ZunResult Ending::ParseEndFile()
{
    char textBuffer[38];

    ZunBool firstLineDisplayed = false;
    i32 charactersRead = 0;

    memset(textBuffer, 0, sizeof(textBuffer));

    if (this->clearTimer > 0)
    {
        this->clearTimer--;
        if (this->minWaitClearFrames != 0)
        {
            this->minWaitClearFrames--;
        }
        else
        {
            if (WAS_PRESSED(TH_BUTTON_SELECTMENU) || this->hasSeenEnding && IS_PRESSED(TH_BUTTON_SKIP))
            {
                this->clearTimer = 0;
            }
        }
        if (this->clearTimer <= 0)
        {
            memset(this->sprites, 0, sizeof(this->sprites));
            this->textLine = 0;
        }
        else
        {
            goto break_parser;
        }
    }

    if (this->pauseTimer > 0)
    {
        this->pauseTimer--;

        if (this->minWaitPauseFrames != 0)
        {
            this->minWaitPauseFrames--;
        }
        else
        {
            if (WAS_PRESSED(TH_BUTTON_SELECTMENU) || this->hasSeenEnding && IS_PRESSED(TH_BUTTON_SKIP))
            {
                this->pauseTimer = 0;
            }
        }
        goto break_parser;
    }

    while (true)
    {
        switch (*this->endFileDataPtr)
        {
        case END_READ_OPCODE:
            /* If there is an @ symbol, that means we have an opcode to read. */
            this->endFileDataPtr++;
            switch (*this->endFileDataPtr)
            {
            case END_OPCODE_BACKGROUND: // background(jpg_file)

                if (g_AnmManager->LoadSurface(0, this->endFileDataPtr + 1) != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
                break;

#pragma var_order(scriptIdx, vmIndex, spriteIdx)
            case END_OPCODE_ANM_SET_SLOT: { // anm(vmIndex, scriptIdx, spriteIdx)
                this->endFileDataPtr++;
                i32 vmIndex = this->ReadEndFileParameter();
                i32 scriptIdx = this->ReadEndFileParameter();
                i32 spriteIdx = this->ReadEndFileParameter();
                g_AnmManager->ExecuteAnmIdx(&this->sprites[vmIndex], ANM_OFFSET_STAFF01 + scriptIdx);
                g_AnmManager->SetActiveSprite(&this->sprites[vmIndex], ANM_OFFSET_STAFF01 + spriteIdx);
                break;
            }
#pragma var_order(scrollBGDistance, scrollBGDuration)
            case END_OPCODE_BACKGROUND_SCROLL: { // scrollbg(scrollBGDistance, scrollBGDuration)
                this->endFileDataPtr++;
                i32 scrollBGDistance = this->ReadEndFileParameter();
                i32 scrollBGDuration = this->ReadEndFileParameter();
                this->backgroundScrollSpeed = scrollBGDistance / (f32)scrollBGDuration;
                break;
            }
            case END_OPCODE_BACKGROUND_SCROLL_SET: // setscroll(newVertCoordinate)
                this->endFileDataPtr++;
                this->backgroundPos.y = this->ReadEndFileParameter(); // newVertCoordinate
                break;

            case END_OPCODE_END_SWITCH: { // exec(endfile)

                if (this->LoadEnding(this->endFileDataPtr + 1) != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
                charactersRead = 0;
                firstLineDisplayed = false;
                for (i32 shottype = 0; shottype < SHOTTYPE_COUNT; shottype++)
                {
                    for (i32 difficulty = 0; difficulty < EXTRA; difficulty++)
                    {
                        if (g_GameManager.clrd[shottype].stagesClearedWithoutContinues[difficulty] == ALL_CLEARED ||
                            g_GameManager.clrd[shottype].stagesCleared[difficulty] == ALL_CLEARED)
                        {
                            this->hasSeenEnding = true;
                            break;
                        }
                    }
                }
                // no break
            }
            case END_OPCODE_CLEAR: { // staffroll()
                for (i32 spriteIdx = 0; spriteIdx < END_SPRITE_COUNT; spriteIdx++)
                {
                    this->sprites[spriteIdx].anmFileIndex = 0;
                }
                break;
            }
            case END_OPCODE_MUSIC: // musicplay(file)
                g_Supervisor.PlayAudio(this->endFileDataPtr + 1);
                break;

            case END_OPCODE_MUSIC_FADE_OUT: { // musicfade(duration)
                this->endFileDataPtr++;
                float musicFadeFrames = this->ReadEndFileParameter();
                g_Supervisor.FadeOutMusic(musicFadeFrames);
                break;
            }
            case END_OPCODE_SET_DELAY: // setdelay(longDelay, shortDelay)
                this->endFileDataPtr++;
                this->longDelay = this->ReadEndFileParameter();  // longDelay
                this->shortDelay = this->ReadEndFileParameter(); // shortDelay
                break;

            case END_OPCODE_TEXT_COLOR: // color(bgr_color)
                this->endFileDataPtr++;
                this->textColor = this->ReadEndFileParameter(); // newcolor
                break;

            case END_OPCODE_WAIT_CLEAR: // waitreset(maxframes, minframes)
                this->endFileDataPtr++;
                this->clearTimer = this->ReadEndFileParameter();         // maxFrames
                this->minWaitClearFrames = this->ReadEndFileParameter(); // minframes
                // Skip to end of line
                while (*this->endFileDataPtr != '\n' && *this->endFileDataPtr != '\r')
                {
                    this->endFileDataPtr++;
                }
                // Skip to start of next line
                while (*this->endFileDataPtr == '\n' || *this->endFileDataPtr == '\r')
                {
                    this->endFileDataPtr++;
                }
                goto break_parser;

            case END_OPCODE_WAIT: // wait(maxFrames, minFrames)
                this->endFileDataPtr++;
                this->pauseTimer = this->ReadEndFileParameter();         // maxFrames
                this->minWaitPauseFrames = this->ReadEndFileParameter(); // minFrames
                // Skip to end of line
                while (*this->endFileDataPtr != '\n' && *this->endFileDataPtr != '\r')
                {
                    this->endFileDataPtr++;
                }
                // Skip to start of next line
                while (*this->endFileDataPtr == '\n' || *this->endFileDataPtr == '\r')
                {
                    this->endFileDataPtr++;
                }
                goto break_parser;

            case END_OPCODE_FADE_IN_BLACK: // fadeinblack(frames). UNUSED
                this->endFileDataPtr++;
                this->fadeType = ENDING_FADE_TYPE_FADE_IN_BLACK;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadEndFileParameter(); // fadeInBlackFrames
                break;

            case END_OPCODE_FADE_OUT_BLACK: // fadeoutblack(frames). UNUSED
                this->endFileDataPtr++;
                this->fadeType = ENDING_FADE_TYPE_FADE_OUT_BLACK;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadEndFileParameter(); // fadeOutBlackFrames
                break;

            case END_OPCODE_FADE_IN_WHITE: // fadein(frames)
                this->endFileDataPtr++;
                this->fadeType = ENDING_FADE_TYPE_FADE_IN_WHITE;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadEndFileParameter(); // fadeInFrames
                break;

            case END_OPCODE_FADE_OUT_WHITE: // fadeout(frames)
                this->endFileDataPtr++;
                this->fadeType = ENDING_FADE_TYPE_FADE_OUT_WHITE;
                this->fadeTimer = 0;
                this->fadeDuration = this->ReadEndFileParameter(); // fadeOutFrames
                break;

            case END_OPCODE_END_DELETE:
                return ZUN_ERROR;
            }

            // Skip to end of line
            while (*this->endFileDataPtr != '\n' && *this->endFileDataPtr != '\r')
            {
                this->endFileDataPtr++;
            }
            // Skip to start of next line
            while (*this->endFileDataPtr == '\n' || *this->endFileDataPtr == '\r')
            {
                this->endFileDataPtr++;
            }
            break;

        case '\0':
        case '\n':
        case '\r':
            // When encountered a breakline or null byte, display the text already loaded in textBuffer
            if (charactersRead != 0)
            {
                g_AnmManager->SetAndExecuteScriptIdx(
                    &this->sprites[firstLineDisplayed + this->textLine * SPRITES_PER_LINE_GROUP],
                    ANM_SCRIPT_TEXT_ENDING_TEXT + firstLineDisplayed + this->textLine * SPRITES_PER_LINE_GROUP);
                g_AnmManager->DrawVmTextFmt(
                    &this->sprites[firstLineDisplayed + this->textLine * SPRITES_PER_LINE_GROUP], this->textColor,
                    COLOR_END_TEXT_SHADOW, textBuffer);
            }
            // Skip to start of next line
            while (*this->endFileDataPtr == '\n' || *this->endFileDataPtr == '\0' || *this->endFileDataPtr == '\r')
            {
                this->endFileDataPtr++;
            }

            if (IS_PRESSED(TH_BUTTON_SELECTMENU))
            {
                this->pauseTimer = this->shortDelay;
                this->minWaitPauseFrames = this->shortDelay;
            }
            else
            {
                this->pauseTimer = this->longDelay;
                this->minWaitPauseFrames = this->longDelay;
            }
            this->textLine++;
            goto break_parser;

        default: // END_OPCODE_TEXT_DIALOGUE
            // Read 2 characters at a time
            textBuffer[charactersRead] = this->endFileDataPtr[0];
            textBuffer[charactersRead + 1] = this->endFileDataPtr[1];
            charactersRead += 2;
            this->endFileDataPtr += 2;

            // When reached the character limit, display the text now
            if (charactersRead >= 32)
            {
                g_AnmManager->SetAndExecuteScriptIdx(
                    &this->sprites[firstLineDisplayed + this->textLine * SPRITES_PER_LINE_GROUP],
                    ANM_SCRIPT_TEXT_ENDING_TEXT + firstLineDisplayed + this->textLine * SPRITES_PER_LINE_GROUP);
                g_AnmManager->DrawVmTextFmt(
                    &this->sprites[firstLineDisplayed + this->textLine * SPRITES_PER_LINE_GROUP], this->textColor,
                    COLOR_END_TEXT_SHADOW, textBuffer);
                if (firstLineDisplayed)
                {
                    goto break_parser;
                }
                firstLineDisplayed = true;
                charactersRead = 0;

                memset(textBuffer, 0, sizeof(textBuffer));
            }
            // continue;
        }
    }

break_parser:
    this->scriptTime++;
    this->backgroundPos.y -= this->backgroundScrollSpeed;
    if (this->backgroundPos.y <= 0.0f)
    {
        this->backgroundPos.y = 0.0f;
        this->backgroundScrollSpeed = 0.0f;
    }

    return ZUN_SUCCESS;
}

ZunResult Ending::LoadEnding(const char *endFilePath)
{
    u8 *endFileDat = this->endFileData;
    this->endFileData = FileSystem::OpenPath(endFilePath);
    if (this->endFileData == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_ENDING_END_FILE_CORRUPTED);
        return ZUN_ERROR;
    }
    else
    {
        this->endFileDataPtr = (char *)this->endFileData;
        this->longDelay = 8;
        this->pauseTimer = 0;
        this->scriptTime = 0;
        if (endFileDat != NULL)
        {
            free(endFileDat);
        }
        return ZUN_SUCCESS;
    }
}

static ChainCallbackResult Ending_OnUpdate(Ending *ending)
{
    i32 framesSkipped = 0;
skipping:
    if (ending->ParseEndFile() != ZUN_SUCCESS)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
    }
    for (i32 idx = 0; idx < END_SPRITE_COUNT; idx++)
    {
        if (ending->sprites[idx].anmFileIndex != 0)
        {
            g_AnmManager->ExecuteScript(&ending->sprites[idx]);
        }
    }
    if (ending->hasSeenEnding && IS_PRESSED(TH_BUTTON_SKIP) && framesSkipped < 4)
    {
        framesSkipped++;
        goto skipping;
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

static ChainCallbackResult Ending_OnDraw(Ending *ending)
{
    g_AnmManager->DrawEndingRect(0, 0, 0, ending->backgroundPos.x, ending->backgroundPos.y, GAME_WINDOW_WIDTH,
                                 GAME_WINDOW_HEIGHT);
    for (i32 idx = 0; idx < END_SPRITE_COUNT; idx++)
    {
        if (ending->sprites[idx].anmFileIndex != 0)
        {
            g_AnmManager->DrawNoRotation(&ending->sprites[idx]);
        }
    }
    ending->FadingEffect();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(unusedShottype, shottype)
static ZunResult Ending_AddedCallback(Ending *ending)
{
    i32 unusedShottype = CURRENT_SHOTTYPE();

    g_GameManager.isGameCompleted = true;
    g_Supervisor.isInEnding = true;
    g_Supervisor.LoadPbg3(ED_PBG3_INDEX, TH_ED_DAT_FILE);
    g_AnmManager->LoadAnm(ANM_FILE_STAFF01, "data/staff01.anm", ANM_OFFSET_STAFF01);
    g_AnmManager->LoadAnm(ANM_FILE_STAFF02, "data/staff02.anm", ANM_OFFSET_STAFF02);
    g_AnmManager->LoadAnm(ANM_FILE_STAFF03, "data/staff03.anm", ANM_OFFSET_STAFF03);

    g_AnmManager->ClearTexture();
    g_AnmManager->ClearSprite();
    g_AnmManager->ClearBlendMode();
    g_AnmManager->ClearVertexShader();

    i32 shottype = CURRENT_SHOTTYPE();
    ending->hasSeenEnding = false;
    if (g_GameManager.continuesUsed == 0)
    {
        if (g_GameManager.clrd[shottype].stagesClearedWithoutContinues[g_GameManager.difficulty] == ALL_CLEARED)
        {
            ending->hasSeenEnding = true;
        }

        g_GameManager.clrd[shottype].stagesClearedWithoutContinues[g_GameManager.difficulty] = ALL_CLEARED;
    }
    else
    {
        if (g_GameManager.clrd[shottype].stagesCleared[g_GameManager.difficulty] == ALL_CLEARED)
        {
            ending->hasSeenEnding = true;
        }
    }
    g_GameManager.clrd[shottype].stagesCleared[g_GameManager.difficulty] = ALL_CLEARED;

    if (g_GameManager.difficulty == EASY || g_GameManager.continuesUsed != 0)
    {
        switch (g_GameManager.character)
        {
        case REIMU:
            if (ending->LoadEnding("data/end00b.end") != ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            break;
        case MARISA:
            if (ending->LoadEnding("data/end10b.end") != ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            break;
        }
    }
    else
    {
        switch (g_GameManager.character)
        {
        case REIMU:
            if (g_GameManager.shotType == SHOT_TYPE_A)
            {
                if (ending->LoadEnding("data/end00.end") != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
            }
            else
            {
                if (ending->LoadEnding("data/end01.end") != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
            }
            break;
        case MARISA:
            if (g_GameManager.shotType == SHOT_TYPE_A)
            {
                if (ending->LoadEnding("data/end10.end") != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
            }
            else
            {
                if (ending->LoadEnding("data/end11.end") != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
            }
            break;
        }
    }
    return ZUN_SUCCESS;
}

static ZunResult Ending_DeletedCallback(Ending *ending)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_STAFF01);
    g_AnmManager->ReleaseAnm(ANM_FILE_STAFF02);
    g_AnmManager->ReleaseAnm(ANM_FILE_STAFF03);

    g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;

    g_AnmManager->ReleaseSurface(0);
    ZUN_FREE(ending->endFileData);

    g_FuncChain.Cut(ending->drawChain);
    ending->drawChain = NULL;

    ZUN_DELETE(ending);

    g_Supervisor.isInEnding = false;
    g_Supervisor.ReleasePbg3(ED_PBG3_INDEX);
    return ZUN_SUCCESS;
}

ZunResult Ending_RegisterChain()
{
    Ending *ending = ZUN_NEW(Ending);
    ending->calcChain = g_FuncChain.CreateElem((ChainCallback)Ending_OnUpdate);
    ending->calcChain->arg = ending;
    ending->calcChain->addedCallback = (ChainAddedCallback)Ending_AddedCallback;
    ending->calcChain->deletedCallback = (ChainDeletedCallback)Ending_DeletedCallback;
    if (g_FuncChain.AddToCalcChain(ending->calcChain, TH_CHAIN_PRIO_CALC_ENDING) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    ending->drawChain = g_FuncChain.CreateElem((ChainCallback)Ending_OnDraw);
    ending->drawChain->arg = ending;
    g_FuncChain.AddToDrawChain(ending->drawChain, TH_CHAIN_PRIO_DRAW_ENDING);

    return ZUN_SUCCESS;
}
} // namespace th06

#include "Ending.hpp"
#include "AnmIdx.hpp"
#include "AnmManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ScreenEffect.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th06
{
i32 Ending::ReadEndFileParameter()
{
    i32 readResult;

    readResult = atol(this->endFileDataPtr);
    while (this->endFileDataPtr[0] != '\0')
    {
        this->endFileDataPtr++;
    }
    while (this->endFileDataPtr[0] == '\0')
    {
        this->endFileDataPtr++;
    }
    return readResult;
}

#pragma var_order(endingRect, color)
void Ending::FadingEffect()
{
    ZunRect endingRect;
    ZunColor color;

    endingRect.left = 0.0;
    endingRect.top = 0.0;
    endingRect.right = 640.0;
    endingRect.bottom = 480.0;

    switch (this->fadeType)
    {
    case ENDING_FADE_TYPE_FADE_IN_BLACK:
        if (this->timeFading >= this->fadeFrames)
        {
            this->fadeType = ENDING_FADE_TYPE_NO_FADE;
            this->endingFadeColor = 0x00000000;
            break;
        }
        else
        {
            color = 255 - this->timeFading * 255 / this->fadeFrames;
            this->endingFadeColor = COLOR_SET_ALPHA(COLOR_BLACK, color);
            this->timeFading++;
            break;
        }
    case ENDING_FADE_TYPE_FADE_OUT_BLACK:
        if (this->timeFading >= this->fadeFrames)
        {
            this->endingFadeColor = COLOR_BLACK;
            break;
        }
        else
        {
            color = this->timeFading * 255 / this->fadeFrames;
            this->endingFadeColor = COLOR_SET_ALPHA(COLOR_BLACK, color);
            this->timeFading++;
            break;
        }
    case ENDING_FADE_TYPE_FADE_IN_WHITE:
        if (this->timeFading >= this->fadeFrames)
        {
            this->fadeType = ENDING_FADE_TYPE_NO_FADE;
            this->endingFadeColor = 0x00000000;
            break;
        }
        else
        {
            color = 255 - this->timeFading * 255 / this->fadeFrames;
            this->endingFadeColor = COLOR_SET_ALPHA(COLOR_WHITE, color);
            this->timeFading++;
            break;
        }
    case ENDING_FADE_TYPE_FADE_OUT_WHITE:
        if (this->timeFading >= this->fadeFrames)
        {
            this->endingFadeColor = COLOR_WHITE;
            break;
        }
        else
        {
            color = this->timeFading * 255 / this->fadeFrames;
            this->endingFadeColor = COLOR_SET_ALPHA(COLOR_WHITE, color);
            this->timeFading++;
            break;
        }
    case ENDING_FADE_TYPE_NO_FADE:
        this->endingFadeColor = 0x00000000;
        break;
    }
    if ((this->endingFadeColor & COLOR_BLACK) != 0)
    {
        ScreenEffect::DrawSquare(&endingRect, this->endingFadeColor);
    }
}

#pragma var_order(lineDisplayed, textBuffer, charactersReaded, anmScriptIdx, vmIndex, anmSpriteIdx, scrollBGDistance,  \
                  scrollBGDuration, characterIdx, diffIdx, spriteIdx, musicFadeFrames, fill)
ZunResult Ending::ParseEndFile()
{
    i32 vmIndex;
    i32 anmScriptIdx;
    i32 anmSpriteIdx;
    i32 scrollBGDistance;
    i32 scrollBGDuration;
    f32 musicFadeFrames;
    i32 spriteIdx;
    i32 diffIdx;
    i32 characterIdx;
    i32 charactersReaded;
    ZunBool lineDisplayed;
    i32 fill[6];

    char textBuffer[39];

    lineDisplayed = false;
    charactersReaded = 0;

    memset(textBuffer, 0, sizeof(textBuffer) - 1);

    if (this->timer3 > 0)
    {
        this->timer3.Decrement(1);
        if (this->minWaitResetFrames != 0)
        {
            this->minWaitResetFrames--;
        }
        else
        {
            if (WAS_PRESSED(TH_BUTTON_SELECTMENU) || this->hasSeenEnding && IS_PRESSED(TH_BUTTON_SKIP))
            {
                this->timer3.InitializeForPopup();
            }
        }
        if (this->timer3 <= 0)
        {
            memset(this->sprites, 0, sizeof(this->sprites));
            this->timesFileParsed = 0;
        }
        else
        {
            goto endParsing;
        }
    }

    if (this->timer2 > 0)
    {
        this->timer2.Decrement(1);

        if (this->minWaitFrames != 0)
        {
            this->minWaitFrames--;
        }
        else
        {
            if (WAS_PRESSED(TH_BUTTON_SELECTMENU) || this->hasSeenEnding && IS_PRESSED(TH_BUTTON_SKIP))
            {
                this->timer2.InitializeForPopup();
            }
        }
        goto endParsing;
    }

    while (true)
    {
        switch (this->endFileDataPtr[0])
        {
        case END_READ_OPCODE:
            /* If there is an @ symbol, that means we have an opcode to read. */
            this->endFileDataPtr++;
            switch (this->endFileDataPtr[0])
            {
            case END_OPCODE_BACKGROUND:
                /* background(jpg_file) */

                if (g_AnmManager->LoadSurface(0, this->endFileDataPtr + 1) != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
                break;

            case END_OPCODE_EXECUTE_ANM:
                /* anm(vm_index, script_index, sprite_index) */
                this->endFileDataPtr++;
                vmIndex = this->ReadEndFileParameter();      // vm_index
                anmScriptIdx = this->ReadEndFileParameter(); // script_index
                anmSpriteIdx = this->ReadEndFileParameter(); // sprite_index
                g_AnmManager->ExecuteAnmIdx(&this->sprites[vmIndex], ANM_OFFSET_STAFF01 + anmScriptIdx);
                g_AnmManager->SetActiveSprite(&this->sprites[vmIndex], ANM_OFFSET_STAFF01 + anmSpriteIdx);
                break;

            case END_OPCODE_SCROLL_BACKGROUND:
                /* scrollbg(distance, duration) */
                this->endFileDataPtr++;
                scrollBGDistance = this->ReadEndFileParameter(); // distance
                scrollBGDuration = this->ReadEndFileParameter(); // duration
                this->backgroundScrollSpeed = scrollBGDistance / (f32)scrollBGDuration;
                break;

            case END_OPCODE_SET_VERTICAL_SCROLL_POS:
                /* setscroll(newVertCoordinate) */
                this->endFileDataPtr++;

                this->backgroundPos.y = this->ReadEndFileParameter(); // newVertCoordinate
                break;

            case END_OPCODE_EXEC_END_FILE:
                /* exec(endfile) */

                if (this->LoadEnding(this->endFileDataPtr + 1) != ZUN_SUCCESS)
                {
                    return ZUN_ERROR;
                }
                charactersReaded = 0;
                lineDisplayed = false;
                for (characterIdx = 0; characterIdx < ARRAY_SIZE_SIGNED(g_GameManager.clrd); characterIdx++)
                {
                    for (diffIdx = 0; diffIdx < EXTRA; diffIdx++)
                    {
                        if (g_GameManager.clrd[characterIdx].difficultyClearedWithRetries[diffIdx] == 99 ||
                            g_GameManager.clrd[characterIdx].difficultyClearedWithoutRetries[diffIdx] == 99)
                        {
                            this->hasSeenEnding = true;
                            break;
                        }
                    }
                }

            case END_OPCODE_ROLL_STAFF:
                /* staffroll()
                   Assumingly this clears the entire anm stack allocated for Ending. */

                for (spriteIdx = 0; spriteIdx < ARRAY_SIZE_SIGNED(this->sprites); spriteIdx++)
                {
                    this->sprites[spriteIdx].anmFileIndex = 0;
                }
                break;

            case END_OPCODE_PLAY_MUSIC:
                /* musicplay(file) */
                g_Supervisor.PlayAudio(this->endFileDataPtr + 1);
                break;

            case END_OPCODE_FADE_MUSIC:
                /* musicfade(duration) */
                this->endFileDataPtr++;
                musicFadeFrames = this->ReadEndFileParameter();
                g_Supervisor.FadeOutMusic(musicFadeFrames);
                break;

            case END_OPCODE_SET_DELAY:
                /* setdelay(line2Delay, topLineDelay) */
                this->endFileDataPtr++;

                this->line2Delay = this->ReadEndFileParameter();   // line2Delay
                this->topLineDelay = this->ReadEndFileParameter(); // topLineDelay
                break;

            case END_OPCODE_COLOR:
                /* color(bgr_color) */
                this->endFileDataPtr++;
                this->textColor = this->ReadEndFileParameter(); // newcolor
                break;

            case END_OPCODE_WAIT_RESET:
                /* waitreset(maxframes, minframes) */
                this->endFileDataPtr++;
                this->timer3.SetCurrent(this->ReadEndFileParameter());   // maxFrames
                this->minWaitResetFrames = this->ReadEndFileParameter(); // minframes
                while (this->endFileDataPtr[0] != '\n' && this->endFileDataPtr[0] != '\r')
                {
                    this->endFileDataPtr++;
                }
                while (this->endFileDataPtr[0] == '\n' || this->endFileDataPtr[0] == '\r')
                {
                    this->endFileDataPtr++;
                }
                goto endParsing;

            case END_OPCODE_WAIT:
                /* wait(maxFrames, minFrames) */
                this->endFileDataPtr++;
                this->timer2.SetCurrent(this->ReadEndFileParameter()); // maxFrames
                this->minWaitFrames = this->ReadEndFileParameter();    // minFrames
                while (this->endFileDataPtr[0] != '\n' && this->endFileDataPtr[0] != '\r')
                {
                    this->endFileDataPtr++;
                }
                while (this->endFileDataPtr[0] == '\n' || this->endFileDataPtr[0] == '\r')
                {
                    this->endFileDataPtr++;
                }
                goto endParsing;

            case END_OPCODE_FADE_IN_BLACK:
                /* fadeinblack(frames). UNUSED */
                this->endFileDataPtr++;
                this->fadeType = ENDING_FADE_TYPE_FADE_IN_BLACK;
                this->timeFading = 0;
                this->fadeFrames = this->ReadEndFileParameter(); // fadeInBlackFrames
                break;

            case END_OPCODE_FADE_OUT_BLACK:
                /* fadeoutblack(frames). UNUSED */
                this->endFileDataPtr++;
                this->fadeType = ENDING_FADE_TYPE_FADE_OUT_BLACK;
                this->timeFading = 0;
                this->fadeFrames = this->ReadEndFileParameter(); // fadeOutBlackFrames
                break;

            case END_OPCODE_FADE_IN:
                /* fadein(frames) */
                this->endFileDataPtr++;
                this->fadeType = ENDING_FADE_TYPE_FADE_IN_WHITE;
                this->timeFading = 0;
                this->fadeFrames = this->ReadEndFileParameter(); // fadeInFrames
                break;

            case END_OPCODE_FADE_OUT:
                /* fadeout(frames) */
                this->endFileDataPtr++;
                this->fadeType = ENDING_FADE_TYPE_FADE_OUT_WHITE;
                this->timeFading = 0;
                this->fadeFrames = this->ReadEndFileParameter(); // fadeOutFrames
                break;

            case END_OPCODE_END:
                return ZUN_ERROR;
            }

            while ((this->endFileDataPtr[0] != '\n' && (this->endFileDataPtr[0] != '\r')))
            {
                this->endFileDataPtr++;
            }
            while ((this->endFileDataPtr[0] == '\n' || (this->endFileDataPtr[0] == '\r')))
            {
                this->endFileDataPtr++;
            }
            goto nextOpcode;

        case '\0':
        case '\n':
        case '\r':
            // When encountered a breakline or null byte, display the text already loaded in textBuffer
            if (charactersReaded != 0)
            {
                g_AnmManager->SetAndExecuteScriptIdx(&this->sprites[lineDisplayed + this->timesFileParsed * 2],
                                                     lineDisplayed + ANM_SCRIPT_TEXT_ENDING_TEXT +
                                                         this->timesFileParsed * 2);
                AnmManager::DrawVmTextFmt(g_AnmManager, &this->sprites[lineDisplayed + this->timesFileParsed * 2],
                                          this->textColor, COLOR_END_TEXT_SHADOW, textBuffer);
            }
            while (this->endFileDataPtr[0] == '\n' || this->endFileDataPtr[0] == '\0' ||
                   this->endFileDataPtr[0] == '\r')
            {
                this->endFileDataPtr++;
            }

            // If select button is pressed, display the next line instantly? not sure
            if (IS_PRESSED(TH_BUTTON_SELECTMENU))
            {
                this->timer2.SetCurrent(this->topLineDelay);
                this->minWaitFrames = this->topLineDelay;
            }
            else
            {
                this->timer2.SetCurrent(this->line2Delay);
                this->minWaitFrames = this->line2Delay;
            }

            this->timesFileParsed++;
            goto endParsing;
        default:
            // Read 2 characters at a time
            textBuffer[charactersReaded] = this->endFileDataPtr[0];
            textBuffer[charactersReaded + 1] = this->endFileDataPtr[1];
            charactersReaded += 2;
            this->endFileDataPtr += 2;

            // When reached the character limit, display the text now
            if (charactersReaded >= 32)
            {
                g_AnmManager->SetAndExecuteScriptIdx(&this->sprites[lineDisplayed + this->timesFileParsed * 2],
                                                     lineDisplayed + ANM_SCRIPT_TEXT_ENDING_TEXT +
                                                         this->timesFileParsed * 2);
                AnmManager::DrawVmTextFmt(g_AnmManager, &this->sprites[lineDisplayed + this->timesFileParsed * 2],
                                          this->textColor, COLOR_END_TEXT_SHADOW, textBuffer);
                if (lineDisplayed)
                {
                    goto endParsing;
                }
                lineDisplayed = true;
                charactersReaded = 0;

                memset(textBuffer, 0, sizeof(textBuffer) - 1);
            }
        nextOpcode:
            continue;
        }

        break;
    }

endParsing:
    this->timer1.Tick();
    this->backgroundPos.y -= this->backgroundScrollSpeed;
    if (this->backgroundPos.y <= 0.0f)
    {
        this->backgroundPos.y = 0.0f;
        this->backgroundScrollSpeed = 0.0f;
    }

    return ZUN_SUCCESS;
}

ZunResult Ending::LoadEnding(char *endFilePath)
{
    char *endFileDat;

    endFileDat = this->endFileData;
    this->endFileData = (char *)FileSystem::OpenPath(endFilePath, false);
    if (this->endFileData == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_ENDING_END_FILE_CORRUPTED);
        return ZUN_ERROR;
    }
    else
    {
        this->endFileDataPtr = this->endFileData;
        this->line2Delay = 8;
        this->timer2.InitializeForPopup();
        this->timer1.InitializeForPopup();
        if (endFileDat != NULL)
        {
            free(endFileDat);
        }
        return ZUN_SUCCESS;
    }
}

ZunResult Ending::RegisterChain()
{
    Ending *ending;

    ending = new Ending();
    ending->calcChain = g_Chain.CreateElem((ChainCallback)Ending::OnUpdate);
    ending->calcChain->arg = ending;
    ending->calcChain->addedCallback = (ChainAddedCallback)Ending::AddedCallback;
    ending->calcChain->deletedCallback = (ChainDeletedCallback)Ending::DeletedCallback;
    if (g_Chain.AddToCalcChain(ending->calcChain, TH_CHAIN_PRIO_CALC_ENDING))
    {
        return ZUN_ERROR;
    }

    ending->drawChain = g_Chain.CreateElem((ChainCallback)Ending::OnDraw);
    ending->drawChain->arg = ending;
    g_Chain.AddToDrawChain(ending->drawChain, TH_CHAIN_PRIO_DRAW_ENDING);

    return ZUN_SUCCESS;
}

#pragma var_order(framesPressed, idx)
ChainCallbackResult Ending::OnUpdate(Ending *ending)
{
    i32 idx;
    i32 framesPressed;

    for (framesPressed = 0;;)
    {
        if (ending->ParseEndFile() != ZUN_SUCCESS)
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(ending->sprites); idx++)
        {
            if (ending->sprites[idx].anmFileIndex != 0)
            {
                g_AnmManager->ExecuteScript(&ending->sprites[idx]);
            }
        }
        if (ending->hasSeenEnding && IS_PRESSED(TH_BUTTON_SKIP) && framesPressed < 4)
        {
            framesPressed++;
            continue;
        }
        break;
    };
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult Ending::OnDraw(Ending *ending)
{
    i32 idx;

    g_AnmManager->DrawEndingRect(0, 0, 0, ending->backgroundPos.x, ending->backgroundPos.y, 640, 480);
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(ending->sprites); idx++)
    {
        if (ending->sprites[idx].anmFileIndex != 0)
        {
            g_AnmManager->DrawNoRotation(&ending->sprites[idx]);
        }
    }
    ending->FadingEffect();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(unused, shotTypeAndCharacter)
ZunResult Ending::AddedCallback(Ending *ending)
{
    i32 shotTypeAndCharacter;
    i32 unused;

    unused = g_GameManager.character * 2 + g_GameManager.shotType;

    g_GameManager.isGameCompleted = true;
    g_Supervisor.isInEnding = true;
    g_Supervisor.LoadPbg3(ED_PBG3_INDEX, TH_ED_DAT_FILE);
    g_AnmManager->LoadAnm(ANM_FILE_STAFF01, "data/staff01.anm", ANM_OFFSET_STAFF01);
    g_AnmManager->LoadAnm(ANM_FILE_STAFF02, "data/staff02.anm", ANM_OFFSET_STAFF02);
    g_AnmManager->LoadAnm(ANM_FILE_STAFF03, "data/staff03.anm", ANM_OFFSET_STAFF03);

    g_AnmManager->SetCurrentTexture(NULL);
    g_AnmManager->SetCurrentSprite(NULL);
    g_AnmManager->SetCurrentBlendMode(0xff);
    g_AnmManager->SetCurrentVertexShader(0xff);

    shotTypeAndCharacter = g_GameManager.character * 2 + g_GameManager.shotType;
    ending->hasSeenEnding = false;
    if (g_GameManager.numRetries == 0)
    {
        if (g_GameManager.clrd[shotTypeAndCharacter].difficultyClearedWithRetries[g_GameManager.difficulty] == 99)
        {
            ending->hasSeenEnding = true;
        }

        g_GameManager.clrd[shotTypeAndCharacter].difficultyClearedWithRetries[g_GameManager.difficulty] = 99;
    }
    else
    {
        if (g_GameManager.clrd[shotTypeAndCharacter].difficultyClearedWithoutRetries[g_GameManager.difficulty] == 99)
        {
            ending->hasSeenEnding = true;
        }
    }
    g_GameManager.clrd[shotTypeAndCharacter].difficultyClearedWithoutRetries[g_GameManager.difficulty] = 99;
    if (g_GameManager.difficulty == EASY || g_GameManager.numRetries != 0)
    {
        switch (g_GameManager.character)
        {
        case CHARA_REIMU:
            if (ending->LoadEnding("data/end00b.end") != ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            break;
        case CHARA_MARISA:
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
        case CHARA_REIMU:
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
        case CHARA_MARISA:
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

ZunResult Ending::DeletedCallback(Ending *ending)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_STAFF01);
    g_AnmManager->ReleaseAnm(ANM_FILE_STAFF02);
    g_AnmManager->ReleaseAnm(ANM_FILE_STAFF03);

    g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;

    g_AnmManager->ReleaseSurface(0);

    // This has the same effect as doing "delete ending->endFileData" since delete just calls free, but for some reason,
    // in both ways, the stack doesn't match with the other variable used in delete ending, in theory this should should
    // be correct since ending->endFileData was allocated with malloc. One way to solve it, would be to do the same with
    // ending, and align both variables with var_order, but that would be "incorrect", weird...
    char *endfiledata = ending->endFileData;
    free(endfiledata);

    g_Chain.Cut(ending->drawChain);
    ending->drawChain = NULL;

    delete ending;
    ending = NULL;

    g_Supervisor.isInEnding = false;
    g_Supervisor.ReleasePbg3(ED_PBG3_INDEX);
    return ZUN_SUCCESS;
}
}; // namespace th06

#include "Gui.hpp"

#include <stdio.h>

#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "FileSystem.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "ZunColor.hpp"
#include "utils.hpp"

namespace th06
{
DIFFABLE_STATIC(Gui, g_Gui);
DIFFABLE_STATIC(ChainElem, g_GuiCalcChain);
DIFFABLE_STATIC(ChainElem, g_GuiDrawChain);

ZunBool Gui::IsStageFinished()
{
    return this->impl->loadingScreenSprite.activeSpriteIndex >= 0 && this->impl->loadingScreenSprite.flags.isStopped;
}

void Gui::EndPlayerSpellcard()
{
    (this->impl->bombSpellcardName).pendingInterrupt = 1;
}

void Gui::EndEnemySpellcard()
{
    this->impl->enemySpellcardName.pendingInterrupt = 1;
    return;
}

ZunBool Gui::IsDialogueSkippable()
{
    return (this->impl->msg).dialogueSkippable;
}

void Gui::ShowBonusScore(u32 bonusScore)
{
    this->impl->bonusScore.pos = D3DXVECTOR3(416.0f, 32.0f, 0.0f);
    this->impl->bonusScore.isShown = 1;
    this->impl->bonusScore.timer.InitializeForPopup();
    this->impl->bonusScore.fmtArg = bonusScore;
    return;
}

void Gui::ShowFullPowerMode(i32 fmtArg)
{
    this->impl->fullPowerMode.pos = D3DXVECTOR3(416.0f, 232.0f, 0.0f);
    this->impl->fullPowerMode.isShown = 1;
    this->impl->fullPowerMode.timer.InitializeForPopup();
    this->impl->fullPowerMode.fmtArg = fmtArg;
    return;
}

void Gui::ShowSpellcardBonus(u32 spellcardScore)
{
    this->impl->spellCardBonus.pos = D3DXVECTOR3(224.0f, 16.0f, 0.0f);
    this->impl->spellCardBonus.isShown = 1;
    this->impl->spellCardBonus.timer.InitializeForPopup();
    this->impl->spellCardBonus.fmtArg = spellcardScore;
    return;
}

ChainCallbackResult Gui::OnUpdate(Gui *gui)
{
    if (g_GameManager.isTimeStopped)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    gui->UpdateStageElements();
    gui->impl->RunMsg();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult Gui::OnDraw(Gui *gui)
{
    char spellCardBonusStr[32];
    D3DXVECTOR3 stringPos;

    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (gui->impl->finishedStage)
    {
        stringPos.x = GAME_REGION_LEFT + 42.0f;
        stringPos.y = GAME_REGION_TOP + 112.0f;
        stringPos.z = 0.0;
        g_AsciiManager.color = COLOR_SUNSHINEYELLOW;
        if (g_GameManager.currentStage < EXTRA_STAGE)
        {
            g_AsciiManager.AddFormatText(&stringPos, "Stage Clear\n\n");
        }
        else
        {
            g_AsciiManager.AddFormatText(&stringPos, "All Clear!\n\n");
        }

        stringPos.y += 32.0f;
        g_AsciiManager.color = COLOR_WHITE;
        g_AsciiManager.AddFormatText(&stringPos, "Stage * 1000 = %5d\n", g_GameManager.currentStage * 1000);

        stringPos.y += 16.0f;
        g_AsciiManager.color = COLOR_LAVENDER;
        g_AsciiManager.AddFormatText(&stringPos, "Power *  100 = %5d\n", g_GameManager.currentPower * 100);

        stringPos.y += 16.0f;
        g_AsciiManager.color = COLOR_LIGHTBLUE;
        g_AsciiManager.AddFormatText(&stringPos, "Graze *   10 = %5d\n", g_GameManager.grazeInStage * 10);

        stringPos.y += 16.0f;
        g_AsciiManager.color = COLOR_LIGHT_RED;
        g_AsciiManager.AddFormatText(&stringPos, "    * Point Item %3d\n", g_GameManager.pointItemsCollectedInStage);

        if (EXTRA_STAGE <= g_GameManager.currentStage)
        {
            stringPos.y += 16.0f;
            g_AsciiManager.color = COLOR_LIGHTYELLOW;
            g_AsciiManager.AddFormatText(&stringPos, "Player    = %8d\n", g_GameManager.livesRemaining * 3000000);
            stringPos.y += 16.0f;
            g_AsciiManager.AddFormatText(&stringPos, "Bomb      = %8d\n", g_GameManager.bombsRemaining * 1000000);
        }

        stringPos.y += 32.0f;
        switch (g_GameManager.difficulty)
        {
        case EASY:
            g_AsciiManager.color = COLOR_LIGHT_RED;
            g_AsciiManager.AddFormatText(&stringPos, "Easy Rank      * 0.5\n");
            break;
        case NORMAL:
            g_AsciiManager.color = COLOR_LIGHT_RED;
            g_AsciiManager.AddFormatText(&stringPos, "Normal Rank    * 1.0\n");
            break;
        case HARD:
            g_AsciiManager.color = COLOR_LIGHT_RED;
            g_AsciiManager.AddFormatText(&stringPos, "Hard Rank      * 1.2\n");
            break;
        case LUNATIC:
            g_AsciiManager.color = COLOR_LIGHT_RED;
            g_AsciiManager.AddFormatText(&stringPos, "Lunatic Rank   * 1.5\n");
            break;
        case EXTRA:
            g_AsciiManager.color = COLOR_LIGHT_RED;
            g_AsciiManager.AddFormatText(&stringPos, "Extra Rank     * 2.0\n");
            break;
        }

        stringPos.y += 16.0f;
        if (g_GameManager.difficulty < EXTRA && !g_GameManager.isInPracticeMode)
        {
            switch (g_Supervisor.defaultConfig.lifeCount)
            {
            case 3:
                g_AsciiManager.color = COLOR_LIGHT_RED;
                g_AsciiManager.AddFormatText(&stringPos, "Player Penalty * 0.5\n");
                stringPos.y += 16.0f;
                break;
            case 4:
                g_AsciiManager.color = COLOR_LIGHT_RED;
                g_AsciiManager.AddFormatText(&stringPos, "Player Penalty * 0.2\n");
                stringPos.y += 16.0f;
                break;
            }
        }
        g_AsciiManager.color = COLOR_WHITE;
        g_AsciiManager.AddFormatText(&stringPos, "Total     = %8d", gui->impl->stageScore);
        g_AsciiManager.color = COLOR_WHITE;
    }

    gui->impl->DrawDialogue();
    gui->DrawStageElements();
    gui->DrawGameScene();
    g_AsciiManager.isGui = 1;
    if (gui->impl->bonusScore.isShown)
    {
        g_AsciiManager.color = COLOR_LIGHTYELLOW;
        g_AsciiManager.AddFormatText(&gui->impl->bonusScore.pos, "BONUS %8d", gui->impl->bonusScore.fmtArg);
        g_AsciiManager.color = COLOR_WHITE;
    }
    if (gui->impl->fullPowerMode.isShown)
    {
        g_AsciiManager.color = COLOR_PALEBLUE;
        g_AsciiManager.AddFormatText(&gui->impl->fullPowerMode.pos, "Full Power Mode!!",
                                     gui->impl->fullPowerMode.fmtArg);
        g_AsciiManager.color = COLOR_WHITE;
    }
    if (gui->impl->spellCardBonus.isShown)
    {
        g_AsciiManager.color = COLOR_RED;

        gui->impl->spellCardBonus.pos.x =
            ((f32)GAME_REGION_WIDTH - (f32)strlen("Spell Card Bonus!") * 16.0f) / 2.0f + (f32)GAME_REGION_LEFT;
        gui->impl->spellCardBonus.pos.y = GAME_REGION_TOP + 64.0f;
        g_AsciiManager.AddFormatText(&gui->impl->spellCardBonus.pos, "Spell Card Bonus!");

        gui->impl->spellCardBonus.pos.y += 16.0f;
        sprintf(spellCardBonusStr, "+%d", gui->impl->spellCardBonus.fmtArg);
        gui->impl->spellCardBonus.pos.x =
            ((f32)GAME_REGION_WIDTH - (f32)strlen(spellCardBonusStr) * 32.0f) / 2.0f + (f32)GAME_REGION_LEFT;
        g_AsciiManager.scale.x = 2.0f;
        g_AsciiManager.scale.y = 2.0f;
        g_AsciiManager.color = COLOR_LIGHT_RED;
        g_AsciiManager.AddString(&gui->impl->spellCardBonus.pos, spellCardBonusStr);

        g_AsciiManager.scale.x = 1.0;
        g_AsciiManager.scale.y = 1.0;
        g_AsciiManager.color = COLOR_WHITE;
    }
    g_AsciiManager.isGui = 0;
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void Gui::ShowBombNamePortrait(u32 sprite, char *bombName)
{
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->playerSpellcardPortrait, 0x4a1);
    g_AnmManager->SetActiveSprite(&this->impl->playerSpellcardPortrait, sprite);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->bombSpellcardName, 0x706);
    g_AnmManager->DrawVmTextFmt(g_AnmManager, &this->impl->bombSpellcardName, 0xf0f0ff, 0x0, bombName);
    this->bombSpellcardBarLength = strlen(bombName) * 0xf / 2.0f + 16;
    g_Supervisor.unk198 = 3;
    g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB, 0);
}

void Gui::ShowSpellcard(i32 spellcardSprite, char *spellcardName)
{
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->enemySpellcardPortrait, ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT);
    g_AnmManager->SetActiveSprite(&this->impl->enemySpellcardPortrait, ANM_SPRITE_FACE_STAGE_START + spellcardSprite);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->enemySpellcardName, ANM_SCRIPT_TEXT_ENEMY_SPELLCARD_NAME);
    AnmManager::DrawStringFormat(g_AnmManager, &this->impl->enemySpellcardName, 0xfff0f0, COLOR_RGB(COLOR_BLACK),
                                 spellcardName);
    this->blueSpellcardBarLength = strlen(spellcardName) * 15 / 2.0f + 16.0f;
    g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB, 0);
    return;
}

ZunResult Gui::ActualAddedCallback()
{
    i32 idx;

    if ((i32)(g_Supervisor.curState != SUPERVISOR_STATE_GAMEMANAGER_REINIT))
    {
        memset(this->impl, 0, sizeof(GuiImpl));
        if (g_AnmManager->LoadAnm(ANM_FILE_FRONT, "data/front.anm", ANM_OFFSET_FRONT) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_LOADING, "data/loading.anm", ANM_OFFSET_LOADING) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        this->impl->loadingScreenSprite.activeSpriteIndex = -1;
        switch (g_GameManager.character)
        {
        case CHARA_REIMU:
            if (g_AnmManager->LoadAnm(ANM_FILE_FACE_CHARA_A, "data/face00a.anm", ANM_OFFSET_FACE_CHARA_A) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            if (g_AnmManager->LoadAnm(ANM_FILE_FACE_CHARA_B, "data/face00b.anm", ANM_OFFSET_FACE_CHARA_B) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            if (g_AnmManager->LoadAnm(ANM_FILE_FACE_CHARA_C, "data/face00c.anm", ANM_OFFSET_FACE_CHARA_C) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            break;
        case CHARA_MARISA:
            if (g_AnmManager->LoadAnm(ANM_FILE_FACE_CHARA_A, "data/face01a.anm", ANM_OFFSET_FACE_CHARA_A) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            if (g_AnmManager->LoadAnm(ANM_FILE_FACE_CHARA_B, "data/face01b.anm", ANM_OFFSET_FACE_CHARA_B) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            if (g_AnmManager->LoadAnm(ANM_FILE_FACE_CHARA_C, "data/face01c.anm", ANM_OFFSET_FACE_CHARA_C) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
            break;
        }
    }
    else
    {
        g_AnmManager->SetAndExecuteScriptIdx(&this->impl->loadingScreenSprite, ANM_SCRIPT_LOADING_SHOW_LOADING_SCREEN);
        this->impl->loadingScreenSprite.pendingInterrupt = 1;
    }
    switch (g_GameManager.currentStage)
    {
    case 1:
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_A, "data/face03a.anm", ANM_OFFSET_FACE_STAGE_A) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_B, "data/face03b.anm", ANM_OFFSET_FACE_STAGE_B) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (this->LoadMsg("data/msg1.dat") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case 2:
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_A, "data/face05a.anm", ANM_OFFSET_FACE_STAGE_A) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (this->LoadMsg("data/msg2.dat") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case 3:
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_A, "data/face06a.anm", ANM_OFFSET_FACE_STAGE_A) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_B, "data/face06b.anm", ANM_OFFSET_FACE_STAGE_B) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (this->LoadMsg("data/msg3.dat") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case 4:
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_A, "data/face08a.anm", ANM_OFFSET_FACE_STAGE_A) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_B, "data/face08b.anm", ANM_OFFSET_FACE_STAGE_B) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (this->LoadMsg("data/msg4.dat") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case 5:
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_A, "data/face09a.anm", ANM_OFFSET_FACE_STAGE_A) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_B, "data/face09b.anm", ANM_OFFSET_FACE_STAGE_B) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (this->LoadMsg("data/msg5.dat") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    case 6:
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_A, "data/face09b.anm", ANM_OFFSET_FACE_STAGE_A) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_B, "data/face10a.anm", ANM_OFFSET_FACE_STAGE_B) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_C, "data/face10b.anm", ANM_OFFSET_FACE_STAGE_C) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (this->LoadMsg("data/msg6.dat") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    default:
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_A, "data/face08a.anm", ANM_OFFSET_FACE_STAGE_A) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_B, "data/face12a.anm", ANM_OFFSET_FACE_STAGE_B) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_C, "data/face12b.anm", ANM_OFFSET_FACE_STAGE_C) != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        if (this->LoadMsg("data/msg7.dat") != ZUN_SUCCESS)
        {
            return ZUN_ERROR;
        }
        break;
    }
    if ((i32)(g_Supervisor.curState != SUPERVISOR_STATE_GAMEMANAGER_REINIT))
    {
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->impl->vms); idx++)
        {
            g_AnmManager->SetAndExecuteScriptIdx(&this->impl->vms[idx], ANM_SCRIPT_FRONT_START + idx);
        }
    }
    this->bossPresent = false;
    this->impl->bossHealthBarState = 0;
    this->bossHealthBar1 = 0.0;
    this->bossHealthBar2 = 0.0;
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->playerSpellcardPortrait, ANM_SCRIPT_FACE_BOMB_PORTRAIT);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->enemySpellcardPortrait, ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->bombSpellcardName, ANM_SCRIPT_TEXT_BOMB_NAME);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->enemySpellcardName, ANM_SCRIPT_TEXT_ENEMY_SPELLCARD_NAME);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->bombSpellcardBackground, ANM_SCRIPT_FRONT_BOMB_NAME_BACKGROUND);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->enemySpellcardBackground,
                                         ANM_SCRIPT_FRONT_ENEMY_SPELLCARD_BACKGROUND);
    this->impl->playerSpellcardPortrait.currentInstruction = NULL;
    this->impl->bombSpellcardName.currentInstruction = NULL;
    this->impl->enemySpellcardPortrait.currentInstruction = NULL;
    this->impl->enemySpellcardName.currentInstruction = NULL;
    this->impl->playerSpellcardPortrait.flags.isVisible = 0;
    this->impl->bombSpellcardName.flags.isVisible = 0;
    this->impl->enemySpellcardPortrait.flags.isVisible = 0;
    this->impl->enemySpellcardName.flags.isVisible = 0;
    this->impl->bombSpellcardName.fontWidth = 15;
    this->impl->bombSpellcardName.fontHeight = 15;
    this->impl->enemySpellcardName.fontWidth = 15;
    this->impl->enemySpellcardName.fontHeight = 15;
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->stageNameSprite, ANM_SCRIPT_TEXT_STAGE_NAME);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->songNameSprite, ANM_SCRIPT_TEXT_SONG_NAME);
    AnmManager::DrawStringFormat2(g_AnmManager, &this->impl->stageNameSprite, COLOR_RGB(COLOR_LIGHTCYAN),
                                  COLOR_RGB(COLOR_BLACK), g_Stage.stdData->stageName);
    this->impl->songNameSprite.fontWidth = 16;
    this->impl->songNameSprite.fontHeight = 16;
    AnmManager::DrawStringFormat(g_AnmManager, &this->impl->songNameSprite, COLOR_RGB(COLOR_LIGHTCYAN),
                                 COLOR_RGB(COLOR_BLACK), TH_SONG_NAME, g_Stage.stdData->songNames[0]);
    this->impl->msg.currentMsgIdx = 0xffffffff;
    this->impl->finishedStage = 0;
    this->impl->bonusScore.isShown = 0;
    this->impl->fullPowerMode.isShown = 0;
    this->impl->spellCardBonus.isShown = 0;
    this->flags.flag0 = 2;
    this->flags.flag1 = 2;
    this->flags.flag3 = 2;
    this->flags.flag4 = 2;
    this->flags.flag2 = 2;
    return ZUN_SUCCESS;
}

ZunResult Gui::LoadMsg(char *path)
{
    i32 idx;

    this->FreeMsgFile();
    this->impl->msg.msgFile = (MsgRawHeader *)FileSystem::OpenPath(path, 0);
    if (this->impl->msg.msgFile == NULL)
    {
        GameErrorContext::Log(&g_GameErrorContext, TH_ERR_GUI_MSG_FILE_CORRUPTED, path);
        return ZUN_ERROR;
    }
    this->impl->msg.currentMsgIdx = 0xffffffff;
    this->impl->msg.currentInstr = NULL;
    for (idx = 0; idx < this->impl->msg.msgFile->numInstrs; idx++)
    {
        this->impl->msg.msgFile->instrs[idx] =
            (MsgRawInstr *)((i32)this->impl->msg.msgFile->instrs[idx] + (i32)this->impl->msg.msgFile);
    }
    return ZUN_SUCCESS;
}

void Gui::FreeMsgFile()
{
    MsgRawHeader *msg;
    if ((this->impl->msg).msgFile != NULL)
    {
        msg = (this->impl->msg).msgFile;
        free(msg);
        (this->impl->msg).msgFile = NULL;
    }
}

void Gui::MsgRead(i32 msgIdx)
{
    this->impl->MsgRead(msgIdx);
    g_Supervisor.unk198 = 3;
    return;
}

void GuiImpl::MsgRead(i32 msgIdx)
{
    MsgRawHeader *msgFile;

    if (this->msg.msgFile->numInstrs <= msgIdx)
    {
        return;
    }
    msgFile = this->msg.msgFile;
    memset(&this->msg, 0, sizeof(GuiMsgVm));
    this->msg.currentMsgIdx = msgIdx;
    this->msg.msgFile = msgFile;
    this->msg.currentInstr = this->msg.msgFile->instrs[msgIdx];
    this->msg.dialogueLines[0].anmFileIndex = -1;
    this->msg.dialogueLines[1].anmFileIndex = -1;
    this->msg.fontSize = 15;
    this->msg.textColorsA[0] = COLOR_RGB(COLOR_GUI_1);
    this->msg.textColorsA[1] = COLOR_RGB(COLOR_GUI_2);
    this->msg.textColorsB[0] = 0;
    this->msg.textColorsB[1] = 0;
    this->msg.dialogueSkippable = 1;
    if (g_GameManager.currentStage == 6 && (msgIdx == 0 || msgIdx == 10))
    {
        g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff06.anm", ANM_OFFSET_EFFECTS);
    }
    else if (g_GameManager.currentStage == 7 && (msgIdx == 0 || msgIdx == 10))
    {
        g_AnmManager->LoadAnm(ANM_FILE_EFFECTS, "data/eff07.anm", ANM_OFFSET_EFFECTS);
        g_AnmManager->LoadAnm(ANM_FILE_FACE_STAGE_A, "data/face12c.anm", ANM_OFFSET_FACE_STAGE_A);
    }
    return;
}

ZunResult GuiImpl::RunMsg()
{
    MsgRawInstrArgs *args;

    if (this->msg.currentMsgIdx < 0)
    {
        return ZUN_ERROR;
    }
    if (this->msg.ignoreWaitCounter > 0)
    {
        this->msg.ignoreWaitCounter--;
    }
    if (this->msg.dialogueSkippable && IS_PRESSED(TH_BUTTON_SKIP))
    {
        this->msg.timer.SetCurrent(this->msg.currentInstr->time);
    }
    while ((i32)(this->msg.timer.current >= this->msg.currentInstr->time))
    {
        switch (this->msg.currentInstr->opcode)
        {
        case MSG_OPCODE_MSGDELETE:
            this->msg.currentMsgIdx = 0xffffffff;
            return ZUN_ERROR;
        case MSG_OPCODE_PORTRAITANMSCRIPT:
            args = &this->msg.currentInstr->args;
            g_AnmManager->SetAndExecuteScriptIdx(
                &this->msg.portraits[args->portraitAnmScript.portraitIdx],
                args->portraitAnmScript.anmScriptIdx +
                    (args->portraitAnmScript.portraitIdx == 0 ? ANM_SCRIPT_FACE_START : ANM_SCRIPT_FACE_START + 2));
            break;
        case MSG_OPCODE_PORTRAITANMSPRITE:
            args = &this->msg.currentInstr->args;
            g_AnmManager->SetActiveSprite(
                &this->msg.portraits[args->portraitAnmScript.portraitIdx],
                args->portraitAnmScript.anmScriptIdx +
                    (args->portraitAnmScript.portraitIdx == 0 ? ANM_SCRIPT_FACE_START : ANM_SCRIPT_FACE_START + 8));
            break;
        case MSG_OPCODE_TEXTDIALOGUE:
            args = &this->msg.currentInstr->args;
            if (args->text.textLine == 0 && 0 <= this->msg.dialogueLines[1].anmFileIndex)
            {
                AnmManager::DrawVmTextFmt(g_AnmManager, &this->msg.dialogueLines[1],
                                          this->msg.textColorsA[args->text.textColor],
                                          this->msg.textColorsB[args->text.textColor], " ");
            }
            g_AnmManager->SetAndExecuteScriptIdx(&this->msg.dialogueLines[args->text.textLine],
                                                 0x702 + args->text.textLine);
            this->msg.dialogueLines[args->text.textLine].fontWidth =
                this->msg.dialogueLines[args->text.textLine].fontHeight = this->msg.fontSize;
            AnmManager::DrawVmTextFmt(g_AnmManager, &this->msg.dialogueLines[args->text.textLine],
                                      this->msg.textColorsA[args->text.textColor],
                                      this->msg.textColorsB[args->text.textColor], args->text.text);
            this->msg.framesElapsedDuringPause = 0;
            break;
        case MSG_OPCODE_WAIT:
            if (!this->msg.dialogueSkippable || !IS_PRESSED(TH_BUTTON_SKIP))
            {
                if (!WAS_PRESSED(TH_BUTTON_SHOOT) || this->msg.framesElapsedDuringPause < 8)
                {
                    if (this->msg.framesElapsedDuringPause >= this->msg.currentInstr->args.wait)
                    {
                        break;
                    }
                    this->msg.framesElapsedDuringPause += 1;
                    goto SKIP_TIME_INCREMENT;
                }
            }
            break;
        case MSG_OPCODE_ANMINTERRUPT:
            args = &this->msg.currentInstr->args;
            if (args->anmInterrupt.unk1 < 2)
            {
                this->msg.portraits[args->anmInterrupt.unk1].pendingInterrupt = args->anmInterrupt.unk2;
            }
            else
            {
                this->msg.dialogueLines[args->anmInterrupt.unk1 - 2].pendingInterrupt = args->anmInterrupt.unk2;
            }
            break;
        case MSG_OPCODE_ECLRESUME:
            this->msg.ignoreWaitCounter += 1;
            break;
        case MSG_OPCODE_MUSIC:
            g_AnmManager->SetAndExecuteScriptIdx(&this->songNameSprite, 0x701);
            this->songNameSprite.fontWidth = 16;
            this->songNameSprite.fontHeight = 16;
            AnmManager::DrawStringFormat(g_AnmManager, &this->songNameSprite, COLOR_RGB(COLOR_LIGHTCYAN),
                                         COLOR_RGB(COLOR_BLACK), "♪%s",
                                         g_Stage.stdData->songNames[this->msg.currentInstr->args.music]);
            if (g_Supervisor.PlayMidiFile(this->msg.currentInstr->args.music) != 0)
            {
                g_Supervisor.PlayAudio(g_Stage.stdData->songPaths[this->msg.currentInstr->args.music]);
            }
            break;
        case MSG_OPCODE_TEXTINTRO:
            args = &this->msg.currentInstr->args;
            g_AnmManager->SetAndExecuteScriptIdx(&this->msg.introLines[args->text.textLine],
                                                 args->text.textLine + 0x704);
            AnmManager::DrawStringFormat(g_AnmManager, &this->msg.introLines[args->text.textLine],
                                         this->msg.textColorsA[args->text.textColor],
                                         this->msg.textColorsB[args->text.textColor], args->text.text);
            this->msg.framesElapsedDuringPause = 0;
            break;
        case MSG_OPCODE_STAGERESULTS:
            this->finishedStage = 1;
            if (g_GameManager.currentStage < 6)
            {
                g_AnmManager->SetAndExecuteScriptIdx(&this->loadingScreenSprite,
                                                     ANM_SCRIPT_LOADING_SHOW_LOADING_SCREEN);
            }
            else
            {
                g_GameManager.extraLives = 0xff;
            }
            break;
        case MSG_OPCODE_MSGHALT:
            goto SKIP_TIME_INCREMENT;
        case MSG_OPCODE_MUSICFADEOUT:
            g_Supervisor.FadeOutMusic(4.0);
            break;
        case MSG_OPCODE_STAGEEND:
            g_GameManager.guiScore = g_GameManager.score;
            if (g_GameManager.isInPracticeMode)
            {
                g_GameManager.guiScore = g_GameManager.score;
                g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
                goto SKIP_TIME_INCREMENT;
            }
            if (g_GameManager.currentStage < 5 || (g_GameManager.difficulty != EASY && g_GameManager.currentStage == 5))
            {
                g_Supervisor.curState = SUPERVISOR_STATE_GAMEMANAGER_REINIT;
            }
            else if (!g_GameManager.isInReplay)
            {
                if (g_GameManager.difficulty == EXTRA)
                {
                    g_GameManager.isGameCompleted = 1;
                    g_GameManager.guiScore = g_GameManager.score;
                    g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
                    goto SKIP_TIME_INCREMENT;
                }
                else
                {
                    g_Supervisor.curState = SUPERVISOR_STATE_ENDING;
                }
            }
            else
            {
                g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU_REPLAY;
            }
            goto SKIP_TIME_INCREMENT;
        case MSG_OPCODE_WAITSKIPPABLE:
            this->msg.dialogueSkippable = this->msg.currentInstr->args.dialogueSkippable;
            break;
        }
        this->msg.currentInstr =
            (MsgRawInstr *)(((i32) & this->msg.currentInstr->args) + this->msg.currentInstr->argSize);
    }
    this->msg.timer.NextTick();
SKIP_TIME_INCREMENT:
    g_AnmManager->ExecuteScript(&this->msg.portraits[0]);
    g_AnmManager->ExecuteScript(&this->msg.portraits[1]);
    g_AnmManager->ExecuteScript(&this->msg.dialogueLines[0]);
    g_AnmManager->ExecuteScript(&this->msg.dialogueLines[1]);
    g_AnmManager->ExecuteScript(&this->msg.introLines[0]);
    g_AnmManager->ExecuteScript(&this->msg.introLines[1]);
    if ((i32)(this->msg.timer.current < 60) && this->msg.dialogueSkippable && IS_PRESSED(TH_BUTTON_SKIP))
    {
        this->msg.timer.SetCurrent(60);
    }
    return ZUN_SUCCESS;
}

#pragma var_order(dialogueBoxHeight, vertices)
ZunResult GuiImpl::DrawDialogue()
{
    f32 dialogueBoxHeight;

    if (this->msg.currentMsgIdx < 0)
    {
        return ZUN_ERROR;
    }
    if (g_GameManager.currentStage == 6 && (this->msg.currentMsgIdx == 1 || this->msg.currentMsgIdx == 11))
    {
        return ZUN_SUCCESS;
    }
    if ((i32)(this->msg.timer.current < 60))
    {
        dialogueBoxHeight = this->msg.timer.AsFramesFloat() * 48.0f / 60.0f;
    }
    else
    {
        dialogueBoxHeight = 48.0f;
    }
    VertexDiffuseXyzrwh vertices[4];
    // Probably not what Zun wrote, but I don't like Zun's design. My guess is
    // Zun made a separate vertex structure with a D3DXVECTOR3 for the xyz, a
    // separate f32 for the w, and a D3DCOLOR for the diffuse. This kinda makes
    // no sense though - the position is a D3DXVECTOR4.
    memcpy(&vertices[0].position,
           &D3DXVECTOR3(g_GameManager.arcadeRegionTopLeftPos.x + (g_GameManager.arcadeRegionSize.x - 256.0f) / 2.0f -
                            16.0f,
                        384.0f, 0.0f),
           sizeof(D3DXVECTOR3));

    memcpy(&vertices[1].position,
           &D3DXVECTOR3(g_GameManager.arcadeRegionTopLeftPos.x + (g_GameManager.arcadeRegionSize.x - 256.0f) / 2.0f +
                            256.0f + 16.0f,
                        384.0f, 0.0f),
           sizeof(D3DXVECTOR3));

    memcpy(&vertices[2].position,
           &D3DXVECTOR3(g_GameManager.arcadeRegionTopLeftPos.x + (g_GameManager.arcadeRegionSize.x - 256.0f) / 2.0f -
                            16.0f,
                        384.0f + dialogueBoxHeight, 0.0f),
           sizeof(D3DXVECTOR3));

    memcpy(&vertices[3].position,
           &D3DXVECTOR3(g_GameManager.arcadeRegionTopLeftPos.x + (g_GameManager.arcadeRegionSize.x - 256.0f) / 2.0f +
                            256.0f + 16.0f,
                        384.0f + dialogueBoxHeight, 0.0f),
           sizeof(D3DXVECTOR3));

    vertices[0].diffuse = vertices[1].diffuse = 0xd0000000;
    vertices[2].diffuse = vertices[3].diffuse = 0x90000000;
    vertices[0].position.w = vertices[1].position.w = vertices[2].position.w = vertices[3].position.w = 1.0f;
    g_AnmManager->DrawNoRotation(&this->msg.portraits[0]);
    g_AnmManager->DrawNoRotation(&this->msg.portraits[1]);
    if (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 1) == 0)
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (((g_Supervisor.cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST) & 1) == 0)
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0);
    }
    g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
    g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
    g_AnmManager->SetCurrentVertexShader(0xff);
    g_AnmManager->SetCurrentColorOp(0xff);
    g_AnmManager->SetCurrentBlendMode(0xff);
    g_AnmManager->SetCurrentZWriteDisable(0xff);
    if (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 1) == 0)
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
    g_AnmManager->DrawNoRotation(&this->msg.dialogueLines[0]);
    g_AnmManager->DrawNoRotation(&this->msg.dialogueLines[1]);
    g_AnmManager->DrawNoRotation(&this->msg.introLines[0]);
    g_AnmManager->DrawNoRotation(&this->msg.introLines[1]);
    return ZUN_SUCCESS;
}

BOOL Gui::MsgWait()
{
    if (this->impl->msg.ignoreWaitCounter > 0)
    {
        return FALSE;
    }
    return 0 <= this->impl->msg.currentMsgIdx;
}

BOOL Gui::HasCurrentMsgIdx()
{
    return 0 <= this->impl->msg.currentMsgIdx;
}

#pragma var_order(idx, stageScore)
void Gui::UpdateStageElements()
{
    i32 stageScore;
    i32 idx;

    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->impl->vms); idx++)
    {
        if (idx == 19 && this->impl->msg.currentMsgIdx < 0)
        {
            if (this->bossPresent)
            {
                if (!this->impl->bossHealthBarState)
                {
                    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->vms[idx], ANM_SCRIPT_FRONT_ENEMY_TEXT);
                    this->impl->bossHealthBarState = 1;
                    this->bossUIOpacity = 0;
                }
                else
                {
                    if (g_AnmManager->ExecuteScript(&this->impl->vms[idx]))
                    {
                        this->impl->bossHealthBarState = 2;
                    }
                    if (this->bossUIOpacity < 256 - 4)
                    {
                        this->bossUIOpacity += 4;
                    }
                    else
                    {
                        this->bossUIOpacity = 0xff;
                    }
                }
            }
            else if (this->impl->bossHealthBarState != 0)
            {
                if (this->impl->bossHealthBarState <= 2)
                {
                    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->vms[idx], ANM_SCRIPT_FRONT_ENEMY_TEXT2);
                    this->impl->bossHealthBarState = 3;
                }
                if (this->bossUIOpacity > 0)
                {
                    this->bossUIOpacity -= 4;
                }
                else
                {
                    this->bossUIOpacity = 0;
                }
                if (g_AnmManager->ExecuteScript(&this->impl->vms[idx]))
                {
                    this->impl->bossHealthBarState = 0;
                    this->bossHealthBar2 = 0.0f;
                    this->bossUIOpacity = 0;
                }
            }
            if (2 <= this->impl->bossHealthBarState)
            {
                if (this->bossHealthBar1 > this->bossHealthBar2)
                {
                    this->bossHealthBar2 += 0.01f;
                    if (this->bossHealthBar1 < this->bossHealthBar2)
                    {
                        this->bossHealthBar2 = this->bossHealthBar1;
                    }
                }
                else if (this->bossHealthBar1 < this->bossHealthBar2)
                {
                    this->bossHealthBar2 -= 0.02f;
                    if (this->bossHealthBar1 > this->bossHealthBar2)
                    {
                        this->bossHealthBar2 = this->bossHealthBar1;
                    }
                }
            }
        }
        else
        {
            g_AnmManager->ExecuteScript(&this->impl->vms[idx]);
        }
    }
    g_AnmManager->ExecuteScript(&this->impl->stageNameSprite);
    g_AnmManager->ExecuteScript(&this->impl->songNameSprite);
    g_AnmManager->ExecuteScript(&this->impl->playerSpellcardPortrait);
    g_AnmManager->ExecuteScript(&this->impl->bombSpellcardName);
    g_AnmManager->ExecuteScript(&this->impl->enemySpellcardPortrait);
    g_AnmManager->ExecuteScript(&this->impl->enemySpellcardName);
    if (0 <= this->impl->loadingScreenSprite.activeSpriteIndex &&
        g_AnmManager->ExecuteScript(&this->impl->loadingScreenSprite) != 0)
    {
        this->impl->loadingScreenSprite.activeSpriteIndex = -1;
    }
    if (this->impl->bonusScore.isShown)
    {
        if ((i32)(this->impl->bonusScore.timer.current < 30))
        {
            this->impl->bonusScore.pos.x =
                (this->impl->bonusScore.timer.AsFramesFloat() * -312.0f / 30.0f) + (f32)GAME_REGION_RIGHT;
        }
        else
        {
            this->impl->bonusScore.pos.x = 104.0f;
        }
        if ((i32)(250 <= this->impl->bonusScore.timer.current))
        {
            this->impl->bonusScore.isShown = 0;
        }
        this->TickTimer(&this->impl->bonusScore.timer);
    }
    if (this->impl->fullPowerMode.isShown)
    {
        if ((i32)(this->impl->fullPowerMode.timer.current < 30))
        {
            this->impl->fullPowerMode.pos.x =
                (this->impl->fullPowerMode.timer.AsFramesFloat() * -312.0f / 30.0f) + (f32)GAME_REGION_RIGHT;
        }
        else
        {
            this->impl->fullPowerMode.pos.x = 104.0f;
        }
        if ((i32)(180 <= this->impl->fullPowerMode.timer.current))
        {
            this->impl->fullPowerMode.isShown = 0;
        }
        this->TickTimer(&this->impl->fullPowerMode.timer);
    }
    if (this->impl->spellCardBonus.isShown)
    {
        if ((i32)(280 <= this->impl->spellCardBonus.timer.current))
        {
            this->impl->spellCardBonus.isShown = 0;
        }
        this->TickTimer(&this->impl->spellCardBonus.timer);
    }
    if (this->impl->finishedStage == 1)
    {
        stageScore = 0;
        stageScore += g_GameManager.currentStage * 1000;
        stageScore += g_GameManager.grazeInStage * 10;
        stageScore += g_GameManager.currentPower * 100;
        stageScore *= g_GameManager.pointItemsCollectedInStage;
        if (6 <= g_GameManager.currentStage)
        {
            stageScore += g_GameManager.livesRemaining * 3000000;
            stageScore += g_GameManager.bombsRemaining * 1000000;
        }
        switch (g_GameManager.difficulty)
        {
        case EASY:
            stageScore /= 2;
            stageScore -= stageScore % 10;
            break;
        case HARD:
            stageScore = stageScore * 12 / 10;
            stageScore -= stageScore % 10;
            break;
        case LUNATIC:
            stageScore = stageScore * 15 / 10;
            stageScore -= stageScore % 10;
            break;
        case EXTRA:
            stageScore *= 2;
            stageScore -= stageScore % 10;
            break;
        }
        switch (g_Supervisor.defaultConfig.lifeCount)
        {
        case 3:
            stageScore = stageScore * 5 / 10;
            stageScore -= stageScore % 10;
            break;
        case 4:
            stageScore = stageScore * 2 / 10;
            stageScore -= stageScore % 10;
            break;
        }
        this->impl->stageScore = stageScore;
        g_GameManager.score += stageScore;
        this->impl->finishedStage += 1;
    }
    return;
}

static ZunColor COLOR1 = 0xa0d0ff;
static ZunColor COLOR2 = 0xa080ff;
static ZunColor COLOR3 = 0xe080c0;
static ZunColor COLOR4 = 0xff4040;

#pragma var_order(yPos, xPos, idx, vm)
void Gui::DrawGameScene()
{
    AnmVm *vm;
    i32 idx;
    f32 xPos;
    f32 yPos;

    if (this->impl->msg.currentMsgIdx < 0 && (this->bossPresent + this->impl->bossHealthBarState) > 0)
    {
#pragma var_order(cappedSpellcardSecondsRemaining, bossLivesColor, textPos)
        vm = &this->impl->vms[19];
        g_AnmManager->DrawNoRotation(vm);
        vm = &this->impl->vms[21];
        vm->flags.anchor = AnmVmAnchor_TopLeft;
        vm->scaleX = (this->bossHealthBar2 * 288.0f) / 14.0f;
        vm->pos.x = 96.0f;
        vm->pos.y = 24.0f;
        vm->pos.z = 0.0;
        g_AnmManager->DrawNoRotation(vm);
        D3DXVECTOR3 textPos(80.0f, 16.0f, 0.0);
        g_AsciiManager.SetColor(this->bossUIOpacity << 24 | 0xffff80);
        g_AsciiManager.AddFormatText(&textPos, "%d", this->eclSetLives);
        textPos = D3DXVECTOR3(384.0f, 16.0f, 0.0f);
        D3DCOLOR bossLivesColor;
        if (this->spellcardSecondsRemaining >= 20)
        {
            bossLivesColor = COLOR1;
        }
        else if (this->spellcardSecondsRemaining >= 10)
        {
            bossLivesColor = COLOR2;
        }
        else if (this->spellcardSecondsRemaining >= 5)
        {
            bossLivesColor = COLOR3;
        }
        else
        {
            bossLivesColor = COLOR4;
        }

        g_AsciiManager.SetColor(this->bossUIOpacity << 24 | bossLivesColor);
        i32 cappedSpellcardSecondsRemaining =
            this->spellcardSecondsRemaining > 99 ? 99 : this->spellcardSecondsRemaining;
        if (cappedSpellcardSecondsRemaining < 10 &&
            this->lastSpellcardSecondsRemaining != this->spellcardSecondsRemaining)
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_1D, 0);
        }
        g_AsciiManager.AddFormatText(&textPos, "%.2d", cappedSpellcardSecondsRemaining);
        g_AsciiManager.color = COLOR_WHITE;
        this->lastSpellcardSecondsRemaining = this->spellcardSecondsRemaining;
    }
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = 640;
    g_Supervisor.viewport.Height = 480;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    vm = &this->impl->vms[6];
    if (((g_Supervisor.cfg.opts >> GCOS_DISPLAY_MINIMUM_GRAPHICS) & 1) == 0 &&
        (vm->currentInstruction != NULL || g_Supervisor.unk198 != 0 || g_Supervisor.IsUnknown()))
    {
        for (yPos = 0.0f; yPos < 464.0f; yPos += 32.0f)
        {
            vm->pos = D3DXVECTOR3(0.0f, yPos, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        for (xPos = 416.0f; xPos < 624.0f; xPos += 32.0f)
        {
            for (yPos = 0.0f; yPos < 464.0f; yPos += 32.0f)
            {
                vm->pos = D3DXVECTOR3(xPos, yPos, 0.49f);
                g_AnmManager->DrawNoRotation(vm);
            }
        }
        vm = &this->impl->vms[7];
        for (xPos = 32.0f; xPos < 416.0f; xPos += 32.0f)
        {
            vm->pos = D3DXVECTOR3(xPos, 0.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        vm = &this->impl->vms[8];
        for (xPos = 32.0f; xPos < 416.0f; xPos += 32.0f)
        {
            vm->pos = D3DXVECTOR3(xPos, 464.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        g_AnmManager->Draw(&this->impl->vms[5]);
        g_AnmManager->Draw(&this->impl->vms[0]);
        g_AnmManager->Draw(&this->impl->vms[1]);
        g_AnmManager->Draw(&this->impl->vms[3]);
        g_AnmManager->Draw(&this->impl->vms[4]);
        g_AnmManager->Draw(&this->impl->vms[2]);
        g_AnmManager->DrawNoRotation(&this->impl->vms[9]);
        g_AnmManager->DrawNoRotation(&this->impl->vms[10]);
        g_AnmManager->DrawNoRotation(&this->impl->vms[11]);
        g_AnmManager->DrawNoRotation(&this->impl->vms[12]);
        g_AnmManager->DrawNoRotation(&this->impl->vms[13]);
        g_AnmManager->DrawNoRotation(&this->impl->vms[14]);
        g_AnmManager->DrawNoRotation(&this->impl->vms[15]);
        this->flags.flag0 = 2;
        this->flags.flag1 = 2;
        this->flags.flag3 = 2;
        this->flags.flag4 = 2;
        this->flags.flag2 = 2;
    }
    if ((g_Supervisor.cfg.opts >> GCOS_DISPLAY_MINIMUM_GRAPHICS & 1) == 0)
    {
        vm = &this->impl->vms[22];
        xPos = 496.0f;
        vm->pos = D3DXVECTOR3(xPos, 58.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
        vm->pos = D3DXVECTOR3(xPos, 82.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
        if (this->flags.flag0)
        {
            vm->pos = D3DXVECTOR3(xPos, 122.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.flag1)
        {
            vm->pos = D3DXVECTOR3(xPos, 146.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.flag2)
        {
            vm->pos = D3DXVECTOR3(xPos, 186.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.flag3)
        {
            vm->pos = D3DXVECTOR3(xPos, 206.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.flag4)
        {
            vm->pos = D3DXVECTOR3(xPos, 226.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        vm->pos = D3DXVECTOR3(488.0f, 464.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
        vm->pos = D3DXVECTOR3(0.0, 464.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
    }
    if (this->flags.flag0 || ((g_Supervisor.cfg.opts >> GCOS_DISPLAY_MINIMUM_GRAPHICS & 1) != 0))
    {
        vm = &this->impl->vms[16];
        for (idx = 0, xPos = 496.0f; idx < g_GameManager.livesRemaining; idx++, xPos += 16.0f)
        {
            vm->pos = D3DXVECTOR3(xPos, 122.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
    }
    if (this->flags.flag1 || ((g_Supervisor.cfg.opts >> GCOS_DISPLAY_MINIMUM_GRAPHICS & 1) != 0))
    {
        vm = &this->impl->vms[17];
        for (idx = 0, xPos = 496.0f; idx < g_GameManager.bombsRemaining; idx++, xPos += 16.0f)
        {
            vm->pos = D3DXVECTOR3(xPos, 146.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
    }
    if (this->flags.flag2 || ((g_Supervisor.cfg.opts >> GCOS_DISPLAY_MINIMUM_GRAPHICS & 1) != 0))
    {
        VertexDiffuseXyzrwh vertices[4];
        if (g_GameManager.currentPower > 0)
        {
            memcpy(&vertices[0].position, &D3DXVECTOR3(496.0f, 186.0f, 0.1f), sizeof(D3DXVECTOR3));
            memcpy(&vertices[1].position, &D3DXVECTOR3(g_GameManager.currentPower + 496 + 0.0f, 186.0f, 0.1f),
                   sizeof(D3DXVECTOR3));
            memcpy(&vertices[2].position, &D3DXVECTOR3(496.0f, 202.0f, 0.1f), sizeof(D3DXVECTOR3));
            memcpy(&vertices[3].position, &D3DXVECTOR3(g_GameManager.currentPower + 496 + 0.0f, 202.0f, 0.1f),
                   sizeof(D3DXVECTOR3));

            vertices[0].diffuse = vertices[2].diffuse = 0xe0e0e0ff;
            vertices[1].diffuse = vertices[3].diffuse = 0x80e0e0ff;

            vertices[0].position.w = vertices[1].position.w = vertices[2].position.w = vertices[3].position.w = 1.0;

            if ((g_Supervisor.cfg.opts >> 8 & 1) == 0)
            {
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            }
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            if ((g_Supervisor.cfg.opts >> GCOS_TURN_OFF_DEPTH_TEST & 1) == 0)
            {
                g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
                g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            }
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
            g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
            g_AnmManager->SetCurrentVertexShader(0xff);
            g_AnmManager->SetCurrentColorOp(0xff);
            g_AnmManager->SetCurrentBlendMode(0xff);
            g_AnmManager->SetCurrentZWriteDisable(0xff);
            if ((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP & 1) == 0)
            {
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            }
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            if (128 <= g_GameManager.currentPower)
            {
                vm = &this->impl->vms[18];
                vm->pos = D3DXVECTOR3(496.0f, 186.0f, 0.0f);
                g_AnmManager->DrawNoRotation(vm);
            }
        }
        if (g_GameManager.currentPower < 128)
        {
            g_AsciiManager.AddFormatText(&D3DXVECTOR3(496.0f, 186.0f, 0.0f), "%d", g_GameManager.currentPower);
        }
    }
    {
        D3DXVECTOR3 elemPos(496.0f, 82.0f, 0.0f);
        g_AsciiManager.AddFormatText(&elemPos, "%.9d", g_GameManager.guiScore);
        elemPos = D3DXVECTOR3(496.0f, 58.0f, 0.0f);
        g_AsciiManager.AddFormatText(&elemPos, "%.9d", g_GameManager.highScore);
        if (this->flags.flag3 || ((g_Supervisor.cfg.opts >> 4 & 1) != 0))
        {
            elemPos = D3DXVECTOR3(496.0f, 206.0f, 0.0f);
            g_AsciiManager.AddFormatText(&elemPos, "%d", g_GameManager.grazeInStage);
        }
        if (this->flags.flag4 || ((g_Supervisor.cfg.opts >> 4 & 1) != 0))
        {
            elemPos = D3DXVECTOR3(496.0f, 226.0f, 0.0f);
            g_AsciiManager.AddFormatText(&elemPos, "%d", g_GameManager.pointItemsCollectedInStage);
        }
    }
    if (this->flags.flag0)
    {
        this->flags.flag0--;
    }
    if (this->flags.flag2)
    {
        this->flags.flag2--;
    }
    if (this->flags.flag1)
    {
        this->flags.flag1--;
    }
    if (this->flags.flag3)
    {
        this->flags.flag3--;
    }
    if (this->flags.flag4)
    {
        this->flags.flag4--;
    }
    return;
}

#pragma var_order(stageTextPos, stageTextColor, demoTextColor)
void Gui::DrawStageElements()
{
    D3DXVECTOR3 stageTextPos;
    ZunColor stageTextColor;
    ZunColor demoTextColor;

    if (this->impl->stageNameSprite.flags.isVisible)
    {

        stageTextPos.x = 168.0f;
        stageTextPos.y = 198.0f;
        stageTextPos.z = 0.0f;
        if (!g_GameManager.demoMode)
        {
            g_AnmManager->Draw2(&this->impl->stageNameSprite);

            // this looks like an inline function, maybe ZunColor is a struct?
            stageTextColor = COLOR_COMBINE_ALPHA(COLOR_SUNSHINEYELLOW, this->impl->stageNameSprite.color);
            g_AsciiManager.color = stageTextColor;

            if (g_GameManager.currentStage < EXTRA_STAGE)
            {
                stageTextPos.x = 168.0f;
                g_AsciiManager.AddFormatText(&stageTextPos, "STAGE %d", g_GameManager.currentStage);
            }
            else if (g_GameManager.currentStage == EXTRA_STAGE)
            {
                stageTextPos.x = 136.0f;
                g_AsciiManager.AddFormatText(&stageTextPos, "FINAL STAGE");
            }
            else
            {
                stageTextPos.x = 136.0f;
                g_AsciiManager.AddFormatText(&stageTextPos, "EXTRA STAGE");
            }
        }
        else
        {
            demoTextColor = COLOR_COMBINE_ALPHA(COLOR_SUNSHINEYELLOW, this->impl->stageNameSprite.color);
            g_AsciiManager.color = demoTextColor;

            stageTextPos.x = 136.0f;

            g_AsciiManager.AddFormatText(&stageTextPos, " DEMO PLAY");
        }
        g_AsciiManager.color = COLOR_WHITE;
    }

    if (this->impl->songNameSprite.flags.isVisible && !g_GameManager.demoMode)
    {
        g_AnmManager->Draw2(&this->impl->songNameSprite);
    }
    if (this->impl->playerSpellcardPortrait.flags.isVisible)
    {
        g_AnmManager->DrawNoRotation(&this->impl->playerSpellcardPortrait);
    }
    if (this->impl->enemySpellcardPortrait.flags.isVisible)
    {
        g_AnmManager->DrawNoRotation(&this->impl->enemySpellcardPortrait);
    }

    if (this->impl->bombSpellcardName.flags.isVisible)
    {
        this->impl->bombSpellcardBackground.pos = this->impl->bombSpellcardName.pos;
        this->impl->bombSpellcardBackground.pos.x +=
            this->bombSpellcardBarLength * 16.0f / 15.0f / 2.0f + -128.0f - 16.0f;
        this->impl->bombSpellcardBackground.scaleX = this->bombSpellcardBarLength / 14.0f;
        g_AnmManager->DrawNoRotation(&this->impl->bombSpellcardBackground);
        g_AnmManager->DrawNoRotation(&this->impl->bombSpellcardName);
    }
    if (this->impl->enemySpellcardName.flags.isVisible)
    {

        this->impl->enemySpellcardBackground.pos = this->impl->enemySpellcardName.pos;
        this->impl->enemySpellcardBackground.pos.x += 128.0f - this->blueSpellcardBarLength * 16.0f / 15.0f / 2.0f;
        this->impl->enemySpellcardBackground.scaleX = this->blueSpellcardBarLength / 14.0f;
        g_AnmManager->DrawNoRotation(&this->impl->enemySpellcardBackground);
        g_AnmManager->DrawNoRotation(&this->impl->enemySpellcardName);
    }
    if (this->impl->loadingScreenSprite.activeSpriteIndex >= 0)
    {
        g_Supervisor.viewport.X = g_GameManager.arcadeRegionTopLeftPos.x;
        g_Supervisor.viewport.Y = g_GameManager.arcadeRegionTopLeftPos.y;

        g_Supervisor.viewport.Width = g_GameManager.arcadeRegionSize.x;
        g_Supervisor.viewport.Height = g_GameManager.arcadeRegionSize.y;

        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
        g_AnmManager->DrawNoRotation(&this->impl->loadingScreenSprite);
    }
}

#pragma optimize("s", on)
ZunResult Gui::AddedCallback(Gui *gui)
{
    return gui->ActualAddedCallback();
}
#pragma optimize("", on)

ZunResult Gui::DeletedCallback(Gui *gui)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_FACE_STAGE_A);
    g_AnmManager->ReleaseAnm(ANM_FILE_FACE_STAGE_B);
    g_AnmManager->ReleaseAnm(ANM_FILE_FACE_STAGE_C);
    gui->FreeMsgFile();
    if ((i32)(g_Supervisor.curState != SUPERVISOR_STATE_GAMEMANAGER_REINIT))
    {
        g_AnmManager->ReleaseAnm(ANM_FILE_FRONT);
        g_AnmManager->ReleaseAnm(ANM_FILE_LOADING);
        g_AnmManager->ReleaseAnm(ANM_FILE_FACE_CHARA_A);
        g_AnmManager->ReleaseAnm(ANM_FILE_FACE_CHARA_B);
        g_AnmManager->ReleaseAnm(ANM_FILE_FACE_CHARA_C);
        delete gui->impl;
        gui->impl = NULL;
    }
    return ZUN_SUCCESS;
}

ZunResult Gui::RegisterChain()
{
    Gui *gui = &g_Gui;
    if ((i32)(g_Supervisor.curState != SUPERVISOR_STATE_GAMEMANAGER_REINIT))
    {
        memset(gui, 0, sizeof(Gui));
        gui->impl = new GuiImpl();
    }
    g_GuiCalcChain.callback = (ChainCallback)Gui::OnUpdate;
    g_GuiCalcChain.addedCallback = NULL;
    g_GuiCalcChain.deletedCallback = NULL;
    g_GuiCalcChain.addedCallback = (ChainAddedCallback)Gui::AddedCallback;
    g_GuiCalcChain.deletedCallback = (ChainDeletedCallback)Gui::DeletedCallback;
    g_GuiCalcChain.arg = gui;
    if (g_Chain.AddToCalcChain(&g_GuiCalcChain, TH_CHAIN_PRIO_CALC_GUI) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    g_GuiDrawChain.callback = (ChainCallback)Gui::OnDraw;
    g_GuiDrawChain.addedCallback = NULL;
    g_GuiDrawChain.deletedCallback = NULL;
    g_GuiDrawChain.arg = gui;
    g_Chain.AddToDrawChain(&g_GuiDrawChain, TH_CHAIN_PRIO_DRAW_GUI);
    return ZUN_SUCCESS;
}

GuiImpl::GuiImpl() {

};

#pragma optimize("s", on)
void Gui::CutChain()
{
    g_Chain.Cut(&g_GuiCalcChain);
    g_Chain.Cut(&g_GuiDrawChain);
    return;
}
#pragma optimize("", on)
}; // namespace th06

#include "Gui.hpp"

#include <stdio.h>

#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "GameManager.hpp"
#include "GameWindow.hpp"
#include "Global.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "ZunColor.hpp"

namespace th06
{
enum MsgOpcode
{
    MSG_OPCODE_MSG_DELETE,
    MSG_OPCODE_PORTRAIT_ANM_SCRIPT,
    MSG_OPCODE_PORTRAIT_ANM_SPRITE,
    MSG_OPCODE_TEXT_DIALOGUE,
    MSG_OPCODE_WAIT,
    MSG_OPCODE_ANM_INTERRUPT,
    MSG_OPCODE_ECL_RESUME,
    MSG_OPCODE_MUSIC,
    MSG_OPCODE_TEXT_INTRO,
    MSG_OPCODE_STAGE_RESULTS,
    MSG_OPCODE_MSG_HALT,
    MSG_OPCODE_STAGE_END,
    MSG_OPCODE_MUSIC_FADE_OUT,
    MSG_OPCODE_MSG_FLAG_WAIT_SKIPPABLE,
};

struct MsgRawInstr
{
    u16 time;
    u8 opcode;
    u8 argSize;
    u8 args[];
};
ZUN_ASSERT_SIZE(MsgRawInstr, 0x4);

struct MsgRawHeader
{
    i32 numInstrs;
    MsgRawInstr *instrs[];
};
ZUN_ASSERT_SIZE(MsgRawHeader, 0x4);

#define MSG_PORTRAIT_COUNT 2
#define MSG_DIALOGUE_LINE_COUNT 2

struct GuiMsgVm
{
    MsgRawHeader *msgFile;
    MsgRawInstr *currentInstr;
    i32 currentMsgIdx;
    ZunTimer scriptTimer;
    i32 framesElapsedDuringPause;
    AnmVm portraits[MSG_PORTRAIT_COUNT];
    AnmVm dialogueLines[MSG_DIALOGUE_LINE_COUNT];
    AnmVm introLines[2];
    D3DCOLOR textColorsA[4];
    D3DCOLOR textColorsB[4];
    u32 fontSize;
    u32 ignoreWaitCounter;
    u8 dialogueSkippable;
    alignment_padding(0x3);
};
ZUN_ASSERT_TYPE(GuiMsgVm, 0x6a8, 4);

struct GuiFormattedText
{
    D3DXVECTOR3 pos;
    i32 fmtArg;
    ZunBool isShown;
    ZunTimer timer;
};
ZUN_ASSERT_TYPE(GuiFormattedText, 0x20, 4);

struct GuiImpl
{
    ZunResult RunMsg();
    ZunResult DrawDialogue();
    void MsgRead(i32 msgIdx);

    AnmVm vms[26];
    u8 bossHealthBarState;
    alignment_padding(0x3);
    AnmVm stageNameSprite;
    AnmVm songNameSprite;
    AnmVm playerSpellcardPortrait;
    AnmVm enemySpellcardPortrait;
    AnmVm bombSpellcardName;
    AnmVm enemySpellcardName;
    AnmVm bombSpellcardBackground;
    AnmVm enemySpellcardBackground;
    AnmVm loadingScreenSprite;
    GuiMsgVm msg;
    ZunBool finishedStage;
    u32 stageScore;
    GuiFormattedText bonusScore;
    GuiFormattedText fullPowerMode;
    GuiFormattedText spellCardBonus;
};
ZUN_ASSERT_TYPE(GuiImpl, 0x2c44, 4);

BSS_SORT(G1) Gui g_Gui;
BSS_SORT(G3) ChainElem g_GuiCalcChain;
BSS_SORT(G2) ChainElem g_GuiDrawChain;

ZunBool Gui::IsStageFinished()
{
    return this->impl->loadingScreenSprite.activeSpriteIndex >= 0 && this->impl->loadingScreenSprite.IsStopped();
}

void Gui::EndPlayerSpellcard()
{
    this->impl->bombSpellcardName.pendingInterrupt = 1;
}

void Gui::EndEnemySpellcard()
{
    this->impl->enemySpellcardName.pendingInterrupt = 1;
}

ZunBool Gui::IsDialogueSkippable()
{
    return this->impl->msg.dialogueSkippable;
}

void Gui::ShowBonusScore(u32 bonusScore)
{
    this->impl->bonusScore.pos = D3DXVECTOR3(416.0f, 32.0f, 0.0f);
    this->impl->bonusScore.isShown = true;
    this->impl->bonusScore.timer = 0;
    this->impl->bonusScore.fmtArg = bonusScore;
}

void Gui::ShowFullPowerMode(i32 fmtArg)
{
    this->impl->fullPowerMode.pos = D3DXVECTOR3(416.0f, 232.0f, 0.0f);
    this->impl->fullPowerMode.isShown = true;
    this->impl->fullPowerMode.timer = 0;
    this->impl->fullPowerMode.fmtArg = fmtArg;
}

void Gui::ShowSpellcardBonus(u32 spellcardScore)
{
    this->impl->spellCardBonus.pos = D3DXVECTOR3(224.0f, 16.0f, 0.0f);
    this->impl->spellCardBonus.isShown = true;
    this->impl->spellCardBonus.timer = 0;
    this->impl->spellCardBonus.fmtArg = spellcardScore;
}

ChainCallbackResult Gui_OnUpdate(Gui *gui)
{
    if (g_GameManager.isTimeStopped)
    {
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    gui->UpdateStageElements();
    gui->impl->RunMsg();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult Gui_OnDraw(Gui *gui)
{
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (gui->impl->finishedStage)
    {
        D3DXVECTOR3 stringPos(GAME_REGION_POS_X + 42.0f, GAME_REGION_POS_Y + 112.0f, 0.0f);
        g_AsciiManager.SetColor(COLOR_SUNSHINEYELLOW);
        if (g_GameManager.currentStage < EXTRA_STAGE)
        {
            g_AsciiManager.AddFormatText(&stringPos, "Stage Clear\n\n");
        }
        else
        {
            g_AsciiManager.AddFormatText(&stringPos, "All Clear!\n\n");
        }

        stringPos.y += 32.0f;
        g_AsciiManager.SetColor(COLOR_WHITE);
        g_AsciiManager.AddFormatText(&stringPos, "Stage * 1000 = %5d\n", g_GameManager.currentStage * 1000);

        stringPos.y += 16.0f;
        g_AsciiManager.SetColor(COLOR_LAVENDER);
        g_AsciiManager.AddFormatText(&stringPos, "Power *  100 = %5d\n", g_GameManager.currentPower * 100);

        stringPos.y += 16.0f;
        g_AsciiManager.SetColor(COLOR_LIGHTBLUE);
        g_AsciiManager.AddFormatText(&stringPos, "Graze *   10 = %5d\n", g_GameManager.grazeInStage * 10);

        stringPos.y += 16.0f;
        g_AsciiManager.SetColor(COLOR_LIGHT_RED);
        g_AsciiManager.AddFormatText(&stringPos, "    * Point Item %3d\n", g_GameManager.pointItemsCollectedInStage);

        if (EXTRA_STAGE <= g_GameManager.currentStage)
        {
            stringPos.y += 16.0f;
            g_AsciiManager.SetColor(COLOR_LIGHT_YELLOW);
            g_AsciiManager.AddFormatText(&stringPos, "Player    = %8d\n", g_GameManager.livesRemaining * 3000000);
            stringPos.y += 16.0f;
            g_AsciiManager.AddFormatText(&stringPos, "Bomb      = %8d\n", g_GameManager.bombsRemaining * 1000000);
        }

        stringPos.y += 32.0f;
        switch (g_GameManager.difficulty)
        {
        case EASY:
            g_AsciiManager.SetColor(COLOR_LIGHT_RED);
            g_AsciiManager.AddFormatText(&stringPos, "Easy Rank      * 0.5\n");
            break;
        case NORMAL:
            g_AsciiManager.SetColor(COLOR_LIGHT_RED);
            g_AsciiManager.AddFormatText(&stringPos, "Normal Rank    * 1.0\n");
            break;
        case HARD:
            g_AsciiManager.SetColor(COLOR_LIGHT_RED);
            g_AsciiManager.AddFormatText(&stringPos, "Hard Rank      * 1.2\n");
            break;
        case LUNATIC:
            g_AsciiManager.SetColor(COLOR_LIGHT_RED);
            g_AsciiManager.AddFormatText(&stringPos, "Lunatic Rank   * 1.5\n");
            break;
        case EXTRA:
            g_AsciiManager.SetColor(COLOR_LIGHT_RED);
            g_AsciiManager.AddFormatText(&stringPos, "Extra Rank     * 2.0\n");
            break;
        }

        stringPos.y += 16.0f;
        if (g_GameManager.difficulty < EXTRA && !g_GameManager.isInPracticeMode)
        {
#if BUILD_VERSION >= BUILD_VERSION_102h
            switch (g_Supervisor.defaultConfig.lifeCount)
#else
            switch (g_Supervisor.cfg.lifeCount)
#endif
            {
            case 3:
                g_AsciiManager.SetColor(COLOR_LIGHT_RED);
                g_AsciiManager.AddFormatText(&stringPos, "Player Penalty * 0.5\n");
                stringPos.y += 16.0f;
                break;
            case 4:
                g_AsciiManager.SetColor(COLOR_LIGHT_RED);
                g_AsciiManager.AddFormatText(&stringPos, "Player Penalty * 0.2\n");
                stringPos.y += 16.0f;
                break;
            }
        }
        g_AsciiManager.SetColor(COLOR_WHITE);
        g_AsciiManager.AddFormatText(&stringPos, "Total     = %8d", gui->impl->stageScore);
        g_AsciiManager.SetColor(COLOR_WHITE);
    }

    gui->impl->DrawDialogue();
    gui->DrawStageElements();
    gui->DrawGameScene();
    g_AsciiManager.SetIsGui(true);
    if (gui->impl->bonusScore.isShown)
    {
        g_AsciiManager.SetColor(COLOR_LIGHT_YELLOW);
        g_AsciiManager.AddFormatText(&gui->impl->bonusScore.pos, "BONUS %8d", gui->impl->bonusScore.fmtArg);
        g_AsciiManager.SetColor(COLOR_WHITE);
    }
    if (gui->impl->fullPowerMode.isShown)
    {
        g_AsciiManager.SetColor(COLOR_PALEBLUE);
        g_AsciiManager.AddFormatText(&gui->impl->fullPowerMode.pos, "Full Power Mode!!",
                                     gui->impl->fullPowerMode.fmtArg);
        g_AsciiManager.SetColor(COLOR_WHITE);
    }
    if (gui->impl->spellCardBonus.isShown)
    {
        g_AsciiManager.SetColor(COLOR_RED);

        gui->impl->spellCardBonus.pos.x =
            (GAME_REGION_WIDTH - (f32)strlen("Spell Card Bonus!") * 16.0f) / 2.0f + GAME_REGION_POS_X;
        gui->impl->spellCardBonus.pos.y = GAME_REGION_POS_Y + 64.0f;
        g_AsciiManager.AddFormatText(&gui->impl->spellCardBonus.pos, "Spell Card Bonus!");

        gui->impl->spellCardBonus.pos.y += 16.0f;
        char spellCardBonusStr[32];
        sprintf(spellCardBonusStr, "+%d", gui->impl->spellCardBonus.fmtArg);
        gui->impl->spellCardBonus.pos.x =
            (GAME_REGION_WIDTH - (f32)strlen(spellCardBonusStr) * 32.0f) / 2.0f + GAME_REGION_POS_X;
        g_AsciiManager.SetScale(2.0f, 2.0f);
        g_AsciiManager.SetColor(COLOR_LIGHT_RED);
        g_AsciiManager.AddString(&gui->impl->spellCardBonus.pos, spellCardBonusStr);

        g_AsciiManager.SetScale(1.0f, 1.0f);
        g_AsciiManager.SetColor(COLOR_WHITE);
    }
    g_AsciiManager.SetIsGui(false);
    g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void Gui::ShowBombNamePortrait(u32 sprite, const char *bombName)
{
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->playerSpellcardPortrait, ANM_SCRIPT_FACE_BOMB_PORTRAIT);
    g_AnmManager->SetActiveSprite(&this->impl->playerSpellcardPortrait, sprite);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->bombSpellcardName, ANM_SCRIPT_TEXT_BOMB_NAME);
    g_AnmManager->DrawVmTextFmt(&this->impl->bombSpellcardName, COLOR_RGB(COLOR_BARELY_BLUE), COLOR_RGB(COLOR_BLACK),
                                bombName);
    this->bombSpellcardBarLength = strlen(bombName) * 15 / 2.0f + 16.0f; // TODO: Is this 15 the font size?
#if !TRIALBUILD
    g_Supervisor.forceRedrawFrames = 3;
#endif
    g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB);
}

void Gui::ShowSpellcard(i32 spellcardSprite, const char *spellcardName)
{
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->enemySpellcardPortrait, ANM_SCRIPT_FACE_ENEMY_SPELLCARD_PORTRAIT);
    g_AnmManager->SetActiveSprite(&this->impl->enemySpellcardPortrait, ANM_SPRITE_FACE_STAGE_START + spellcardSprite);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->enemySpellcardName, ANM_SCRIPT_TEXT_ENEMY_SPELLCARD_NAME);
    g_AnmManager->DrawStringFormat(&this->impl->enemySpellcardName, COLOR_RGB(COLOR_BARELY_RED), COLOR_RGB(COLOR_BLACK),
                                   spellcardName);
    this->blueSpellcardBarLength = strlen(spellcardName) * 15 / 2.0f + 16.0f;
    g_SoundPlayer.PlaySoundByIdx(SOUND_BOMB);
}

ZunResult Gui::ActualAddedCallback()
{
    if (g_Supervisor.IsNotLoadingNextStage())
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
    if (g_Supervisor.IsNotLoadingNextStage())
    {
        for (i32 idx = 0; idx < ARRAY_SIZE_SIGNED(this->impl->vms); idx++)
        {
            g_AnmManager->SetAndExecuteScriptIdx(&this->impl->vms[idx], ANM_SCRIPT_FRONT_START + idx);
        }
    }
    this->bossPresent = false;
    this->impl->bossHealthBarState = 0;
    this->bossHealthBar1 = 0.0f;
    this->bossHealthBar2 = 0.0f;
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
    this->impl->playerSpellcardPortrait.flags.isVisible = false;
    this->impl->bombSpellcardName.flags.isVisible = false;
    this->impl->enemySpellcardPortrait.flags.isVisible = false;
    this->impl->enemySpellcardName.flags.isVisible = false;
    this->impl->bombSpellcardName.fontWidth = DEFAULT_ANM_FONT_SIZE;
    this->impl->bombSpellcardName.fontHeight = DEFAULT_ANM_FONT_SIZE;
    this->impl->enemySpellcardName.fontWidth = DEFAULT_ANM_FONT_SIZE;
    this->impl->enemySpellcardName.fontHeight = DEFAULT_ANM_FONT_SIZE;
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->stageNameSprite, ANM_SCRIPT_TEXT_STAGE_NAME);
    g_AnmManager->SetAndExecuteScriptIdx(&this->impl->songNameSprite, ANM_SCRIPT_TEXT_SONG_NAME);
    g_AnmManager->DrawStringFormat2(&this->impl->stageNameSprite, COLOR_RGB(COLOR_LIGHTCYAN), COLOR_RGB(COLOR_BLACK),
                                    g_Stage.stdData->stageName);
    this->impl->songNameSprite.fontWidth = DEFAULT_ANM_FONT_SIZE + 1;
    this->impl->songNameSprite.fontHeight = DEFAULT_ANM_FONT_SIZE + 1;
    g_AnmManager->DrawStringFormat(&this->impl->songNameSprite, COLOR_RGB(COLOR_LIGHTCYAN), COLOR_RGB(COLOR_BLACK),
                                   TH_SONG_NAME, g_Stage.stdData->songNames[0]);
    this->impl->msg.currentMsgIdx = -1;
    this->impl->finishedStage = FALSE;
    this->impl->bonusScore.isShown = false;
    this->impl->fullPowerMode.isShown = false;
    this->impl->spellCardBonus.isShown = false;
    this->flags.flag0 = 2;
    this->flags.flag1 = 2;
    this->flags.flag3 = 2;
    this->flags.flag4 = 2;
    this->flags.flag2 = 2;
    return ZUN_SUCCESS;
}

ZunResult Gui::LoadMsg(const char *path)
{
    this->FreeMsgFile();
    this->impl->msg.msgFile = (MsgRawHeader *)FileSystem::OpenPath(path);
    if (this->impl->msg.msgFile == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_GUI_MSG_FILE_CORRUPTED, path);
        return ZUN_ERROR;
    }
    this->impl->msg.currentMsgIdx = -1;
    this->impl->msg.currentInstr = NULL;
    for (i32 idx = 0; idx < this->impl->msg.msgFile->numInstrs; idx++)
    {
        this->impl->msg.msgFile->instrs[idx] =
            (MsgRawInstr *)((i32)this->impl->msg.msgFile->instrs[idx] + (i32)this->impl->msg.msgFile);
    }
    return ZUN_SUCCESS;
}

void Gui::FreeMsgFile()
{
    ZUN_SAFE_FREE(this->impl->msg.msgFile);
}

void Gui::MsgRead(i32 msgIdx)
{
    this->impl->MsgRead(msgIdx);
#if !TRIALBUILD
    g_Supervisor.forceRedrawFrames = 3;
#endif
}

void GuiImpl::MsgRead(i32 msgIdx)
{
    if (this->msg.msgFile->numInstrs <= msgIdx)
    {
        return;
    }
    MsgRawHeader *msgFile = this->msg.msgFile;
    memset(&this->msg, 0, sizeof(GuiMsgVm));
    this->msg.currentMsgIdx = msgIdx;
    this->msg.msgFile = msgFile;
    this->msg.currentInstr = this->msg.msgFile->instrs[msgIdx];
    this->msg.dialogueLines[0].anmFileIndex = -1;
    this->msg.dialogueLines[1].anmFileIndex = -1;
    this->msg.fontSize = DEFAULT_ANM_FONT_SIZE;
    this->msg.textColorsA[0] = COLOR_RGB(COLOR_GUI_1);
    this->msg.textColorsA[1] = COLOR_RGB(COLOR_GUI_2);
    this->msg.textColorsB[0] = COLOR_RGB(COLOR_BLACK);
    this->msg.textColorsB[1] = COLOR_RGB(COLOR_BLACK);
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
}

#define GET_ARG(type, num) ((type *)args)[num]

ZunResult GuiImpl::RunMsg()
{
    u8 *args;

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
        this->msg.scriptTimer = this->msg.currentInstr->time;
    }
    while (this->msg.scriptTimer >= (i32)this->msg.currentInstr->time)
    {
        switch (this->msg.currentInstr->opcode)
        {
        case MSG_OPCODE_MSG_DELETE:
            this->msg.currentMsgIdx = -1;
            return ZUN_ERROR;
        case MSG_OPCODE_PORTRAIT_ANM_SCRIPT:
            args = this->msg.currentInstr->args;
            g_AnmManager->SetAndExecuteScriptIdx(
                &this->msg.portraits[GET_ARG(i16, 0)],
                GET_ARG(i16, 1) + (GET_ARG(i16, 0) == 0 ? ANM_SCRIPT_FACE_START : ANM_SCRIPT_FACE_START + 2));
            break;
        case MSG_OPCODE_PORTRAIT_ANM_SPRITE:
            args = this->msg.currentInstr->args;
            g_AnmManager->SetActiveSprite(
                &this->msg.portraits[GET_ARG(i16, 0)],
                GET_ARG(i16, 1) + (GET_ARG(i16, 0) == 0 ? ANM_SCRIPT_FACE_START : ANM_SCRIPT_FACE_START + 8));
            break;
        case MSG_OPCODE_TEXT_DIALOGUE:
            args = this->msg.currentInstr->args;
            if (GET_ARG(i16, 1) == 0 && this->msg.dialogueLines[1].anmFileIndex >= 0)
            {
                g_AnmManager->DrawVmTextFmt(&this->msg.dialogueLines[1], this->msg.textColorsA[GET_ARG(i16, 0)],
                                            this->msg.textColorsB[GET_ARG(i16, 0)], " ");
            }
            g_AnmManager->SetAndExecuteScriptIdx(&this->msg.dialogueLines[GET_ARG(i16, 1)],
                                                 ANM_SCRIPT_TEXT_DIALOGUE_LINES + GET_ARG(i16, 1));
            this->msg.dialogueLines[GET_ARG(i16, 1)].fontWidth = this->msg.dialogueLines[GET_ARG(i16, 1)].fontHeight =
                this->msg.fontSize;
            g_AnmManager->DrawVmTextFmt(&this->msg.dialogueLines[GET_ARG(i16, 1)],
                                        this->msg.textColorsA[GET_ARG(i16, 0)], this->msg.textColorsB[GET_ARG(i16, 0)],
                                        (char *)(args + 4));
            this->msg.framesElapsedDuringPause = 0;
            break;
        case MSG_OPCODE_WAIT:
            if (!this->msg.dialogueSkippable || !IS_PRESSED(TH_BUTTON_SKIP))
            {
                if (!WAS_PRESSED(TH_BUTTON_SHOOT) || this->msg.framesElapsedDuringPause < 8)
                {
                    if (this->msg.framesElapsedDuringPause >= *(i32 *)this->msg.currentInstr->args)
                    {
                        break;
                    }
                    this->msg.framesElapsedDuringPause += 1;
                    goto break_skip_time;
                }
            }
            break;
        case MSG_OPCODE_ANM_INTERRUPT:
            args = this->msg.currentInstr->args;
            if (GET_ARG(i16, 0) < MSG_PORTRAIT_COUNT)
            {
                this->msg.portraits[GET_ARG(i16, 0)].pendingInterrupt = GET_ARG(u8, 2);
            }
            else
            {
                this->msg.dialogueLines[GET_ARG(i16, 0) - MSG_PORTRAIT_COUNT].pendingInterrupt = GET_ARG(u8, 2);
            }
            break;
        case MSG_OPCODE_ECL_RESUME:
            this->msg.ignoreWaitCounter += 1;
            break;
        case MSG_OPCODE_MUSIC:
            g_AnmManager->SetAndExecuteScriptIdx(&this->songNameSprite, ANM_SCRIPT_TEXT_SONG_NAME);
            this->songNameSprite.fontWidth = DEFAULT_ANM_FONT_SIZE + 1;
            this->songNameSprite.fontHeight = DEFAULT_ANM_FONT_SIZE + 1;
            g_AnmManager->DrawStringFormat(&this->songNameSprite, COLOR_RGB(COLOR_LIGHTCYAN), COLOR_RGB(COLOR_BLACK),
                                           TH_SONG_NAME,
                                           g_Stage.stdData->songNames[*(i32 *)this->msg.currentInstr->args]);
            if (g_Supervisor.PlayMidiFile(*(i32 *)this->msg.currentInstr->args))
            {
                g_Supervisor.PlayAudio(g_Stage.stdData->songPaths[*(i32 *)this->msg.currentInstr->args]);
            }
            break;
        case MSG_OPCODE_TEXT_INTRO:
            args = this->msg.currentInstr->args;
            g_AnmManager->SetAndExecuteScriptIdx(&this->msg.introLines[GET_ARG(i16, 1)],
                                                 ANM_SCRIPT_TEXT_INTRO_LINES + GET_ARG(i16, 1));
            g_AnmManager->DrawStringFormat(&this->msg.introLines[GET_ARG(i16, 1)],
                                           this->msg.textColorsA[GET_ARG(i16, 0)],
                                           this->msg.textColorsB[GET_ARG(i16, 0)], (char *)(args + 4));
            this->msg.framesElapsedDuringPause = 0;
            break;
        case MSG_OPCODE_STAGE_RESULTS:
            this->finishedStage = TRUE;
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
        case MSG_OPCODE_MSG_HALT:
            goto break_skip_time;
        case MSG_OPCODE_MUSIC_FADE_OUT:
            g_Supervisor.FadeOutMusic(4.0f);
            break;
        case MSG_OPCODE_STAGE_END:
            g_GameManager.guiScore = g_GameManager.score;
            if (g_GameManager.isInPracticeMode)
            {
                g_GameManager.guiScore = g_GameManager.score;
                g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
                goto break_skip_time;
            }
            if (
#if !TRIALBUILD
                g_GameManager.currentStage < 5 || (g_GameManager.difficulty != EASY && g_GameManager.currentStage == 5)
#else
                g_GameManager.currentStage < 3
#endif
            )
            {
                g_Supervisor.curState = SUPERVISOR_STATE_NEXT_STAGE;
            }
            else if (!g_GameManager.isInReplay)
            {
#if !TRIALBUILD
                if (g_GameManager.difficulty == EXTRA)
                {
                    g_GameManager.isGameCompleted = true;
                    g_GameManager.guiScore = g_GameManager.score;
                    g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
                    goto break_skip_time;
                }
                else
                {
                    g_Supervisor.curState = SUPERVISOR_STATE_ENDING;
                }
#else
                g_Supervisor.curState = SUPERVISOR_STATE_RESULTSCREEN_FROMGAME;
#endif
            }
            else
            {
                g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU_REPLAY;
            }
            goto break_skip_time;
        case MSG_OPCODE_MSG_FLAG_WAIT_SKIPPABLE:
            this->msg.dialogueSkippable = *(i32 *)this->msg.currentInstr->args;
            break;
        }
        this->msg.currentInstr = (MsgRawInstr *)((u32)this->msg.currentInstr->args + this->msg.currentInstr->argSize);
    }
    this->msg.scriptTimer++;
break_skip_time:
    g_AnmManager->ExecuteScript(&this->msg.portraits[0]);
    g_AnmManager->ExecuteScript(&this->msg.portraits[1]);
    g_AnmManager->ExecuteScript(&this->msg.dialogueLines[0]);
    g_AnmManager->ExecuteScript(&this->msg.dialogueLines[1]);
    g_AnmManager->ExecuteScript(&this->msg.introLines[0]);
    g_AnmManager->ExecuteScript(&this->msg.introLines[1]);
    if (this->msg.scriptTimer < 60 && this->msg.dialogueSkippable && IS_PRESSED(TH_BUTTON_SKIP))
    {
        this->msg.scriptTimer = 60;
    }
    return ZUN_SUCCESS;
}

#undef GET_ARG

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
    if (this->msg.scriptTimer < 60)
    {
        dialogueBoxHeight = (f32)this->msg.scriptTimer * 48.0f / 60.0f;
    }
    else
    {
        dialogueBoxHeight = 48.0f;
    }
    VertexDiffuseXyzrwh vertices[4];
    vertices[0].position = D3DXVECTOR3(
        g_GameManager.gameRegionScreenPos.x + (g_GameManager.gameRegionSize.x - 256.0f) / 2.0f - 16.0f, 384.0f, 0.0f);

    vertices[1].position = D3DXVECTOR3(g_GameManager.gameRegionScreenPos.x +
                                           (g_GameManager.gameRegionSize.x - 256.0f) / 2.0f + 256.0f + 16.0f,
                                       384.0f, 0.0f);

    vertices[2].position =
        D3DXVECTOR3(g_GameManager.gameRegionScreenPos.x + (g_GameManager.gameRegionSize.x - 256.0f) / 2.0f - 16.0f,
                    384.0f + dialogueBoxHeight, 0.0f);

    vertices[3].position = D3DXVECTOR3(g_GameManager.gameRegionScreenPos.x +
                                           (g_GameManager.gameRegionSize.x - 256.0f) / 2.0f + 256.0f + 16.0f,
                                       384.0f + dialogueBoxHeight, 0.0f);

    vertices[0].diffuse = vertices[1].diffuse = 0xd0000000;
    vertices[2].diffuse = vertices[3].diffuse = 0x90000000;
    vertices[0].position_w = vertices[1].position_w = vertices[2].position_w = vertices[3].position_w = 1.0f;
    g_AnmManager->DrawNoRotation(&this->msg.portraits[0]);
    g_AnmManager->DrawNoRotation(&this->msg.portraits[1]);
    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    if (!g_Supervisor.IsDepthTestDisabled())
    {
        g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    }
    g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
    g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
    g_AnmManager->SetCurrentVertexShader(AnmVertexShader_NotSet);
    g_AnmManager->SetCurrentColorOp(AnmColorOp_NotSet);
    g_AnmManager->SetCurrentBlendMode(AnmBlendMode_NotSet);
    g_AnmManager->SetCurrentZWriteDisable(AnmZWriteState_NotSet);
    if (!g_Supervisor.IsColorCompositingDisabled())
    {
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    }
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_AnmManager->DrawNoRotation(&this->msg.dialogueLines[0]);
    g_AnmManager->DrawNoRotation(&this->msg.dialogueLines[1]);
    g_AnmManager->DrawNoRotation(&this->msg.introLines[0]);
    g_AnmManager->DrawNoRotation(&this->msg.introLines[1]);
    return ZUN_SUCCESS;
}

ZunBool Gui::MsgWait()
{
    if (this->impl->msg.ignoreWaitCounter > 0)
    {
        return false;
    }
    return this->impl->msg.currentMsgIdx >= 0;
}

ZunBool Gui::HasCurrentMsgIdx()
{
    return this->impl->msg.currentMsgIdx >= 0;
}

void Gui::UpdateStageElements()
{
    for (i32 idx = 0; idx < ARRAY_SIZE_SIGNED(this->impl->vms); idx++)
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
                        this->bossUIOpacity = 255;
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
            if (this->impl->bossHealthBarState >= 2)
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
    if (this->impl->loadingScreenSprite.activeSpriteIndex >= 0 &&
        g_AnmManager->ExecuteScript(&this->impl->loadingScreenSprite) != 0)
    {
        this->impl->loadingScreenSprite.activeSpriteIndex = -1;
    }
    if (this->impl->bonusScore.isShown)
    {
        if (this->impl->bonusScore.timer < 30)
        {
            this->impl->bonusScore.pos.x = GAME_REGION_POS_RIGHT + -312.0f * (f32)this->impl->bonusScore.timer / 30.0f;
        }
        else
        {
            this->impl->bonusScore.pos.x = 104.0f;
        }
        if (this->impl->bonusScore.timer >= 250)
        {
            this->impl->bonusScore.isShown = false;
        }
        this->impl->bonusScore.timer++;
    }
    if (this->impl->fullPowerMode.isShown)
    {
        if (this->impl->fullPowerMode.timer < 30)
        {
            this->impl->fullPowerMode.pos.x =
                GAME_REGION_POS_RIGHT + -312.0f * (f32)this->impl->fullPowerMode.timer / 30.0f;
        }
        else
        {
            this->impl->fullPowerMode.pos.x = 104.0f;
        }
        if (this->impl->fullPowerMode.timer >= 180)
        {
            this->impl->fullPowerMode.isShown = false;
        }
        this->impl->fullPowerMode.timer++;
    }
    if (this->impl->spellCardBonus.isShown)
    {
        if (this->impl->spellCardBonus.timer >= 280)
        {
            this->impl->spellCardBonus.isShown = false;
        }
        this->impl->spellCardBonus.timer++;
    }
    if (this->impl->finishedStage == TRUE)
    {
        i32 stageScore = 0;
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
#if BUILD_VERSION >= BUILD_VERSION_102h
        switch (g_Supervisor.defaultConfig.lifeCount)
#else
        switch (g_Supervisor.cfg.lifeCount)
#endif
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
        g_GameManager.AddScore(stageScore);
        this->impl->finishedStage += 1;
    }
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

#pragma var_order(cappedSpellcardSecondsRemaining, bossLivesColor, textPos)
    if (this->impl->msg.currentMsgIdx < 0 && (this->bossPresent + this->impl->bossHealthBarState) > 0)
    {
        vm = &this->impl->vms[19];
        g_AnmManager->DrawNoRotation(vm);
        vm = &this->impl->vms[21];
        vm->flags.anchor = AnmVmAnchor_TopLeft;
        vm->scaleX = (this->bossHealthBar2 * 288.0f) / 14.0f;
        vm->pos.x = 96.0f;
        vm->pos.y = 24.0f;
        vm->pos.z = 0.0f;
        g_AnmManager->DrawNoRotation(vm);
        D3DXVECTOR3 textPos(80.0f, 16.0f, 0.0f);
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
            g_SoundPlayer.PlaySoundByIdx(SOUND_1D);
        }
        g_AsciiManager.AddFormatText(&textPos, "%.2d", cappedSpellcardSecondsRemaining);
        g_AsciiManager.SetColor(COLOR_WHITE);
        this->lastSpellcardSecondsRemaining = this->spellcardSecondsRemaining;
    }
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = GAME_WINDOW_WIDTH;
    g_Supervisor.viewport.Height = GAME_WINDOW_HEIGHT;
    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    vm = &this->impl->vms[6];
    if (!g_Supervisor.IsMinimumGraphicsMode() &&
        (vm->currentInstruction != NULL || g_Supervisor.forceRedrawFrames != 0 ||
         g_Supervisor.ShouldForceBackbufferClear()))
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
    if (!g_Supervisor.IsMinimumGraphicsMode())
    {
        vm = &this->impl->vms[22];
        xPos = 496.0f;
        vm->pos = D3DXVECTOR3(xPos, 58.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
        vm->pos = D3DXVECTOR3(xPos, 82.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
        if (this->flags.flag0 != 0)
        {
            vm->pos = D3DXVECTOR3(xPos, 122.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.flag1 != 0)
        {
            vm->pos = D3DXVECTOR3(xPos, 146.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.flag2 != 0)
        {
            vm->pos = D3DXVECTOR3(xPos, 186.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.flag3 != 0)
        {
            vm->pos = D3DXVECTOR3(xPos, 206.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        if (this->flags.flag4 != 0)
        {
            vm->pos = D3DXVECTOR3(xPos, 226.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
        vm->pos = D3DXVECTOR3(488.0f, 464.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
        vm->pos = D3DXVECTOR3(0.0f, 464.0f, 0.49f);
        g_AnmManager->DrawNoRotation(vm);
    }
    if (this->flags.flag0 != 0 || g_Supervisor.IsMinimumGraphicsMode())
    {
        vm = &this->impl->vms[16];
        for (idx = 0, xPos = 496.0f; idx < g_GameManager.livesRemaining; idx++, xPos += 16.0f)
        {
            vm->pos = D3DXVECTOR3(xPos, 122.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
    }
    if (this->flags.flag1 != 0 || g_Supervisor.IsMinimumGraphicsMode())
    {
        vm = &this->impl->vms[17];
        for (idx = 0, xPos = 496.0f; idx < g_GameManager.bombsRemaining; idx++, xPos += 16.0f)
        {
            vm->pos = D3DXVECTOR3(xPos, 146.0f, 0.49f);
            g_AnmManager->DrawNoRotation(vm);
        }
    }
    if (this->flags.flag2 != 0 || g_Supervisor.IsMinimumGraphicsMode())
    {
        VertexDiffuseXyzrwh vertices[4];
        if (g_GameManager.currentPower > 0)
        {
            vertices[0].position = D3DXVECTOR3(496.0f, 186.0f, 0.1f);
            vertices[1].position = D3DXVECTOR3(g_GameManager.currentPower + 496 + 0.0f, 186.0f, 0.1f);
            vertices[2].position = D3DXVECTOR3(496.0f, 202.0f, 0.1f);
            vertices[3].position = D3DXVECTOR3(g_GameManager.currentPower + 496 + 0.0f, 202.0f, 0.1f);

            vertices[0].diffuse = vertices[2].diffuse = 0xe0e0e0ff;
            vertices[1].diffuse = vertices[3].diffuse = 0x80e0e0ff;

            vertices[0].position_w = vertices[1].position_w = vertices[2].position_w = vertices[3].position_w = 1.0;

            if (!g_Supervisor.IsColorCompositingDisabled())
            {
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            }
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            if (!g_Supervisor.IsDepthTestDisabled())
            {
                g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
                g_Supervisor.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            }
            g_Supervisor.d3dDevice->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
            g_Supervisor.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(VertexDiffuseXyzrwh));
            g_AnmManager->SetCurrentVertexShader(AnmVertexShader_NotSet);
            g_AnmManager->SetCurrentColorOp(AnmColorOp_NotSet);
            g_AnmManager->SetCurrentBlendMode(AnmBlendMode_NotSet);
            g_AnmManager->SetCurrentZWriteDisable(AnmZWriteState_NotSet);
            if (!g_Supervisor.IsColorCompositingDisabled())
            {
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            }
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            g_Supervisor.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            if (g_GameManager.currentPower >= MAX_POWER)
            {
                vm = &this->impl->vms[18];
                vm->pos = D3DXVECTOR3(496.0f, 186.0f, 0.0f);
                g_AnmManager->DrawNoRotation(vm);
            }
        }
        if (g_GameManager.currentPower < MAX_POWER)
        {
            g_AsciiManager.AddFormatText(&D3DXVECTOR3(496.0f, 186.0f, 0.0f), "%d", g_GameManager.currentPower);
        }
    }
    {
        D3DXVECTOR3 elemPos(496.0f, 82.0f, 0.0f);
        g_AsciiManager.AddFormatText(&elemPos, "%.9d", g_GameManager.guiScore);
        elemPos = D3DXVECTOR3(496.0f, 58.0f, 0.0f);
        g_AsciiManager.AddFormatText(&elemPos, "%.9d", g_GameManager.highScore);
        if (this->flags.flag3 || g_Supervisor.IsMinimumGraphicsMode())
        {
            elemPos = D3DXVECTOR3(496.0f, 206.0f, 0.0f);
            g_AsciiManager.AddFormatText(&elemPos, "%d", g_GameManager.grazeInStage);
        }
        if (this->flags.flag4 || g_Supervisor.IsMinimumGraphicsMode())
        {
            elemPos = D3DXVECTOR3(496.0f, 226.0f, 0.0f);
            g_AsciiManager.AddFormatText(&elemPos, "%d", g_GameManager.pointItemsCollectedInStage);
        }
    }
    if (this->flags.flag0 != 0)
    {
        this->flags.flag0--;
    }
    if (this->flags.flag2 != 0)
    {
        this->flags.flag2--;
    }
    if (this->flags.flag1 != 0)
    {
        this->flags.flag1--;
    }
    if (this->flags.flag3 != 0)
    {
        this->flags.flag3--;
    }
    if (this->flags.flag4 != 0)
    {
        this->flags.flag4--;
    }
}

void Gui::DrawStageElements()
{
    D3DXVECTOR3 stageTextPos;

    if (this->impl->stageNameSprite.IsVisible())
    {
        stageTextPos.x = 168.0f;
        stageTextPos.y = 198.0f;
        stageTextPos.z = 0.0f;
        if (!g_GameManager.demoMode)
        {
            g_AnmManager->Draw2(&this->impl->stageNameSprite);

            // this looks like an inline function, maybe ZunColor is a struct?
            ZunColor stageTextColor = COLOR_COMBINE_ALPHA(COLOR_SUNSHINEYELLOW, this->impl->stageNameSprite.color);
            g_AsciiManager.SetColor(stageTextColor);

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
            ZunColor demoTextColor = COLOR_COMBINE_ALPHA(COLOR_SUNSHINEYELLOW, this->impl->stageNameSprite.color);
            g_AsciiManager.SetColor(demoTextColor);

            stageTextPos.x = 136.0f;

            g_AsciiManager.AddFormatText(&stageTextPos, " DEMO PLAY");
        }
        g_AsciiManager.SetColor(COLOR_WHITE);
    }

    if (this->impl->songNameSprite.IsVisible() && !g_GameManager.demoMode)
    {
        g_AnmManager->Draw2(&this->impl->songNameSprite);
    }
    if (this->impl->playerSpellcardPortrait.IsVisible())
    {
        g_AnmManager->DrawNoRotation(&this->impl->playerSpellcardPortrait);
    }
    if (this->impl->enemySpellcardPortrait.IsVisible())
    {
        g_AnmManager->DrawNoRotation(&this->impl->enemySpellcardPortrait);
    }

    if (this->impl->bombSpellcardName.IsVisible())
    {
        this->impl->bombSpellcardBackground.pos = this->impl->bombSpellcardName.pos;
        this->impl->bombSpellcardBackground.pos.x +=
            this->bombSpellcardBarLength * 16.0f / 15.0f / 2.0f + -128.0f - 16.0f;
        this->impl->bombSpellcardBackground.scaleX = this->bombSpellcardBarLength / 14.0f;
        g_AnmManager->DrawNoRotation(&this->impl->bombSpellcardBackground);
        g_AnmManager->DrawNoRotation(&this->impl->bombSpellcardName);
    }
    if (this->impl->enemySpellcardName.IsVisible())
    {
        this->impl->enemySpellcardBackground.pos = this->impl->enemySpellcardName.pos;
        this->impl->enemySpellcardBackground.pos.x += 128.0f - this->blueSpellcardBarLength * 16.0f / 15.0f / 2.0f;
        this->impl->enemySpellcardBackground.scaleX = this->blueSpellcardBarLength / 14.0f;
        g_AnmManager->DrawNoRotation(&this->impl->enemySpellcardBackground);
        g_AnmManager->DrawNoRotation(&this->impl->enemySpellcardName);
    }
    if (this->impl->loadingScreenSprite.activeSpriteIndex >= 0)
    {
        g_Supervisor.viewport.X = g_GameManager.gameRegionScreenPos.x;
        g_Supervisor.viewport.Y = g_GameManager.gameRegionScreenPos.y;
        g_Supervisor.viewport.Width = g_GameManager.gameRegionSize.x;
        g_Supervisor.viewport.Height = g_GameManager.gameRegionSize.y;

        g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
        g_AnmManager->DrawNoRotation(&this->impl->loadingScreenSprite);
    }
}

ZunResult Gui_AddedCallback(Gui *gui)
{
    return gui->ActualAddedCallback();
}

ZunResult Gui_DeletedCallback(Gui *gui)
{
    g_AnmManager->ReleaseAnm(ANM_FILE_FACE_STAGE_A);
    g_AnmManager->ReleaseAnm(ANM_FILE_FACE_STAGE_B);
    g_AnmManager->ReleaseAnm(ANM_FILE_FACE_STAGE_C);
    gui->FreeMsgFile();
    if (g_Supervisor.IsNotLoadingNextStage())
    {
        g_AnmManager->ReleaseAnm(ANM_FILE_FRONT);
        g_AnmManager->ReleaseAnm(ANM_FILE_LOADING);
        g_AnmManager->ReleaseAnm(ANM_FILE_FACE_CHARA_A);
        g_AnmManager->ReleaseAnm(ANM_FILE_FACE_CHARA_B);
        g_AnmManager->ReleaseAnm(ANM_FILE_FACE_CHARA_C);
        ZUN_DELETE(gui->impl);
    }
    return ZUN_SUCCESS;
}

ZunResult Gui_RegisterChain()
{
    Gui *gui = &g_Gui;
    if (g_Supervisor.IsNotLoadingNextStage())
    {
        memset(gui, 0, sizeof(Gui));
        gui->impl = ZUN_NEW(GuiImpl);
    }
    g_GuiCalcChain.SetCallback((ChainCallback)Gui_OnUpdate);
    g_GuiCalcChain.addedCallback = (ChainAddedCallback)Gui_AddedCallback;
    g_GuiCalcChain.deletedCallback = (ChainDeletedCallback)Gui_DeletedCallback;
    g_GuiCalcChain.arg = gui;
    if (g_Chain.AddToCalcChain(&g_GuiCalcChain, TH_CHAIN_PRIO_CALC_GUI) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }
    g_GuiDrawChain.SetCallback((ChainCallback)Gui_OnDraw);
    g_GuiDrawChain.arg = gui;
    g_Chain.AddToDrawChain(&g_GuiDrawChain, TH_CHAIN_PRIO_DRAW_GUI);
    return ZUN_SUCCESS;
}

void Gui_CutChain()
{
    g_Chain.Cut(&g_GuiCalcChain);
    g_Chain.Cut(&g_GuiDrawChain);
}
} // namespace th06

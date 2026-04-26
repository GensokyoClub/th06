#pragma once

#include "AnmVm.hpp"
#include "Chain.hpp"
#include "Enemy.hpp"
#include "ZunTimer.hpp"
#include "inttypes.hpp"
// #include <Windows.h>

enum MsgOps
{
    MSG_OPCODE_MSGDELETE,
    MSG_OPCODE_PORTRAITANMSCRIPT,
    MSG_OPCODE_PORTRAITANMSPRITE,
    MSG_OPCODE_TEXTDIALOGUE,
    MSG_OPCODE_WAIT,
    MSG_OPCODE_ANMINTERRUPT,
    MSG_OPCODE_ECLRESUME,
    MSG_OPCODE_MUSIC,
    MSG_OPCODE_TEXTINTRO,
    MSG_OPCODE_STAGERESULTS,
    MSG_OPCODE_MSGHALT,
    MSG_OPCODE_STAGEEND,
    MSG_OPCODE_MUSICFADEOUT,
    MSG_OPCODE_WAITSKIPPABLE,
};

struct MsgRawInstrArgPortraitAnmScript
{
    i16 portraitIdx;
    i16 anmScriptIdx;
};
struct MsgRawInstrArgText
{
    i16 textColor;
    i16 textLine;
    char text[1];
};
struct MsgRawInstrArgAnmInterrupt
{
    i16 unk1;
    u8 unk2;
};
union MsgRawInstrArgs {
    MsgRawInstrArgPortraitAnmScript portraitAnmScript;
    MsgRawInstrArgText text;
    i32 dialogueSkippable;
    i32 wait;
    MsgRawInstrArgAnmInterrupt anmInterrupt;
    i32 music;
};
struct MsgRawInstr
{
    u16 time;
    u8 opcode;
    u8 argSize;
    MsgRawInstrArgs args;
};

struct MsgRawHeader
{
    i32 numInstrs;
    u32 instrsOffsets[1];
};

struct GuiMsgVm
{
    const MsgRawHeader *msgFile;
    const MsgRawInstr **instrs;
    const MsgRawInstr *currentInstr;
    i32 currentMsgIdx;
    ZunTimer timer;
    i32 framesElapsedDuringPause;
    AnmVm portraits[2];
    AnmVm dialogueLines[2];
    AnmVm introLines[2];
    ZunColor textColorsA[4];
    ZunColor textColorsB[4];
    u32 fontSize;
    u32 ignoreWaitCounter;
    u8 dialogueSkippable;
};

struct GuiFormattedText
{
    ZunVec3 pos;
    i32 fmtArg;
    i32 isShown;
    ZunTimer timer;
};

struct GuiImpl
{
    GuiImpl();
    ZunResult RunMsg();
    ZunResult DrawDialogue() const;
    void MsgRead(i32 msgIdx);

    AnmVm vms[26];
    u8 bossHealthBarState;
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
    u32 finishedStage;
    u32 stageScore;
    GuiFormattedText bonusScore;
    GuiFormattedText fullPowerMode;
    GuiFormattedText spellCardBonus;
};
struct GuiFlags
{
    u32 flag0 : 2;
    u32 flag1 : 2;
    u32 flag2 : 2;
    u32 flag3 : 2;
    u32 flag4 : 2;
};

struct Gui
{
    static ZunResult RegisterChain();
    static void CutChain();
    static ZunResult AddedCallback(Gui *);
    static ZunResult DeletedCallback(Gui *);
    static ChainCallbackResult OnUpdate(Gui *);
    static ChainCallbackResult OnDraw(Gui *);

    ZunResult ActualAddedCallback();
    ZunResult LoadMsg(const char *path) const;
    void FreeMsgFile() const;

    bool IsStageFinished() const;

    void UpdateStageElements();
    bool HasCurrentMsgIdx() const;

    void DrawStageElements() const;
    void DrawGameScene();

    void MsgRead(i32 msgIdx) const;
    bool MsgWait() const;

    void ShowSpellcard(i32 spellcardSprite, const char *spellcardName);
    void ShowSpellcardBonus(u32 spellcardScore) const;
    void ShowBombNamePortrait(u32 sprite, const char *bombName);
    void ShowBonusScore(u32 bonusScore) const;
    void EndEnemySpellcard() const;
    void EndPlayerSpellcard() const;
    bool IsDialogueSkippable() const;

    void ShowFullPowerMode(i32 fmtArg) const;

    void SetBossHealthBar(f32 val)
    {
        this->bossHealthBar1 = val;
    }

    bool BossPresent() const
    {
        return this->bossPresent;
    }

    void SetSpellcardSeconds(i32 val)
    {
        this->spellcardSecondsRemaining = val;
    }

    i32 SpellcardSecondsRemaining() const
    {
        return this->spellcardSecondsRemaining;
    }

    void TickTimer(ZunTimer *timer)
    {
        timer->NextTick();
    }

    GuiFlags flags;
    GuiImpl *impl;
    f32 bombSpellcardBarLength;
    f32 blueSpellcardBarLength;
    u32 bossUIOpacity;
    i32 eclSetLives;
    i32 spellcardSecondsRemaining;
    i32 lastSpellcardSecondsRemaining;
    bool bossPresent;
    f32 bossHealthBar1;
    f32 bossHealthBar2;
};

extern Gui g_Gui;

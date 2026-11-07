#pragma once
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
struct GuiImpl;
struct GuiRedrawCounters
{
    u32 lifeIcons : 2;
    u32 bombIcons : 2;
    u32 powerGauge : 2;
    u32 grazeCounter : 2;
    u32 pointItemCounter : 2;
    alignment_bitfields(u32, 22);
};
ZUN_ASSERT_TYPE(GuiRedrawCounters, 0x4, 4);

#define MSG_SCRIPTS_PER_CHARACTER 10

#define MSG_SCRIPT_ID(character, num) ((num) + (character) * MSG_SCRIPTS_PER_CHARACTER)

struct Gui
{
    ZunResult ActualAddedCallback();
    ZunResult LoadMsg(const char *path);
    void FreeMsgFile();

    ZunBool IsStageFinished();

    void UpdateStageElements();
    ZunBool IsMsgActive();

    void DrawStageElements();
    void DrawGameScene();

    void MsgRead(i32 msgIdx);
    ZunBool MsgWait();

    void ShowSpellcard(i32 spellcardSprite, const char *spellcardName);
    void ShowSpellcardBonus(u32 spellcardScore);
    void ShowBombNamePortrait(u32 sprite, const char *bombName);
    void ShowBonusScore(u32 bonusScore);
    void EndEnemySpellcard();
    void EndPlayerSpellcard();
    ZunBool IsDialogueSkippable();

    void ShowFullPowerMode(i32 fmtArg);

    void SetBossHealthBar(f32 val)
    {
        this->bossHealthBarValue = val;
    }

    void SetSpellcardSeconds(i32 val)
    {
        this->spellcardSecondsRemaining = val;
    }

    void SetLives(i32 lives)
    {
        this->bossLifeCount = lives;
    }

    bool SetBossPresent(bool bossPresent)
    {
        this->bossPresent = bossPresent;
        return bossPresent;
    }

    bool IsBossPresent()
    {
        return this->bossPresent;
    }

    i32 SpellcardSecondsRemaining()
    {
        return this->spellcardSecondsRemaining;
    }

    GuiRedrawCounters redrawFrames;
    GuiImpl *impl;
    f32 bombNameBarLength;
    f32 spellcardNameBarLength;
    u32 bossUIAlpha;
    i32 bossLifeCount;
    i32 spellcardSecondsRemaining;
    i32 lastSpellcardSecondsRemaining;
    bool bossPresent;
    alignment_padding(0x3);
    f32 bossHealthBarValue;
    f32 bossHealthBarDisplay; // Used to make the visual bar lag behind the real value with *fancy* animations
};
ZUN_ASSERT_TYPE(Gui, 0x2c, 4);

ZunResult Gui_RegisterChain();
void Gui_CutChain();

extern Gui g_Gui;
} // namespace th06

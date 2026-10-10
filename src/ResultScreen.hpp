#pragma once
#include "AnmVm.hpp"
#include "Global.hpp"
#include "ReplayData.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

namespace th06
{
#define DEFAULT_MAGIC "DMYS"
#define TH6K_MAGIC 'K6HT'
#define HSCR_MAGIC 'RCSH'
#define CLRD_MAGIC 'DRLC'
#define PSCR_MAGIC 'RCSP'
#define CATK_MAGIC 'KTAC'

#define HSCR_NUM_DIFFICULTIES 5
#define HSCR_NUM_SCORES_SLOTS 10

#define PSCR_NUM_STAGES 6
#define PSCR_NUM_DIFFICULTIES 4

#define CATK_NUM_CAPTURES 64

#define TH6K_VERSION 16

#define RESULT_KEYBOARD_COLUMNS 16
#define RESULT_KEYBOARD_ROWS 6
#define RESULT_KEYBOARD_CHARACTERS RESULT_KEYBOARD_COLUMNS *RESULT_KEYBOARD_ROWS
#define RESULT_KEYBOARD_SPACE 94
#define RESULT_KEYBOARD_END 95

#define SCORE_DAT_FILE_BUFFER_SIZE 0xa0000

struct Th6k
{
    u32 magic;
    u16 th6kLen;
    u16 unkLen;
    u8 version;
    u8 flag_9;
    alignment_padding(0x2);
};
ZUN_ASSERT_TYPE(Th6k, 0xc, 4);

struct Catk
{
    Th6k base;
    i32 captureScore;
    u16 idx;
    u8 nameCsum;
    u8 characterShotType[SHOTTYPE_COUNT + 1];
    char name[34]; // probably 36 since 34 as the ECL spell buffer length is likely a bug
    unreferenced_fields(0x2);
    u16 numAttempts;
    u16 numSuccess;
};
ZUN_ASSERT_TYPE(Catk, 0x40, 4);

struct Clrd
{
    Th6k base;
    u8 difficultyClearedWithRetries[5];
    u8 difficultyClearedWithoutRetries[5];
    u8 characterShotType;
    alignment_padding(0x1);
};
ZUN_ASSERT_TYPE(Clrd, 0x18, 4);

struct Pscr
{
    Th6k base;
    i32 score;
    u8 character;
    u8 difficulty;
    u8 stage;
    alignment_padding(0x1);
};
ZUN_ASSERT_TYPE(Pscr, 0x14, 4);

struct Hscr
{
    Th6k base;
    u32 score;
    u8 character;
    u8 difficulty;
    u8 stage;
    char name[9];
};
ZUN_ASSERT_TYPE(Hscr, 0x1c, 4);

struct ScoreListNode
{
    ScoreListNode()
    {
        this->prev = NULL;
        this->next = NULL;
        this->data = NULL;
    }

    ScoreListNode *prev;
    ScoreListNode *next;
    Hscr *data;
};
ZUN_ASSERT_TYPE(ScoreListNode, 0xc, 4);

// score.dat is opaque outside of ResultScreen.cpp: everyone else only passes
// the handle OpenScore returns back into these functions.
struct ScoreDat;
ScoreDat *OpenScore(const char *path);
void ReleaseScoreDat(ScoreDat *scoreDat);

u32 GetHighScore(ScoreDat *scoreDat, ScoreListNode *node, u32 character, u32 difficulty);

ZunResult ParseCatk(ScoreDat *scoreDat, Catk *catk);
ZunResult ParseClrd(ScoreDat *scoreDat, Clrd *out);
ZunResult ParsePscr(ScoreDat *scoreDat, Pscr *out);

ZunResult ResultScreen_RegisterChain(ZunBool unk);
} // namespace th06

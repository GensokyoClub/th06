#pragma once

#include "AnmVm.hpp"
#include "ReplayData.hpp"
#include "ZunResult.hpp"
#include "inttypes.hpp"

namespace th06
{

#define TH6K_MAGIC 'K6HT'
#define HSCR_MAGIC 'RCSH'
#define CLRD_MAGIC 'DRLC'
#define PSCR_MAGIC 'RCSP'
#define CATK_MAGIC 'KTAC'

#define HSCR_NUM_CHARS_SHOTTYPES 4
#define HSCR_NUM_DIFFICULTIES 5
#define HSCR_NUM_SCORES_SLOTS 10

#define TH6K_VERSION 16

#define RESULT_KEYBOARD_COLUMNS 16
#define RESULT_KEYBOARD_ROWS 6
#define RESULT_KEYBOARD_CHARACTERS RESULT_KEYBOARD_COLUMNS *RESULT_KEYBOARD_ROWS
#define RESULT_KEYBOARD_SPACE 94
#define RESULT_KEYBOARD_END 95

#define SCORE_DAT_FILE_BUFFER_SIZE 0xa0000

enum ResultScreenState
{
    RESULT_SCREEN_STATE_INIT = 0,
    RESULT_SCREEN_STATE_CHOOSING_DIFFICULTY,
    RESULT_SCREEN_STATE_EXITING,
    RESULT_SCREEN_STATE_BEST_SCORES_EASY,
    RESULT_SCREEN_STATE_BEST_SCORES_NORMAL,
    RESULT_SCREEN_STATE_BEST_SCORES_HARD,
    RESULT_SCREEN_STATE_BEST_SCORES_LUNATIC,
    RESULT_SCREEN_STATE_BEST_SCORES_EXTRA,
    RESULT_SCREEN_STATE_SPELLCARDS,
    RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME,
    RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION,
    RESULT_SCREEN_STATE_CANT_SAVE_REPLAY,
    RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE,
    RESULT_SCREEN_STATE_WRITING_REPLAY_NAME,
    RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE,
    RESULT_SCREEN_STATE_STATS_SCREEN,
    RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION,
    RESULT_SCREEN_STATE_EXIT,
};

enum ResultScreenMainMenuCursor
{
    RESULT_SCREEN_CURSOR_EASY,
    RESULT_SCREEN_CURSOR_NORMAL,
    RESULT_SCREEN_CURSOR_HARD,
    RESULT_SCREEN_CURSOR_LUNATIC,
    RESULT_SCREEN_CURSOR_EXTRA,
    RESULT_SCREEN_CURSOR_SPELLCARDS,
    RESULT_SCREEN_CURSOR_EXIT
};

struct Th6k
{
    Th6k *ShiftOneByte()
    {
        return (Th6k *)(((u8 *)this) + 1);
    };

    Th6k *ShiftBytes(i32 value)
    {
        return (Th6k *)(((u8 *)this) + value);
    };

    u32 magic;
    u16 th6kLen;
    u16 unkLen;
    u8 version;
    u8 unk_9;
};
ZUN_ASSERT_SIZE(Th6k, 0xc);

struct Catk
{
    Th6k base;
    i32 captureScore;
    u16 idx;
    u8 nameCsum;
    u8 characterShotType;
    u32 unk_14;
    char name[32];
    u32 unk_38;
    u16 numAttempts;
    u16 numSuccess;
};
ZUN_ASSERT_SIZE(Catk, 0x40);

struct Clrd
{
    Th6k base;
    u8 difficultyClearedWithRetries[5];
    u8 difficultyClearedWithoutRetries[5];
    u8 characterShotType;
};
ZUN_ASSERT_SIZE(Clrd, 0x18);

struct Pscr
{
    Pscr *ShiftOneByte()
    {
        return (Pscr *)(((u8 *)this) + 1);
    };

    Pscr *ShiftBytes(i32 value)
    {
        return (Pscr *)(((u8 *)this) + value);
    };

    Th6k base;
    i32 score;
    u8 character;
    u8 difficulty;
    u8 stage;
};
ZUN_ASSERT_SIZE(Pscr, 0x14);

struct Hscr
{
    Hscr *ShiftBytes(i32 value)
    {
        return (Hscr *)(((u8 *)this) + value);
    };

    Th6k base;
    u32 score;
    u8 character;
    u8 difficulty;
    u8 stage;
    char name[9];
};
ZUN_ASSERT_SIZE(Hscr, 0x1c);

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
ZUN_ASSERT_SIZE(ScoreListNode, 0xc);

struct ScoreRaw
{
    Th6k *ShiftOneByte()
    {
        return (Th6k *)(((u8 *)this) + 1);
    };

    Th6k *ShiftBytes(i32 value)
    {
        return (Th6k *)(((u8 *)this) + value);
    };

    u8 xorseed[2];
    u16 csum;
    u16 unk_8;
    u8 unk[2];
    u32 dataOffset;
    u32 padding; // Originally used as space for a ScoreListNode pointer, but that caused obvious ABI issues
    u32 fileLen;
};
ZUN_ASSERT_SIZE(ScoreDat, 0x14);

struct ScoreDat
{
    ScoreRaw *rawScoreFile;
    ScoreListNode *scores;
};

struct ResultScreen
{
    ResultScreen();
    ~ResultScreen()
    {
        ScoreDat *sd = this->scoreDat;
        free(sd);
    };

    static ZunResult RegisterChain(i32 unk);
    static ChainCallbackResult OnUpdate(ResultScreen *r);
    static ChainCallbackResult OnDraw(ResultScreen *r);
    static ZunResult AddedCallback(ResultScreen *r);
    static ZunResult DeletedCallback(ResultScreen *r);

    static ScoreDat *OpenScore(char *path);
    static ZunResult ParseCatk(ScoreDat *s, Catk *catk);
    static ZunResult ParseClrd(ScoreDat *s, Clrd *out);
    static ZunResult ParsePscr(ScoreDat *s, Pscr *out);

    static void WriteScore(ResultScreen *r);
    void FreeScore(i32 difficulty, i32 character);
    static u32 GetHighScore(ScoreDat *s, ScoreListNode *node, u32 character, u32 difficulty);
    static void ReleaseScoreDat(ScoreDat *s);

    static void MoveCursor(ResultScreen *r, i32 len);
    static bool MoveCursorHorizontally(ResultScreen *r, i32 len);

    static void FreeAllScores(ScoreListNode *scores);

    i32 HandleResultKeyboard();
    i32 HandleReplaySaveKeyboard();
    ZunResult CheckConfirmButton();

    static i32 LinkScore(ScoreListNode *, Hscr *);
    i32 LinkScoreEx(Hscr *out, i32 difficulty, i32 character);
    u32 DrawFinalStats();

    ScoreDat *scoreDat;
    i32 frameTimer;
    i32 resultScreenState;
    i32 lastResultScreenState;
    i32 cursor;
    i32 lastBestScoresCursor;
    i32 previousCursor;
    i32 replayNumber;
    i32 selectedCharacter;
    i32 charUsed;
    i32 lastSpellcardSelected;
    i32 diffSelected;
    i32 cheatCodeStep;
    char replayName[8];
    i32 unk_3c;
    AnmVm unk_40[38];
    AnmVm unk_28a0[16];
    AnmVm unk_39a0;
    ScoreListNode scores[HSCR_NUM_DIFFICULTIES][HSCR_NUM_CHARS_SHOTTYPES];
    Hscr defaultScore[HSCR_NUM_DIFFICULTIES][HSCR_NUM_CHARS_SHOTTYPES][HSCR_NUM_SCORES_SLOTS];
    Hscr hscr;
    Th6k fileHeader;
    ChainElem *calcChain;
    ChainElem *drawChain;
    ReplayHeader replays[15];
    ReplayHeader defaultReplay;
};
ZUN_ASSERT_SIZE(ResultScreen, 0x56b0);
}; // namespace th06

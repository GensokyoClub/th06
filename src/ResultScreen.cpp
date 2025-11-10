#include "ResultScreen.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "BulletManager.hpp"
#include "Chain.hpp"
#include "ChainPriorities.hpp"
#include "Controller.hpp"
#include "FileSystem.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "Rng.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "ZunMath.hpp"
#include "i18n.hpp"
#include "utils.hpp"
// #include <direct.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>

namespace th06
{

DIFFABLE_STATIC_ARRAY_ASSIGN(f32, 5, g_DifficultyWeightsList) = {-30.0f, -10.0f, 20.0f, 30.0f, 30.0f};

DIFFABLE_STATIC_ASSIGN(u32, g_DefaultMagic) = 'DMYS';

// EoSD assumes every character in this array is a single byte, which is a safe assumption in SJIS, but not
//   in UTF-8, so we have to encode '･' with an escape sequence
DIFFABLE_STATIC_ASSIGN(const char *, g_AlphabetList) =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ.,:;\xA5@abcdefghijklmnopqrstuvwxyz+-/*=%0123456789(){}[]<>#!?'\"$      --";

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 6, g_CharacterList) = {TH_HAKUREI_REIMU_SPIRIT,  TH_HAKUREI_REIMU_DREAM,
                                                            TH_KIRISAME_MARISA_DEVIL, TH_KIRISAME_MARISA_LOVE,
                                                            TH_SATSUKI_RIN_FLOWER,    TH_SATSUKI_RIN_WIND};

DIFFABLE_STATIC_ARRAY_ASSIGN(f32, 5, g_SpellcardsWeightsList) = {1.0f, 1.5f, 1.5f, 2.0f, 2.5f};

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 5, g_RightAlignedDifficultyList) = {"     Easy", "   Normal", "     Hard",
                                                                         "  Lunatic", "    Extra"};

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 4, g_ShortCharacterList2) = {"ReimuA ", "ReimuB ", "MarisaA", "MarisaB"};

#define DEFAULT_HIGH_SCORE_NAME "Nanashi "

ScoreDat *ResultScreen::OpenScore(const char *path)
{
    u8 *bytes;
    i32 bytesShifted;
    i32 fileLen;
    Th6k *decryptedFilePointer;
    i32 remainingData;
    i32 scoreListNodeSize;
    u16 checksum;
    u8 xorValue;
    i32 scoreDatSize;
    ScoreRaw *scoreRaw;
    ScoreDat *scoreDat;

    scoreDat = (ScoreDat *)malloc(sizeof(ScoreDat));
    scoreRaw = (ScoreRaw *)FileSystem::OpenPath(path);
    if (scoreRaw == NULL)
    {
    FAILED_TO_READ:
        scoreDatSize = sizeof(ScoreRaw);
        scoreRaw = (ScoreRaw *)std::malloc(scoreDatSize);
        scoreRaw->dataOffset = sizeof(ScoreRaw);
        scoreRaw->fileLen = sizeof(ScoreRaw);
    }
    else
    {
        if (g_LastFileSize < sizeof(ScoreRaw))
        {
            free(scoreRaw);
            goto FAILED_TO_READ;
        }

        remainingData = g_LastFileSize - 2;
        checksum = 0;
        xorValue = 0;
        bytesShifted = 0;
        bytes = &scoreRaw->xorseed[1];

        while (0 < remainingData)
        {

            xorValue += bytes[0];
            // Invert top 3 bits and bottom 5 bits
            xorValue = (xorValue & 0xe0) >> 5 | (xorValue & 0x1f) << 3;
            // xor one byte later with the resulting inverted bits
            bytes[1] ^= xorValue;
            if (bytesShifted >= 2)
            {
                checksum += bytes[1];
            }
            bytes++;
            remainingData--;
            bytesShifted++;
        }
        if (scoreRaw->csum != checksum)
        {
            free(scoreRaw);
            goto FAILED_TO_READ;
        }
        fileLen = scoreRaw->fileLen;
        decryptedFilePointer = scoreRaw->ShiftBytes(scoreRaw->dataOffset);
        fileLen -= scoreRaw->dataOffset;
        while (fileLen > 0)
        {
            if (decryptedFilePointer->magic == TH6K_MAGIC)
                break;

            decryptedFilePointer = decryptedFilePointer->ShiftBytes(decryptedFilePointer->th6kLen);
            fileLen = fileLen - decryptedFilePointer->th6kLen;
        }
        if (fileLen <= 0)
        {
            free(scoreRaw);
            goto FAILED_TO_READ;
        };
    }

    scoreDat->rawScoreFile = scoreRaw;

    scoreListNodeSize = sizeof(ScoreListNode);
    scoreDat->scores = (ScoreListNode *)std::malloc(scoreListNodeSize);
    scoreDat->scores->next = NULL;
    scoreDat->scores->data = NULL;
    scoreDat->scores->prev = NULL;
    return scoreDat;
}

u32 ResultScreen::GetHighScore(ScoreDat *scoreDat, ScoreListNode *node, u32 character, u32 difficulty)
{
    u32 score;
    u32 dataScore;
    i32 remainingSize;
    Hscr *highScore;
    ScoreRaw *scoreHeader;

    scoreHeader = scoreDat->rawScoreFile;

    if (node == NULL)
    {
        ResultScreen::FreeAllScores(scoreDat->scores);
        scoreDat->scores->next = NULL;
        scoreDat->scores->data = NULL;
        scoreDat->scores->prev = NULL;
    }

    remainingSize = scoreHeader->fileLen;
    highScore = (Hscr *)scoreHeader->ShiftBytes(scoreHeader->dataOffset);
    remainingSize -= scoreHeader->dataOffset;

    while (remainingSize > 0)
    {
        if (highScore->base.magic == HSCR_MAGIC && highScore->base.version == TH6K_VERSION &&
            highScore->character == character && highScore->difficulty == difficulty)
        {
            if (node != NULL)
            {
                ResultScreen::LinkScore(node, highScore);
            }
            else
            {
                ResultScreen::LinkScore(scoreDat->scores, highScore);
            }
        }

        remainingSize -= highScore->base.th6kLen;
        highScore = highScore->ShiftBytes(highScore->base.th6kLen);
    }
    if (scoreDat->scores->next != NULL)
    {
        if (scoreDat->scores->next->data->score > 1000000)
        {
            dataScore = scoreDat->scores->next->data->score;
        }
        else
        {
            dataScore = 1000000;
        }
        score = dataScore;
    }
    else
    {
        score = 1000000;
    }
    return score;
}

i32 ResultScreen::LinkScore(ScoreListNode *prevNode, Hscr *newScore)
{
    i32 scoresAmount;
    ScoreListNode *nextNode;
    i32 scoreNodeSize;

    scoresAmount = 0;
    while (prevNode->next != NULL)
    {
        if (prevNode->next->data != NULL && prevNode->next->data->score <= newScore->score)
        {
            break;
        }
        prevNode = prevNode->next;
        scoresAmount++;
    }
    nextNode = prevNode->next;
    scoreNodeSize = sizeof(ScoreListNode);

    prevNode->next = (ScoreListNode *)std::malloc(scoreNodeSize);
    prevNode->next->prev = prevNode;
    prevNode = prevNode->next;
    prevNode->data = newScore;
    prevNode->next = nextNode;
    return scoresAmount;
}

void ResultScreen::FreeAllScores(ScoreListNode *scores)
{
    ScoreListNode *next;
    scores = scores->next;
    while (scores != NULL)
    {
        next = scores->next;
        free(scores);
        scores = next;
    }
}

bool ResultScreen::ParseCatk(ScoreDat *scoreDat, Catk *outCatk)
{

    i32 cursor;
    Catk *parsedCatk;
    ScoreRaw *header;
    header = scoreDat->rawScoreFile;

    if (outCatk == NULL)
    {
        return false;
    }

    parsedCatk = (Catk *)header->ShiftBytes(header->dataOffset);
    cursor = header->fileLen - header->dataOffset;
    while (cursor > 0)
    {
        if (parsedCatk->base.magic == CATK_MAGIC && parsedCatk->base.version == TH6K_VERSION)
        {
            if (parsedCatk->idx >= CATK_NUM_CAPTURES)
                break;

            outCatk[parsedCatk->idx] = *parsedCatk;
        }
        cursor -= parsedCatk->base.th6kLen;
        parsedCatk = (Catk *)&parsedCatk->name[parsedCatk->base.th6kLen - 0x18];
    }
    return true;
}

bool ResultScreen::ParseClrd(ScoreDat *scoreDat, Clrd *outClrd)
{
    i32 cursor;
    Clrd *parsedClrd;
    ScoreRaw *header;
    i32 characterShotType;
    i32 difficulty;
    header = scoreDat->rawScoreFile;

    if (outClrd == NULL)
    {
        return false;
    }

    for (characterShotType = 0; characterShotType < CLRD_NUM_CHARACTERS; characterShotType++)
    {
        memset(&outClrd[characterShotType], 0, sizeof(Clrd));

        outClrd[characterShotType].base.magic = CLRD_MAGIC;
        outClrd[characterShotType].base.unkLen = sizeof(Clrd);
        outClrd[characterShotType].base.th6kLen = sizeof(Clrd);
        outClrd[characterShotType].base.version = TH6K_VERSION;
        outClrd[characterShotType].characterShotType = characterShotType;

        for (difficulty = 0; difficulty < ARRAY_SIZE_SIGNED(outClrd[0].difficultyClearedWithoutRetries); difficulty++)
        {
            outClrd[characterShotType].difficultyClearedWithRetries[difficulty] = 1;
            outClrd[characterShotType].difficultyClearedWithoutRetries[difficulty] = 1;
        }
    }

    parsedClrd = (Clrd *)header->ShiftBytes(header->dataOffset);
    cursor = header->fileLen - header->dataOffset;
    while (cursor > 0)
    {
        if (parsedClrd->base.magic == CLRD_MAGIC && parsedClrd->base.version == TH6K_VERSION)
        {
            if (parsedClrd->characterShotType >= CLRD_NUM_CHARACTERS)
                break;

            outClrd[parsedClrd->characterShotType] = *parsedClrd;
        }
        cursor -= parsedClrd->base.th6kLen;
        parsedClrd = (Clrd *)(((u8 *)&parsedClrd->base) + parsedClrd->base.th6kLen);
    }
    return true;
}

bool ResultScreen::ParsePscr(ScoreDat *scoreDat, Pscr *outClrd)
{
    i32 cursor;
    Pscr *parsedPscr;
    ScoreRaw *header;
    i32 stage;
    i32 character;
    i32 difficulty;
    header = scoreDat->rawScoreFile;
    Pscr *pscr;

    if (outClrd == NULL)
    {
        return false;
    }

    for (pscr = outClrd, character = 0; character < PSCR_NUM_CHARS_SHOTTYPES; character++)
    {
        for (stage = 0; stage < PSCR_NUM_STAGES; stage++)
        {
            for (difficulty = 0; difficulty < PSCR_NUM_DIFFICULTIES; difficulty++, pscr++)
            {

                std::memset(pscr, 0, sizeof(Pscr));

                pscr->base.magic = PSCR_MAGIC;
                pscr->base.unkLen = sizeof(Pscr);
                pscr->base.th6kLen = sizeof(Pscr);
                pscr->base.version = 16;
                pscr->character = character;
                pscr->difficulty = difficulty;
                pscr->stage = stage;
            }
        }
    }

    parsedPscr = (Pscr *)header->ShiftBytes(header->dataOffset);
    cursor = header->fileLen - header->dataOffset;

    while (cursor > 0)
    {
        if (parsedPscr->base.magic == PSCR_MAGIC && parsedPscr->base.version == TH6K_VERSION)
        {
            pscr = parsedPscr;
            if (pscr->character >= PSCR_NUM_CHARS_SHOTTYPES || pscr->difficulty >= PSCR_NUM_DIFFICULTIES + 1 ||
                pscr->stage >= PSCR_NUM_STAGES + 1)
                break;

            outClrd[pscr->character * 6 * 4 + pscr->stage * 4 + pscr->difficulty] = *pscr;
        }
        cursor -= parsedPscr->base.th6kLen;
        parsedPscr = parsedPscr->ShiftBytes(parsedPscr->base.th6kLen);
    }
    return true;
}

void ResultScreen::ReleaseScoreDat(ScoreDat *scoreDat)
{
    ScoreListNode *scores;
    ResultScreen::FreeAllScores(scoreDat->scores);
    scores = scoreDat->scores;
    free(scores);
    free(scoreDat->rawScoreFile);
    free(scoreDat);
}

void ResultScreen::WriteScore(ResultScreen *resultScreen)
{

    u8 *fileBuffer;
    u8 originalByte;
    i32 fileBufferSize;
    ScoreRaw *scoreRaw;
    i32 characterSlot;
    u8 xorValue;
    i32 remainingSize;
    i32 shotType;
    i32 stage;
    Pscr *pscr;
    Catk *catk;
    Clrd *clrd;
    i32 character;
    ScoreListNode *currentCharacter;
    i32 sizeOfFile;
    u8 *bytes;
    i32 difficulty;

    sizeOfFile = 0;

    fileBufferSize = SCORE_DAT_FILE_BUFFER_SIZE;
    fileBuffer = (u8 *)malloc(fileBufferSize);

    std::memcpy(fileBuffer + sizeOfFile, resultScreen->scoreDat->rawScoreFile, sizeof(ScoreRaw));

    sizeOfFile += sizeof(ScoreRaw);
    resultScreen->fileHeader.magic = TH6K_MAGIC;
    resultScreen->fileHeader.unkLen = sizeof(Th6k);
    resultScreen->fileHeader.th6kLen = sizeof(Th6k);
    resultScreen->fileHeader.version = TH6K_VERSION;

    std::memcpy(fileBuffer + sizeOfFile, &resultScreen->fileHeader, sizeof(Th6k));
    sizeOfFile += sizeof(Th6k);

    for (difficulty = 0; difficulty < HSCR_NUM_DIFFICULTIES; difficulty++)
    {

        for (character = 0; character < HSCR_NUM_CHARS_SHOTTYPES; character++)
        {
            currentCharacter = resultScreen->scores[difficulty][character].next;
            characterSlot = 0;
            for (;;)
            {
                if (currentCharacter != NULL)
                {

                    if (currentCharacter->data->base.magic == HSCR_MAGIC)
                    {
                        currentCharacter->data->character = character;
                        currentCharacter->data->difficulty = difficulty;
                        currentCharacter->data->base.unkLen = sizeof(Hscr);
                        currentCharacter->data->base.th6kLen = sizeof(Hscr);
                        currentCharacter->data->base.version = TH6K_VERSION;
                        currentCharacter->data->base.unk_9 = 0;
                        std::memcpy(fileBuffer + sizeOfFile, currentCharacter->data, sizeof(Hscr));
                        sizeOfFile += sizeof(Hscr);
                    }
                    currentCharacter = currentCharacter->next;
                    characterSlot++;

                    if (characterSlot >= HSCR_NUM_SCORES_SLOTS)
                    {
                        break;
                    }
                    else
                    {
                        continue;
                    }
                };
                break;
            };
        }
    };

    clrd = g_GameManager.clrd;
    for (difficulty = 0; difficulty < CLRD_NUM_CHARACTERS; difficulty++, clrd++)
    {
        clrd->base.magic = CLRD_MAGIC;
        clrd->base.unkLen = sizeof(Clrd);
        clrd->base.th6kLen = sizeof(Clrd);
        clrd->base.version = TH6K_VERSION;
        std::memcpy(fileBuffer + sizeOfFile, clrd, sizeof(Clrd));

        sizeOfFile += sizeof(Clrd);
    }
    catk = &g_GameManager.catk[0];
    for (difficulty = 0; difficulty < CATK_NUM_CAPTURES; difficulty++, catk++)
    {
        if (catk->base.magic == CATK_MAGIC)
        {
            catk->idx = difficulty;
            catk->base.unkLen = sizeof(Catk);
            catk->base.th6kLen = sizeof(Catk);
            catk->base.version = TH6K_VERSION;
            std::memcpy(fileBuffer + sizeOfFile, catk, sizeof(Catk));
            sizeOfFile += sizeof(Catk);
        }
    }
    pscr = &g_GameManager.pscr[0][0][0];
    for (difficulty = 0; difficulty < PSCR_NUM_DIFFICULTIES; difficulty++)
    {
        for (stage = 0; stage < PSCR_NUM_STAGES; stage++)
        {
            for (shotType = 0; shotType < PSCR_NUM_CHARS_SHOTTYPES; shotType++, pscr++)
            {
                if (pscr->score != 0)
                {
                    std::memcpy(fileBuffer + sizeOfFile, pscr, sizeof(Pscr));
                    sizeOfFile += sizeof(Pscr);
                }
            }
        }
    }
    scoreRaw = (ScoreRaw *)fileBuffer;
    scoreRaw->dataOffset = sizeof(Pscr);
    scoreRaw->fileLen = sizeOfFile;
    scoreRaw->csum = 0;

    scoreRaw->xorseed[1] = g_Rng.GetRandomU16InRange(0x100);
    scoreRaw->unk[0] = g_Rng.GetRandomU16InRange(0x100);
    scoreRaw->unk_8 = 0x10;

    for (remainingSize = 4; remainingSize < sizeOfFile; remainingSize++)
    {
        scoreRaw->csum += fileBuffer[remainingSize];
    }
    xorValue = 0;
    originalByte = 0;

    bytes = (u8 *)scoreRaw->ShiftOneByte();
    remainingSize = sizeOfFile;

    remainingSize -= 2;
    xorValue = bytes[0];

    while (remainingSize > 0)
    {
        originalByte = bytes[1];
        xorValue = (xorValue & 0xe0) >> 5 | (xorValue & 0x1f) << 3;
        bytes[1] ^= xorValue;
        xorValue += originalByte;
        bytes++;
        remainingSize--;
    }
    FileSystem::WriteDataToFile("score.dat", fileBuffer, sizeOfFile);
    std::free(fileBuffer);
}

i32 ResultScreen::LinkScoreEx(Hscr *out, i32 difficulty, i32 character)
{
    return ResultScreen::LinkScore(&this->scores[difficulty][character], out);
}

void ResultScreen::FreeScore(i32 difficulty, i32 character)
{
    ResultScreen::FreeAllScores(&this->scores[difficulty][character]);
}

i32 ResultScreen::HandleResultKeyboard()
{
    i32 idx;
    AnmVm *sprite;
    i32 replayNameIdx;
    i32 replayNameIdx2;

    if (this->frameTimer == 0)
    {
        this->charUsed = g_GameManager.character;
        this->diffSelected = g_GameManager.difficulty;

        sprite = &this->unk_40[0];
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
        {
            sprite->pendingInterrupt = this->diffSelected + 3;
        }

        AnmManager::DrawStringFormat2(g_AnmManager, this->unk_28a0, COLOR_RGB(COLOR_WHITE), COLOR_RGB(COLOR_BLACK),
                                      g_CharacterList[this->charUsed * 2]);
        if (g_GameManager.shotType != SHOT_TYPE_A)
        {
            this->unk_28a0[0].color = COLOR_TRANSPARENT_WHITE;
        }

        AnmManager::DrawStringFormat2(g_AnmManager, &this->unk_28a0[1], COLOR_RGB(COLOR_WHITE), COLOR_RGB(COLOR_BLACK),
                                      g_CharacterList[this->charUsed * 2]);
        if (g_GameManager.shotType != SHOT_TYPE_B)
        {
            this->unk_28a0[1].color = COLOR_TRANSPARENT_WHITE;
        }

        this->hscr.character = this->charUsed * 2 + g_GameManager.shotType;
        this->hscr.difficulty = this->diffSelected;
        this->hscr.score = g_GameManager.score;
        this->hscr.base.version = 16;
        this->hscr.base.magic = *(i32 *)"HSCR";

        if (g_GameManager.isGameCompleted == 0)
        {
            this->hscr.stage = g_GameManager.currentStage;
        }
        else
        {
            this->hscr.stage = 99;
        }

        this->hscr.base.unk_9 = 1;
        std::strcpy(this->hscr.name, "        ");

        if (this->LinkScoreEx(&this->hscr, this->diffSelected, this->charUsed * 2 + g_GameManager.shotType) >= 10)
            goto RETURN_TO_STATS_SCREEN_WITHOUT_SOUND;

        this->cursor = 0;
        std::strcpy(this->replayName, "");
    }
    if (this->frameTimer < 30)
    {
        return 0;
    }
    if (WAS_PRESSED_PERIODIC(TH_BUTTON_UP))
    {
        for (;;)
        {
            this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;

            if (this->selectedCharacter < 0)
            {
                this->selectedCharacter += RESULT_KEYBOARD_CHARACTERS;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                continue;
            }
            break;
        };
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
    }
    if (WAS_PRESSED_PERIODIC(TH_BUTTON_DOWN))
    {
        for (;;)
        {
            this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;

            if (this->selectedCharacter >= RESULT_KEYBOARD_CHARACTERS)
            {
                this->selectedCharacter -= RESULT_KEYBOARD_CHARACTERS;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                continue;
            }
            break;
        };
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
    }
    if (WAS_PRESSED_PERIODIC(TH_BUTTON_LEFT))
    {
        for (;;)
        {
            this->selectedCharacter--;
            if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == RESULT_KEYBOARD_COLUMNS - 1)
            {
                this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;
            }

            if (this->selectedCharacter < 0)
            {
                this->selectedCharacter = RESULT_KEYBOARD_COLUMNS - 1;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                continue;
            }
            break;
        };
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
    }
    if (WAS_PRESSED_PERIODIC(TH_BUTTON_RIGHT))
    {
        for (;;)
        {
            this->selectedCharacter++;

            if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == 0)
            {
                this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;
            }

            if (g_AlphabetList[this->selectedCharacter] == ' ')
            {
                continue;
            }
            break;
        };
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
    }
    if (WAS_PRESSED_PERIODIC(TH_BUTTON_SELECTMENU))
    {
        replayNameIdx = this->cursor >= 8 ? 7 : this->cursor;

        if (this->selectedCharacter < RESULT_KEYBOARD_SPACE)
        {
            this->hscr.name[replayNameIdx] = g_AlphabetList[this->selectedCharacter];
        }
        else if (this->selectedCharacter == RESULT_KEYBOARD_SPACE)
        {
            this->hscr.name[replayNameIdx] = ' ';
        }
        else
        {
            goto RETURN_TO_STATS_SCREEN;
        }

        if (this->cursor < 8)
        {
            this->cursor++;
            if (this->cursor == 8)
            {
                this->selectedCharacter = RESULT_KEYBOARD_END;
            }
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
    }

    if (WAS_PRESSED_PERIODIC(TH_BUTTON_RETURNMENU))
    {
        replayNameIdx2 = this->cursor >= 8 ? 7 : this->cursor;

        if (this->cursor > 0)
        {
            this->cursor--;
            this->hscr.name[replayNameIdx2] = ' ';
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
    }
    if (WAS_PRESSED(TH_BUTTON_MENU))
    {
    RETURN_TO_STATS_SCREEN:
        g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);

    RETURN_TO_STATS_SCREEN_WITHOUT_SOUND:

        this->resultScreenState = RESULT_SCREEN_STATE_STATS_SCREEN;
        this->frameTimer = 0;

        sprite = &this->unk_40[0];
        for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
        {
            sprite->pendingInterrupt = 2;
        }
        std::strcpy(this->replayName, this->hscr.name);
    }
    return 0;
}

i32 ResultScreen::HandleReplaySaveKeyboard()
{
    AnmVm *sprite;
    i32 replayNameCharacter2;
    char replayPath[64];
    i32 replayNameCharacter;
    char replayToReadPath[64];
    ReplayHeader *replayLoaded;
    i32 idx;
    i32 saveInterrupt;
    std::filesystem::path dirPath;
    std::error_code fileError;
    std::time_t time;
    std::tm *tm;

    time = std::time(NULL);
    tm = std::localtime(&time);

    switch (this->resultScreenState)
    {
    case RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION:
        if (this->frameTimer == 60)
        {
            if (g_GameManager.numRetries != 0)
            {
                saveInterrupt = 0xc;
            }
            else
            {
                if (g_Supervisor.framerateMultiplier < 0.99f)
                {
                    saveInterrupt = 0xd;
                }
                else
                {
                    saveInterrupt = 9;
                }
            }
            sprite = &this->unk_40[1];
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
            {
                sprite->pendingInterrupt = saveInterrupt;
            }
            if (saveInterrupt != 9)
            {
                this->resultScreenState = RESULT_SCREEN_STATE_CANT_SAVE_REPLAY;
            }
            this->cursor = 0;
        }
        sprite = &this->unk_40[16];
        if (this->cursor == 0)
        {
            sprite[0].color = COLOR_COMBINE_ALPHA(COLOR_PASTEL_RED, sprite[0].color);
            sprite[1].color = COLOR_COMBINE_ALPHA(COLOR_ASHEN_GREY, sprite[1].color);
        }
        else
        {
            sprite[0].color = COLOR_COMBINE_ALPHA(COLOR_ASHEN_GREY, sprite[0].color);
            sprite[1].color = COLOR_COMBINE_ALPHA(COLOR_PASTEL_RED, sprite[1].color);
        }
        if (this->frameTimer < 80)
        {
            return 0;
        }
        ResultScreen::MoveCursorHorizontally(this, 2);
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU) || WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto EXIT_WITH_SOUND;
        }
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {

            if (this->cursor == 0)
            {
            GO_TO_CHOOSE_REPLAY_FILE:

                g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
                this->resultScreenState = RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE;

                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 0xa;
                }

                this->frameTimer = 0;
                goto CHOOSE_REPLAY_FILE;
            }

        EXIT_WITH_SOUND:

            this->frameTimer = 0;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            this->resultScreenState = RESULT_SCREEN_STATE_EXITING;
            sprite = &this->unk_40[0];
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
            {
                sprite->pendingInterrupt = 2;
            }
        }
        break;
    case RESULT_SCREEN_STATE_CANT_SAVE_REPLAY:

        if (this->frameTimer < 20)
        {
            return 0;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU) || WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {

            this->frameTimer = 0;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            this->resultScreenState = RESULT_SCREEN_STATE_EXITING;
            sprite = &this->unk_40[0];
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
            {
                sprite->pendingInterrupt = 2;
            }
        }
        break;

    case RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE:

    CHOOSE_REPLAY_FILE:

        if (this->frameTimer == 0)
        {
            dirPath = std::filesystem::path("./replay");
            std::filesystem::create_directory(dirPath, fileError);

            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->replays); idx++)
            {
                std::sprintf(replayToReadPath, "./replay/th6_%.2d.rpy", idx + 1);
                replayLoaded = (ReplayHeader *)FileSystem::OpenPath(replayToReadPath);
                if (replayLoaded == NULL)
                {
                    continue;
                }

                if (ReplayManager::ValidateReplayData(replayLoaded, g_LastFileSize))
                {
                    this->replays[idx] = *replayLoaded;
                }
                std::free(replayLoaded);
            }
        }

        if (this->frameTimer < 20)
        {
            return 0;
        }

        MoveCursor(this, 15);
        this->replayNumber = this->cursor;
        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
            this->replayNumber = this->cursor;
            this->frameTimer = 0;
            sprintf(this->defaultReplay.date, "%02i/%02i/%02i", tm->tm_mon, tm->tm_mday, tm->tm_year % 100);
            (this->defaultReplay).score = g_GameManager.score;
            if (*(i32 *)&this->replays[this->cursor].magic != *(i32 *)&"T6RP" ||
                this->replays[this->cursor].version != GAME_VERSION)
            {
                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 0xf;
                }
                sprite = &this->unk_40[this->replayNumber + 0x16];
                sprite->pendingInterrupt = 0xe;
                this->resultScreenState = RESULT_SCREEN_STATE_WRITING_REPLAY_NAME;
            }
            else
            {
                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 0xb;
                }
                sprite = &this->unk_40[this->replayNumber + 0x16];
                sprite->pendingInterrupt = 0xe;
                this->resultScreenState = RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE;
            }
            this->cursor = 0;
            this->selectedCharacter = 0;
        }
        if (WAS_PRESSED(10))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            this->resultScreenState = RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION;
            sprite = &this->unk_40[0];
            for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
            {
                sprite->pendingInterrupt = 2;
            }
            this->frameTimer = 0;
        }
        break;
    case RESULT_SCREEN_STATE_WRITING_REPLAY_NAME:
        if (this->frameTimer < 30)
        {
            return 0;
        }
        if (WAS_PRESSED_PERIODIC(TH_BUTTON_UP))
        {
            for (;;)
            {
                this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;

                if (this->selectedCharacter < 0)
                {
                    this->selectedCharacter += RESULT_KEYBOARD_CHARACTERS;
                }

                if (g_AlphabetList[this->selectedCharacter] == ' ')
                {
                    continue;
                }
                break;
            };
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
        }
        if (WAS_PRESSED_PERIODIC(TH_BUTTON_DOWN))
        {
            for (;;)
            {
                this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;

                if (this->selectedCharacter >= RESULT_KEYBOARD_CHARACTERS)
                {
                    this->selectedCharacter -= RESULT_KEYBOARD_CHARACTERS;
                }

                if (g_AlphabetList[this->selectedCharacter] == ' ')
                {
                    continue;
                }
                break;
            };
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
        }
        if (WAS_PRESSED_PERIODIC(TH_BUTTON_LEFT))
        {
            for (;;)
            {
                this->selectedCharacter--;
                if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == RESULT_KEYBOARD_COLUMNS - 1)
                {
                    this->selectedCharacter += RESULT_KEYBOARD_COLUMNS;
                }

                if (this->selectedCharacter < 0)
                {
                    this->selectedCharacter = RESULT_KEYBOARD_COLUMNS - 1;
                }

                if (g_AlphabetList[this->selectedCharacter] == ' ')
                {
                    continue;
                }
                break;
            };
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
        }
        if (WAS_PRESSED_PERIODIC(TH_BUTTON_RIGHT))
        {
            for (;;)
            {
                this->selectedCharacter++;
                if (this->selectedCharacter % RESULT_KEYBOARD_COLUMNS == 0)
                {
                    this->selectedCharacter -= RESULT_KEYBOARD_COLUMNS;
                }

                if (g_AlphabetList[this->selectedCharacter] == ' ')
                {
                    continue;
                }
                break;
            };
            g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
        }
        if (WAS_PRESSED_PERIODIC(TH_BUTTON_SELECTMENU))
        {

            replayNameCharacter = this->cursor >= 8 ? 7 : this->cursor;

            if (this->selectedCharacter < RESULT_KEYBOARD_SPACE)
            {
                this->replayName[replayNameCharacter] = g_AlphabetList[this->selectedCharacter];
            }
            else if (this->selectedCharacter == RESULT_KEYBOARD_SPACE)
            {
                this->replayName[replayNameCharacter] = ' ';
            }
            else
            {
                std::sprintf(replayPath, "./replay/th6_%.2d.rpy", this->replayNumber + 1);
                ReplayManager::SaveReplay(replayPath, this->replayName);
                this->frameTimer = 0;
                this->resultScreenState = RESULT_SCREEN_STATE_EXITING;
                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 2;
                }
            }
            if (this->cursor < 8)
            {
                this->cursor++;
                if (this->cursor == 8)
                {
                    this->selectedCharacter = RESULT_KEYBOARD_END;
                }
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_SELECT);
        }

        if (WAS_PRESSED_PERIODIC(TH_BUTTON_RETURNMENU))
        {
            replayNameCharacter2 = this->cursor >= 8 ? 7 : this->cursor;

            if (this->cursor > 0)
            {
                this->cursor--;
                this->replayName[replayNameCharacter2] = ' ';
            }
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
        }
        if (WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto GO_TO_CHOOSE_REPLAY_FILE;
        }
        break;

    case RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE:
        sprite = &this->unk_40[16];
        if (this->cursor == 0)
        {
            sprite[0].color = COLOR_COMBINE_ALPHA(COLOR_PASTEL_RED, sprite[0].color);
            sprite[1].color = COLOR_COMBINE_ALPHA(COLOR_ASHEN_GREY, sprite[1].color);
        }
        else
        {
            sprite[0].color = COLOR_COMBINE_ALPHA(COLOR_ASHEN_GREY, sprite[0].color);
            sprite[1].color = COLOR_COMBINE_ALPHA(COLOR_PASTEL_RED, sprite[1].color);
        }

        if (this->frameTimer < 20)
        {
            return 0;
        }
        MoveCursorHorizontally(this, 2);

        if (WAS_PRESSED(TH_BUTTON_RETURNMENU) || WAS_PRESSED(TH_BUTTON_MENU))
        {
            goto GO_TO_CHOOSE_REPLAY_FILE;
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {

            this->frameTimer = 0;
            if (this->cursor == 0)
            {
                sprite = &this->unk_40[0];
                for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->unk_40); idx++, sprite++)
                {
                    sprite->pendingInterrupt = 15;
                }
                sprite = &this->unk_40[this->replayNumber + 22];
                sprite->pendingInterrupt = 14;
                this->resultScreenState = RESULT_SCREEN_STATE_WRITING_REPLAY_NAME;
                break;
            }
            goto GO_TO_CHOOSE_REPLAY_FILE;
        }
    }
    return 0;
}

void ResultScreen::MoveCursor(ResultScreen *resultScreen, i32 length)
{
    if (WAS_PRESSED_PERIODIC(TH_BUTTON_UP))
    {
        resultScreen->cursor--;
        if (resultScreen->cursor < 0)
        {
            resultScreen->cursor += length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
    }
    if (WAS_PRESSED_PERIODIC(TH_BUTTON_DOWN))
    {
        resultScreen->cursor++;
        if (resultScreen->cursor >= length)
        {
            resultScreen->cursor -= length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
    }
}

bool ResultScreen::MoveCursorHorizontally(ResultScreen *resultScreen, i32 length)
{
    if (WAS_PRESSED_PERIODIC(TH_BUTTON_LEFT))
    {
        resultScreen->cursor--;
        if (resultScreen->cursor < 0)
        {
            resultScreen->cursor += length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
        return true;
    }
    else if (WAS_PRESSED_PERIODIC(TH_BUTTON_RIGHT))
    {
        resultScreen->cursor++;
        if (resultScreen->cursor >= length)
        {
            resultScreen->cursor -= length;
        }
        g_SoundPlayer.PlaySoundByIdx(SOUND_MOVE_MENU);
        return true;
    }
    else
    {
        return false;
    }
}

bool ResultScreen::CheckConfirmButton()
{
    AnmVm *viewport;

    switch (this->resultScreenState)
    {
    case RESULT_SCREEN_STATE_STATS_SCREEN:
        if (this->frameTimer <= 30)
        {
            viewport = &this->unk_40[37];
            viewport->pendingInterrupt = 16;
        }
        if (this->frameTimer >= 90 && WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            viewport = &this->unk_40[37];
            viewport->pendingInterrupt = 2;
            this->frameTimer = 0;
            this->resultScreenState = RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION;
        }
        break;

    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:
        if (this->frameTimer >= 30)
        {
            this->frameTimer = 59;
            this->resultScreenState = RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION;
        }
        break;
    }
    return true;
}

u32 ResultScreen::DrawFinalStats()
{
    f32 completion;
    f32 unknownFloat;
    ZunVec3 strPos;
    AnmVm *viewport;
    i32 color;
    f32 slowdownRate;

    switch (this->resultScreenState)
    {
    case RESULT_SCREEN_STATE_STATS_SCREEN:
    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:

        viewport = &this->unk_40[37];
        color = viewport->color;
        g_AsciiManager.color = color;
        unknownFloat = 0.0;

        completion = g_GameManager.difficulty < 4 ? g_GameManager.counat / 39600.0f : g_GameManager.counat / 89500.0f;
        strPos = viewport->pos;
        strPos.x += 224.0f;
        strPos.y += 32.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.score);

        if (g_GameManager.guiScore < 2000000)
        {
            unknownFloat -= 20.0f;
        }
        else if (g_GameManager.guiScore < 200000000)
        {
            unknownFloat += (g_GameManager.guiScore - 2000000) / 198000000.0f * 60.0f - 20.0f;
        }
        else
        {
            unknownFloat += 40.0f;
        }

        strPos.y += 22.0f;
        g_AsciiManager.AddString(&strPos, g_RightAlignedDifficultyList[g_GameManager.difficulty]);

        unknownFloat += g_DifficultyWeightsList[g_GameManager.difficulty];
        strPos.y += 22.0f;
        if (g_GameManager.difficulty == EASY || !g_GameManager.isGameCompleted)
        {
            g_AsciiManager.AddFormatText(&strPos, "    %3.2f%%", completion * 100.0f);
            unknownFloat += completion * 70.0f;
        }
        else
        {
            g_AsciiManager.AddFormatText(&strPos, "      100%%");
            unknownFloat += 70.0f;
        }
        strPos.y += 22.0f;
        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.numRetries);

        unknownFloat -= g_GameManager.numRetries * 10.0f;
        strPos.y += 22.0f;

        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.deaths);

        unknownFloat -= g_GameManager.deaths * 5.0f - 10.0f;

        strPos.y += 22.0f;

        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.bombsUsed);

        unknownFloat -= g_GameManager.bombsUsed * 2.0f - 10.0f;
        strPos.y += 22.0f;

        g_AsciiManager.AddFormatText(&strPos, "%9d", g_GameManager.spellcardsCaptured);

        unknownFloat += g_GameManager.spellcardsCaptured * g_SpellcardsWeightsList[g_GameManager.difficulty];

        slowdownRate = (g_Supervisor.unk1b4 / g_Supervisor.unk1b8 - 0.5f) * 2;

        if (slowdownRate < 0.0f)
        {
            slowdownRate = 0.0f;
        }
        else if (slowdownRate >= 1.0f)
        {
            slowdownRate = 1.0f;
        }

        slowdownRate = (1 - slowdownRate) * 100.0f;

        strPos.y += 22.0f;
        g_AsciiManager.AddFormatText(&strPos, "    %3.2f%%", slowdownRate);

        if (slowdownRate < 50.0f)
        {
            unknownFloat -= 70.0f * slowdownRate / 100.0f;
        }
        else
        {
            unknownFloat = -999.0f;
        }
        // Useless calculations, maybe in earlier versions it showed the point items and graze, but it was later
        // removed? unknowFloat is also unused, maybe it was some kind of grading system
        if (g_GameManager.pointItemsCollected < 800)
        {
            unknownFloat += 0.01f * g_GameManager.pointItemsCollected;
        }
        else
        {
            unknownFloat += 8.0f;
        }

        if (g_GameManager.grazeInTotal < 5000)
        {
            unknownFloat += 0.0025f * g_GameManager.grazeInTotal;
        }
        else
        {
            unknownFloat += 12.5f;
        }

        g_AsciiManager.color = COLOR_WHITE;
    }
    return 0;
}

bool ResultScreen::RegisterChain(i32 unk)
{
    ResultScreen *resultScreen;
    resultScreen = new ResultScreen();

    utils::DebugPrint(TH_DBG_RESULTSCREEN_COUNAT, g_GameManager.counat);

    resultScreen->calcChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnUpdate);
    resultScreen->calcChain->addedCallback = (ChainAddedCallback)ResultScreen::AddedCallback;
    resultScreen->calcChain->deletedCallback = (ChainDeletedCallback)ResultScreen::DeletedCallback;
    resultScreen->calcChain->arg = resultScreen;

    if (unk != 0)
    {
        if (!g_GameManager.isInPracticeMode)
        {
            resultScreen->resultScreenState = RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME;
        }
        else
        {
            resultScreen->resultScreenState = RESULT_SCREEN_STATE_EXIT;
        }
    }

    if (!g_Chain.AddToCalcChain(resultScreen->calcChain, TH_CHAIN_PRIO_CALC_RESULTSCREEN))
    {
        return false;
    }

    resultScreen->drawChain = g_Chain.CreateElem((ChainCallback)ResultScreen::OnDraw);
    resultScreen->drawChain->arg = resultScreen;
    g_Chain.AddToDrawChain(resultScreen->drawChain, TH_CHAIN_PRIO_DRAW_RESULTSCREEN);

    return true;
}

ResultScreen::ResultScreen()
{
    std::memset(this, 0, sizeof(ResultScreen));
    this->cursor = 1;
}

ChainCallbackResult ResultScreen::OnUpdate(ResultScreen *resultScreen)
{
    i32 difficulty;
    i32 characterShotType;
    AnmVm *vm;
    i32 i;
    switch (resultScreen->resultScreenState)
    {

    case RESULT_SCREEN_STATE_EXIT:
        g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
        return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;

    case RESULT_SCREEN_STATE_INIT:

        if (resultScreen->frameTimer == 0)
        {

            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = 1;
                vm->flags.colorOp = 1;
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vm->color &= COLOR_BLACK;
                }
                else
                {
                    vm->color &= COLOR_WHITE;
                }
            }

            vm = &resultScreen->unk_40[1];
            for (i = 0; i <= 6; i++, vm++)
            {
                if (i == resultScreen->cursor)
                {
                    if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                    {
                        vm->color = COLOR_DARK_GREY;
                    }
                    else
                    {
                        vm->color = COLOR_WHITE;
                    }

                    vm->posOffset = ZunVec3(-4.0f, -4.0f, 0.0f);
                }
                else
                {
                    if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                    {
                        vm->color = COLOR_SET_ALPHA(COLOR_BLACK, 176);
                    }
                    else
                    {
                        vm->color = COLOR_SET_ALPHA(COLOR_WHITE, 176);
                    }
                    vm->posOffset = ZunVec3(0.0f, 0.0f, 0.0f);
                }
            }
        }

        if (resultScreen->frameTimer < 20)
        {
            break;
        }

        resultScreen->resultScreenState++;
        resultScreen->frameTimer = 0;

    case RESULT_SCREEN_STATE_CHOOSING_DIFFICULTY:

        ResultScreen::MoveCursor(resultScreen, 7);

        vm = &resultScreen->unk_40[1];
        for (i = 0; i <= 6; i++, vm++)
        {
            if (i == resultScreen->cursor)
            {
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vm->color = COLOR_DARK_GREY;
                }
                else
                {
                    vm->color = COLOR_WHITE;
                }
                vm->posOffset = ZunVec3(-4.0f, -4.0f, 0.0f);
            }
            else
            {
                if (((g_Supervisor.cfg.opts >> GCOS_USE_D3D_HW_TEXTURE_BLENDING) & 1) == 0)
                {
                    vm->color = COLOR_SET_ALPHA(COLOR_BLACK, 176);
                }
                else
                {
                    vm->color = COLOR_SET_ALPHA(COLOR_WHITE, 176);
                }
                vm->posOffset = ZunVec3(0.0f, 0.0f, 0.0f);
            }
        }

        if (WAS_PRESSED(TH_BUTTON_SELECTMENU))
        {
            vm = &resultScreen->unk_40[0];
            switch (resultScreen->cursor)
            {
            case RESULT_SCREEN_CURSOR_EASY:
            case RESULT_SCREEN_CURSOR_NORMAL:
            case RESULT_SCREEN_CURSOR_HARD:
            case RESULT_SCREEN_CURSOR_LUNATIC:
            case RESULT_SCREEN_CURSOR_EXTRA:
                for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
                {
                    vm->pendingInterrupt = resultScreen->cursor + 3;
                }
                resultScreen->diffSelected = resultScreen->cursor;

                resultScreen->resultScreenState = resultScreen->cursor + RESULT_SCREEN_STATE_BEST_SCORES_EASY;
                resultScreen->lastResultScreenState = resultScreen->resultScreenState;
                resultScreen->frameTimer = 0;
                resultScreen->cursor = resultScreen->lastBestScoresCursor;
                resultScreen->charUsed = -1;
                resultScreen->lastSpellcardSelected = -1;
                break;

            case RESULT_SCREEN_CURSOR_SPELLCARDS:
                for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
                {
                    vm->pendingInterrupt = resultScreen->cursor + 3;
                }
                resultScreen->diffSelected = resultScreen->cursor;
                resultScreen->resultScreenState = RESULT_SCREEN_STATE_SPELLCARDS;
                resultScreen->lastResultScreenState = resultScreen->resultScreenState;
                resultScreen->frameTimer = 0;
                resultScreen->charUsed = -1;
                resultScreen->cursor = resultScreen->previousCursor;
                resultScreen->lastSpellcardSelected = -1;
                break;

            case RESULT_SCREEN_CURSOR_EXIT:
                for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
                {
                    vm->pendingInterrupt = 2;
                }
                resultScreen->resultScreenState = RESULT_SCREEN_STATE_EXITING;
                g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            }
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            resultScreen->cursor = RESULT_SCREEN_CURSOR_EXIT;
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
        }
        break;

    case RESULT_SCREEN_STATE_EXITING:

        if (resultScreen->frameTimer < 60)
        {
            break;
        }
        else
        {
            g_Supervisor.curState = SUPERVISOR_STATE_MAINMENU;
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }

    case RESULT_SCREEN_STATE_BEST_SCORES_EXTRA:

        if (IS_PRESSED(TH_BUTTON_FOCUS) || IS_PRESSED(TH_BUTTON_SKIP))
        {

            if (resultScreen->cheatCodeStep < 5)
            {
                if (WAS_PRESSED(TH_BUTTON_HOME))
                {
                    resultScreen->cheatCodeStep++;
                }
                else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
                {
                    resultScreen->cheatCodeStep = 0;
                }
            }
            else if (resultScreen->cheatCodeStep < 7)
            {
                if (WAS_PRESSED(TH_BUTTON_Q))
                {

                    resultScreen->cheatCodeStep++;
                }
                else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
                {
                    resultScreen->cheatCodeStep = 0;
                }
            }
            else if (resultScreen->cheatCodeStep < 10)
            {
                if (WAS_PRESSED(TH_BUTTON_S))
                {
                    resultScreen->cheatCodeStep++;
                }
                else if (WAS_PRESSED(TH_BUTTON_WRONG_CHEATCODE))
                {
                    resultScreen->cheatCodeStep = 0;
                }
            }
            else
            {
                for (characterShotType = 0; characterShotType < HSCR_NUM_CHARS_SHOTTYPES; characterShotType++)
                {
                    for (difficulty = 0; difficulty < HSCR_NUM_DIFFICULTIES; difficulty++)
                    {
                        g_GameManager.clrd[characterShotType].difficultyClearedWithRetries[difficulty] = 99;
                        g_GameManager.clrd[characterShotType].difficultyClearedWithoutRetries[difficulty] = 99;
                    }
                }
                resultScreen->cheatCodeStep = 0;
                g_SoundPlayer.PlaySoundByIdx(SOUND_1UP);
            }
        }
        else
        {
            resultScreen->cheatCodeStep = 0;
        }
    case RESULT_SCREEN_STATE_BEST_SCORES_EASY:
    case RESULT_SCREEN_STATE_BEST_SCORES_NORMAL:
    case RESULT_SCREEN_STATE_BEST_SCORES_HARD:
    case RESULT_SCREEN_STATE_BEST_SCORES_LUNATIC:

        if (resultScreen->charUsed != resultScreen->cursor && resultScreen->frameTimer == 20)
        {
            resultScreen->charUsed = resultScreen->cursor;
            AnmManager::DrawStringFormat2(g_AnmManager, &resultScreen->unk_28a0[0], COLOR_RGB(COLOR_WHITE),
                                          COLOR_RGB(COLOR_BLACK), g_CharacterList[resultScreen->charUsed * 2]);
            AnmManager::DrawStringFormat2(g_AnmManager, &resultScreen->unk_28a0[1], COLOR_RGB(COLOR_WHITE),
                                          COLOR_RGB(COLOR_BLACK), g_CharacterList[resultScreen->charUsed * 2 + 1]);
        }
        if (resultScreen->frameTimer < 30)
        {
            break;
        }
        if (ResultScreen::MoveCursorHorizontally(resultScreen, 2))
        {
            resultScreen->frameTimer = 0;
            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = resultScreen->diffSelected + 3;
            }
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            resultScreen->resultScreenState = RESULT_SCREEN_STATE_INIT;
            resultScreen->frameTimer = 1;
            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = 1;
            }
            resultScreen->lastBestScoresCursor = resultScreen->cursor;
            resultScreen->cursor = resultScreen->diffSelected;
        }

        break;

    case RESULT_SCREEN_STATE_SPELLCARDS:

        if (resultScreen->lastSpellcardSelected != resultScreen->cursor && resultScreen->frameTimer == 20)
        {

            resultScreen->lastSpellcardSelected = resultScreen->cursor;
            for (i = resultScreen->lastSpellcardSelected * 10; i < resultScreen->lastSpellcardSelected * 10 + 10; i++)
            {
                if (i >= ARRAY_SIZE_SIGNED(g_GameManager.catk))
                {
                    break;
                }
                if (g_GameManager.catk[i].numAttempts == 0)
                {
                    AnmManager::DrawVmTextFmt(g_AnmManager, &resultScreen->unk_28a0[i % 10], COLOR_RGB(COLOR_WHITE),
                                              COLOR_RGB(COLOR_BLACK), TH_UNKNOWN_SPELLCARD);
                }
                else
                {
                    AnmManager::DrawVmTextFmt(g_AnmManager, &resultScreen->unk_28a0[i % 10], COLOR_RGB(COLOR_WHITE),
                                              COLOR_RGB(COLOR_BLACK), g_GameManager.catk[i].name);
                }
            }
        }
        if (resultScreen->frameTimer < 30)
        {
            break;
        }
        if (ResultScreen::MoveCursorHorizontally(resultScreen, 7))
        {
            resultScreen->frameTimer = 0;
            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = resultScreen->diffSelected + 3;
            }
        }
        if (WAS_PRESSED(TH_BUTTON_RETURNMENU))
        {
            g_SoundPlayer.PlaySoundByIdx(SOUND_BACK);
            resultScreen->resultScreenState = RESULT_SCREEN_STATE_INIT;
            resultScreen->frameTimer = 1;
            vm = &resultScreen->unk_40[0];
            for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
            {
                vm->pendingInterrupt = 1;
            }
            resultScreen->previousCursor = resultScreen->cursor;
            resultScreen->cursor = resultScreen->diffSelected;
        }
        break;

    case RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME:
        resultScreen->HandleResultKeyboard();
        break;

    case RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION:
    case RESULT_SCREEN_STATE_CANT_SAVE_REPLAY:
    case RESULT_SCREEN_STATE_CHOOSING_REPLAY_FILE:
    case RESULT_SCREEN_STATE_WRITING_REPLAY_NAME:
    case RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE:
        resultScreen->HandleReplaySaveKeyboard();
        break;

    case RESULT_SCREEN_STATE_STATS_SCREEN:
    case RESULT_SCREEN_STATE_STATS_TO_SAVE_TRANSITION:
        resultScreen->CheckConfirmButton();
        break;
    };

    vm = &resultScreen->unk_40[0];
    for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, vm++)
    {
        g_AnmManager->ExecuteScript(vm);
    }
    resultScreen->frameTimer++;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult th06::ResultScreen::OnDraw(ResultScreen *resultScreen)
{
    // u8 unused[12];
    // u8 unused2;
    // u8 unused3;

    AnmVm *sprite;
    char keyboardCharacter[2];
    ZunVec2 charPos;

    i32 spellcardIdx;
    ZunVec3 spritePos;
    ScoreListNode *ShootScoreListNodeB;
    i32 column;
    i32 row;
    ScoreListNode *ShootScoreListNodeA;

    char name[9];

    ZunVec3 strPos;

    sprite = &resultScreen->unk_40[0];
    g_Supervisor.viewport.X = 0;
    g_Supervisor.viewport.Y = 0;
    g_Supervisor.viewport.Width = 640;
    g_Supervisor.viewport.Height = 480;

    //    g_Supervisor.d3dDevice->SetViewport(&g_Supervisor.viewport);
    g_Supervisor.viewport.Set();
    g_AnmManager->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);

    for (row = 0; row < ARRAY_SIZE_SIGNED(resultScreen->unk_40); row++, sprite++)
    {
        spritePos = sprite->pos;
        sprite->pos += sprite->posOffset;
        g_AnmManager->DrawNoRotation(sprite);
        sprite->pos = spritePos;
    }
    sprite = &resultScreen->unk_40[14];
    if (sprite->pos.x < 640.0f)
    {
        if (resultScreen->lastResultScreenState != 8)
        {
            spritePos = sprite->pos;
            resultScreen->unk_28a0->pos = spritePos;
            g_AnmManager->DrawNoRotation(&resultScreen->unk_28a0[0]);

            spritePos.x += 320.0f;

            resultScreen->unk_28a0[1].pos = spritePos;
            g_AnmManager->DrawNoRotation(&resultScreen->unk_28a0[1]);

            spritePos.x -= -320.0f;
            spritePos.y += 18.0f;
            spritePos.y += 320.0f;

            ShootScoreListNodeA = resultScreen->scores[resultScreen->diffSelected][resultScreen->charUsed * 2].next;
            ShootScoreListNodeB = resultScreen->scores[resultScreen->diffSelected][resultScreen->charUsed * 2 + 1].next;
            for (row = 0; row < 10; row++)
            {
                if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME)
                {
                    if (g_GameManager.shotType == SHOT_TYPE_A)
                    {
                        if (ShootScoreListNodeA->data->base.unk_9 != 0)
                        {
                            g_AsciiManager.color = 0xfff0f0ff;

                            std::strcpy(name, "        ");
                            name[8] = 0;

                            name[resultScreen->cursor >= 8 ? 7 : resultScreen->cursor] = '_';
                            g_AsciiManager.AddFormatText(&spritePos, "   %8s", &name);
                        }
                        else
                        {
                            g_AsciiManager.color = 0x80ffffc0;
                        }
                    }
                    else
                    {
                        g_AsciiManager.color = 0x80ffc0c0;
                    }
                }
                else
                {
                    g_AsciiManager.color = 0xffffc0c0;
                }
                g_AsciiManager.AddFormatText(&spritePos, "%2d", row + 1);

                spritePos.x += 36.0f;
                if (ShootScoreListNodeA->data->stage <= 6)
                {
                    g_AsciiManager.AddFormatText(&spritePos, "%8s %9d(%d)", ShootScoreListNodeA->data->name,
                                                 ShootScoreListNodeA->data->score, ShootScoreListNodeA->data->stage);
                }
                else if (ShootScoreListNodeA->data->stage == 7)
                {
                    g_AsciiManager.AddFormatText(&spritePos, "%8s %9d(1)", ShootScoreListNodeA->data->name,
                                                 ShootScoreListNodeA->data->score);
                }
                else
                {
                    g_AsciiManager.AddFormatText(&spritePos, "%8s %9d(C)", ShootScoreListNodeA->data->name,
                                                 ShootScoreListNodeA->data->score);
                }
                spritePos.x += 300.0f;
                if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME)
                {
                    if (g_GameManager.shotType == SHOT_TYPE_B)
                    {
                        if (ShootScoreListNodeB->data->base.unk_9 != 0)
                        {
                            g_AsciiManager.color = 0xfffff0f0;

                            std::strcpy(name, "        ");
                            name[8] = 0;

                            name[resultScreen->cursor >= 8 ? 7 : resultScreen->cursor] = '_';
                            g_AsciiManager.AddFormatText(&spritePos, "%8s", &name);
                        }
                        else
                        {
                            g_AsciiManager.color = 0xc0c0c0ff;
                        }
                    }
                    else
                    {
                        g_AsciiManager.color = 0x80c0c0ff;
                    }
                }
                else
                {
                    g_AsciiManager.color = 0xffc0c0ff;
                }
                if (ShootScoreListNodeB->data->stage <= 6)
                {
                    g_AsciiManager.AddFormatText(&spritePos, "%8s %9d(%d)", ShootScoreListNodeB->data->name,
                                                 ShootScoreListNodeB->data->score, ShootScoreListNodeB->data->stage);
                }
                else if (ShootScoreListNodeB->data->stage == 7)
                {
                    g_AsciiManager.AddFormatText(&spritePos, "%8s %9d(1)", ShootScoreListNodeB->data->name,
                                                 ShootScoreListNodeB->data->score);
                }
                else
                {
                    g_AsciiManager.AddFormatText(&spritePos, "%8s %9d(C)", ShootScoreListNodeB->data->name,
                                                 ShootScoreListNodeB->data->score);
                }
                spritePos.x -= 336.0f;
                spritePos.y += 336.0f;
                ShootScoreListNodeA = ShootScoreListNodeA->next;
                ShootScoreListNodeB = ShootScoreListNodeB->next;
            }
        }
        else
        {

            spritePos = sprite->pos;
            spritePos.y += 16.0f;

            for (row = 0; row < 10; row++)
            {
                spellcardIdx = resultScreen->lastSpellcardSelected * 10 + row;
                if (spellcardIdx >= ARRAY_SIZE_SIGNED(g_GameManager.catk))
                {
                    break;
                }

                resultScreen->unk_28a0[row].pos = spritePos;
                if (g_GameManager.catk[spellcardIdx].numAttempts == 0)
                {
                    g_AsciiManager.color = 0x80c0c0ff;
                }
                else if (g_GameManager.catk[spellcardIdx].numSuccess == 0)
                {
                    g_AsciiManager.color = 0xffc0a0a0;
                }
                else
                {
                    g_AsciiManager.color = 0xfff0f0ff - row * 0x80800;
                }
                g_AsciiManager.AddFormatText(&spritePos, "No.%.2d", spellcardIdx + 1);

                resultScreen->unk_28a0[row].pos.x += 96.0f;

                g_AnmManager->DrawNoRotation(&resultScreen->unk_28a0[row]);

                spritePos.x += 368.0f;

                g_AsciiManager.AddFormatText(&spritePos, "%3d/%3d", g_GameManager.catk[spellcardIdx].numSuccess,
                                             g_GameManager.catk[spellcardIdx].numAttempts);
                spritePos.x -= 368.0f;
                spritePos.y += 30.0f;
            }
        }
    }
    if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME ||
        resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_REPLAY_NAME)
    {
        spritePos = ZunVec3(160.0f, 356.0f, 0.0f);

        for (row = 0; row < RESULT_KEYBOARD_ROWS; row++)
        {
            for (column = 0; column < RESULT_KEYBOARD_COLUMNS; column++)
            {
                charPos.y = 0.0f;
                charPos.x = 0.0f;
                if (resultScreen->selectedCharacter == row * RESULT_KEYBOARD_COLUMNS + column)
                {
                    g_AsciiManager.color = COLOR_KEYBOARD_KEY_HIGHLIGHT;
                    if (resultScreen->frameTimer % 64 < 32)
                    {
                        charPos.y = 1.2f + 0.8f * (resultScreen->frameTimer % 0x20) / 32.0f;
                    }
                    else
                    {
                        charPos.y = 2.0f - 0.8f * (resultScreen->frameTimer % 0x20) / 32.0f;
                    }
                    g_AsciiManager.scale.x = charPos.y;
                    g_AsciiManager.scale.y = charPos.y;
                    charPos.y = -(charPos.y - 1.0f) * 8.0f;
                    charPos.x = charPos.y;
                }
                else
                {
                    g_AsciiManager.color = COLOR_KEYBOARD_KEY_NORMAL;
                    g_AsciiManager.scale.x = 1.0f;
                    g_AsciiManager.scale.y = 1.0f;
                }
                strPos = spritePos;
                strPos.x += charPos.y;
                strPos.y += charPos.x;
                keyboardCharacter[0] = g_AlphabetList[row * RESULT_KEYBOARD_COLUMNS + column];
                keyboardCharacter[1] = '\0';

                if (row == 5)
                {
                    if (column == 14)
                    {
                        keyboardCharacter[0] = 0x80; // SP
                    }
                    else if (column == 15)
                    {
                        keyboardCharacter[0] = 0x81; // END
                    }
                }

                g_AsciiManager.AddString(&strPos, keyboardCharacter);

                spritePos.x += 20.0f;
            }
            spritePos.x -= column * 20;
            spritePos.y += 18.0f;
        }
    }
    g_AsciiManager.scale.x = 1.0;
    g_AsciiManager.scale.y = 1.0;
    if ((resultScreen->resultScreenState >= RESULT_SCREEN_STATE_SAVE_REPLAY_QUESTION) &&
        (resultScreen->resultScreenState <= RESULT_SCREEN_STATE_OVERWRITE_REPLAY_FILE))
    {
        sprite = &resultScreen->unk_40[15];
        for (row = 0; row < 6; row++, sprite++)
        {
            g_AnmManager->DrawNoRotation(sprite);
        }
        sprite = &resultScreen->unk_40[21];
        spritePos = sprite->pos;
        sprite++;
        g_AsciiManager.AddFormatText(&spritePos, "No.   Name     Date     Player Score");
        for (row = 0; row < ARRAY_SIZE_SIGNED(resultScreen->replays); row++)
        {
            spritePos = sprite->pos;
            sprite++;
            if (row == resultScreen->replayNumber)
            {
                g_AsciiManager.color = COLOR_LIGHT_RED;
            }
            else
            {
                g_AsciiManager.color = COLOR_GREY;
            }
            if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_WRITING_REPLAY_NAME)
            {
                g_AsciiManager.AddFormatText(&spritePos, "No.%.2d %8s %8s %7s %9d", row + 1, &resultScreen->replayName,
                                             resultScreen->defaultReplay.date,
                                             g_ShortCharacterList2[g_GameManager.CharacterShotType()],
                                             resultScreen->defaultReplay.score);
                g_AsciiManager.color = 0xfff0f0ff;

                std::strcpy(name, "        ");

                name[8] = 0;

                name[resultScreen->cursor >= 8 ? 7 : resultScreen->cursor] = '_';
                g_AsciiManager.AddFormatText(&spritePos, "      %8s", &name);
            }
            else if (*(i32 *)&resultScreen->replays[row].magic != *(i32 *)"T6RP" ||
                     resultScreen->replays[row].version != GAME_VERSION)
            {
                g_AsciiManager.AddFormatText(&spritePos, "No.%.2d -------- --/--/-- -------         0", row + 1);
            }
            else
            {
                g_AsciiManager.AddFormatText(&spritePos, "No.%.2d %8s %8s %7s %9d", row + 1,
                                             resultScreen->replays[row].name, resultScreen->replays[row].date,
                                             g_ShortCharacterList2[resultScreen->replays[row].shottypeChara],
                                             resultScreen->replays[row].score);
            }
        }
    }
    g_AsciiManager.color = COLOR_WHITE;
    resultScreen->DrawFinalStats();

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

bool ResultScreen::AddedCallback(ResultScreen *resultScreen)
{

    i32 slot;
    i32 characterShot;
    AnmVm *sprite;
    i32 i;

    if (resultScreen->resultScreenState != RESULT_SCREEN_STATE_EXIT)
    {

        if (!g_AnmManager->LoadSurface(0, "data/result/result.jpg"))
        {
            return false;
        }

        if (!g_AnmManager->LoadAnm(ANM_FILE_RESULT00, "data/result00.anm", ANM_OFFSET_RESULT00))
        {
            return false;
        }

        if (!g_AnmManager->LoadAnm(ANM_FILE_RESULT01, "data/result01.anm", ANM_OFFSET_RESULT01))
        {
            return false;
        }

        if (!g_AnmManager->LoadAnm(ANM_FILE_RESULT02, "data/result02.anm", ANM_OFFSET_RESULT02))
        {
            return false;
        }

        if (!g_AnmManager->LoadAnm(ANM_FILE_RESULT03, "data/result03.anm", ANM_OFFSET_RESULT03))
        {
            return false;
        }

        sprite = &resultScreen->unk_40[0];
        for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_40); i++, sprite++)
        {

            sprite->pos = ZunVec3(0.0f, 0.0f, 0.0f);
            sprite->posOffset = ZunVec3(0.0f, 0.0f, 0.0f);

            // Execute all the scripts from the start of result00 to the end of result02
            g_AnmManager->SetAndExecuteScriptIdx(sprite, ANM_SCRIPT_RESULT00_START + i);
        }

        sprite = &resultScreen->unk_28a0[0];
        for (i = 0; i < ARRAY_SIZE_SIGNED(resultScreen->unk_28a0); i++, sprite++)
        {
            g_AnmManager->InitializeAndSetSprite(sprite, ANM_SCRIPT_TEXT_RESULTSCREEN_CHARACTER_NAME + i);

            sprite->pos = ZunVec3(0.0f, 0.0f, 0.0f);

            sprite->flags.anchor = AnmVmAnchor_TopLeft;

            sprite->fontWidth = 15;
            sprite->fontHeight = 15;
        }
    }

    for (i = 0; i < HSCR_NUM_DIFFICULTIES; i++)
    {
        for (characterShot = 0; characterShot < HSCR_NUM_CHARS_SHOTTYPES; characterShot++)
        {
            for (slot = 0; slot < HSCR_NUM_SCORES_SLOTS; slot++)
            {
                resultScreen->defaultScore[i][characterShot][slot].score = 1000000 - slot * 100000;
                resultScreen->defaultScore[i][characterShot][slot].base.magic = g_DefaultMagic;
                resultScreen->defaultScore[i][characterShot][slot].difficulty = i;
                resultScreen->defaultScore[i][characterShot][slot].base.version = TH6K_VERSION;
                resultScreen->defaultScore[i][characterShot][slot].base.unkLen = sizeof(Hscr);
                resultScreen->defaultScore[i][characterShot][slot].base.th6kLen = sizeof(Hscr);
                resultScreen->defaultScore[i][characterShot][slot].stage = 1;
                resultScreen->defaultScore[i][characterShot][slot].base.unk_9 = 0;

                resultScreen->LinkScoreEx(resultScreen->defaultScore[i][characterShot] + slot, i, characterShot);

                std::strcpy(resultScreen->defaultScore[i][characterShot][slot].name, DEFAULT_HIGH_SCORE_NAME);
            }
        }
    }

    resultScreen->lastBestScoresCursor = 0;
    resultScreen->scoreDat = ResultScreen::OpenScore("score.dat");

    for (i = 0; i < HSCR_NUM_DIFFICULTIES; i++)
    {
        for (characterShot = 0; characterShot < HSCR_NUM_CHARS_SHOTTYPES; characterShot++)
        {
            ResultScreen::GetHighScore(resultScreen->scoreDat, &resultScreen->scores[i][characterShot], characterShot,
                                       i);
        }
    }

    if (resultScreen->resultScreenState != RESULT_SCREEN_STATE_WRITING_HIGHSCORE_NAME &&
        resultScreen->resultScreenState != RESULT_SCREEN_STATE_EXIT)
    {
        ParseCatk(resultScreen->scoreDat, g_GameManager.catk);
        ParseClrd(resultScreen->scoreDat, g_GameManager.clrd);
        ParsePscr(resultScreen->scoreDat, (Pscr *)g_GameManager.pscr);
    }

    if (resultScreen->resultScreenState == RESULT_SCREEN_STATE_EXIT &&
        (u32)g_GameManager.pscr[g_GameManager.CharacterShotType()][g_GameManager.currentStage - 1][g_GameManager.difficulty]
                .score < g_GameManager.score)
    {
        g_GameManager.pscr[g_GameManager.CharacterShotType()][g_GameManager.currentStage - 1][g_GameManager.difficulty]
            .score = g_GameManager.score;
    }

    resultScreen->unk_39a0.activeSpriteIndex = -1;

    return true;
}

bool ResultScreen::DeletedCallback(ResultScreen *resultScreen)
{
    i32 character;
    i32 difficulty;

    if (resultScreen->scoreDat != NULL)
    {
        ResultScreen::WriteScore(resultScreen);
        ResultScreen::ReleaseScoreDat(resultScreen->scoreDat);
    }

    resultScreen->scoreDat = NULL;
    for (difficulty = 0; difficulty < HSCR_NUM_DIFFICULTIES; difficulty++)
    {
        for (character = 0; character < HSCR_NUM_CHARS_SHOTTYPES; character++)
        {
            resultScreen->FreeScore(difficulty, character);
        }
    }
    g_AnmManager->ReleaseAnm(ANM_FILE_RESULT00);
    g_AnmManager->ReleaseAnm(ANM_FILE_RESULT01);
    g_AnmManager->ReleaseAnm(ANM_FILE_RESULT02);
    g_AnmManager->ReleaseAnm(ANM_FILE_RESULT03);
    g_AnmManager->ReleaseSurface(0);

    g_Chain.Cut(resultScreen->drawChain);

    resultScreen->drawChain = NULL;

    delete resultScreen;
    resultScreen = NULL;

    return true;
}

}; // namespace th06

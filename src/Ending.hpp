#pragma once
#include "AnmVm.hpp"
#include "Chain.hpp"
#include "ZunMath.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

namespace th06
{
ZunResult Ending_RegisterChain();
enum EndingFadeType
{
    ENDING_FADE_TYPE_NO_FADE,
    ENDING_FADE_TYPE_FADE_IN_BLACK,
    ENDING_FADE_TYPE_FADE_OUT_BLACK,
    ENDING_FADE_TYPE_FADE_IN_WHITE,
    ENDING_FADE_TYPE_FADE_OUT_WHITE,
};

#define END_READ_OPCODE '@'
enum EndOpcode
{
    END_OPCODE_FADE_IN_BLACK = '0',
    END_OPCODE_FADE_OUT_BLACK = '1',
    END_OPCODE_FADE_IN_WHITE = '2',
    END_OPCODE_FADE_OUT_WHITE = '3',
    END_OPCODE_ANM_SET_SLOT = 'a',
    END_OPCODE_BACKGROUND = 'b',
    END_OPCODE_TEXT_COLOR = 'c',
    END_OPCODE_MUSIC = 'm',
    END_OPCODE_WAIT_CLEAR = 'r',
    END_OPCODE_SET_DELAY = 's',
    END_OPCODE_BACKGROUND_SCROLL_SET = 'v',
    END_OPCODE_WAIT = 'w',
    END_OPCODE_END_DELETE = 'z',
    END_OPCODE_END_SWITCH = 'F',
    END_OPCODE_MUSIC_FADE_OUT = 'M',
    END_OPCODE_CLEAR = 'R',
    END_OPCODE_BACKGROUND_SCROLL = 'V',
};

#define SPRITES_PER_LINE_GROUP 2
// NOTE: Currently unclear if this is intended to be 8 line groups
// or fewer line groups with some extra space for more sprites.
#define END_SPRITE_COUNT 16

struct Ending
{
    Ending()
    {
        memset(this, 0, sizeof(Ending));
        this->longDelay = 8;
        this->pauseTimer = 0;
        this->scriptTime = 0;
        this->backgroundPos.x = 0.0f;
        this->backgroundPos.y = 0.0f;
        this->backgroundScrollSpeed = 0.0f;
    }

    i32 ReadEndFileParameter();

    ZunResult ParseEndFile();

    ZunResult LoadEnding(const char *endFilePath);
    void FadingEffect();

    ChainElem *calcChain;
    ChainElem *drawChain;
    ZunVec2 backgroundPos;
    f32 backgroundScrollSpeed;
    AnmVm sprites[END_SPRITE_COUNT];
    u8 *endFileData;
    ZunBool hasSeenEnding;
    ZunTimer scriptTime;
    ZunTimer pauseTimer;
    ZunTimer clearTimer;
    i32 minWaitClearFrames;
    i32 minWaitPauseFrames;
    i32 longDelay;
    i32 shortDelay;
    unreferenced_fields(0x4);
    i32 textLine;
    ZunColor textColor;
    ZunColor fadeColor;
    i32 fadeTimer;
    i32 fadeDuration;
    EndingFadeType fadeType;
    const char *endFileDataPtr;
};
ZUN_ASSERT_TYPE(Ending, 0x1170, 4);

} // namespace th06

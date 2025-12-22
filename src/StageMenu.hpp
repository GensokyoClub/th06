#pragma once

#include "AnmVm.hpp"
#include "inttypes.hpp"

struct StageMenu
{
    StageMenu();

    i32 OnUpdateGameMenu();
    i32 OnUpdateRetryMenu();

    void OnDrawGameMenu();
    void OnDrawRetryMenu();

    // Current state of this menu.
    u32 curState;
    // Number of frames since last state change. Used to delay certain actions
    // until an animation is finished.
    i32 numFrames;
    AnmVm menuSprites[6];
    AnmVm menuBackground;
};

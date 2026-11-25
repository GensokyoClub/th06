#include "stdafx.h"

#define ANGLE_UP RADIANS(-90.0f)

/*
    BULLETS:
    firerate is applied as a modulo to the shot timer, which
    always counts from 0 to 29. Bullets are shot whenever
    this value equals delay.
    {
        fireRate, delay,
        spawnOffset, size,
        angle, speed,
        damage,
        spawnPosition, bulletType,
        anmScript, shootSfx
    }

    LASERS:
    {
        delayTimerValue, delayTimerIdx,
        spawnOffset, size,
        angle, speed,
        damage,
        spawnPosition, bulletType,
        anmScript, shootSfx
    }
*/

// clang-format off
namespace th06
{
ShtEntry g_ReimuALevel0[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        48,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
};

ShtEntry g_ReimuALevel1[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        48,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(30.0f), 10.0f,
        14,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(30.0f), 10.0f,
        14,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
};

// BUG: No bullets have a shooting sound effect
ShtEntry g_ReimuALevel2[] = {
    {
        5, 0,
        { -4.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(1.0f), 12.0f,
        30,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 4.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(1.0f), 12.0f,
        30,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(30.0f), 10.0f,
        14,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(30.0f), 10.0f,
        14,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
};

ShtEntry g_ReimuALevel3[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(6.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        30,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(6.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(30.0f), 10.0f,
        14,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(30.0f), 10.0f,
        14,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
};

ShtEntry g_ReimuALevel4[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(7.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        30,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(7.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(30.0f), 10.0f,
        12,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(30.0f), 10.0f,
        12,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
};

ShtEntry g_ReimuALevel5[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(7.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        29,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(7.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(30.0f), 10.0f,
        9,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(30.0f), 10.0f,
        9,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(60.0f), 10.0f,
        12,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(60.0f), 10.0f,
        12,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
};

ShtEntry g_ReimuALevel6[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(7.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        28,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(7.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(20.0f), 10.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(20.0f), 10.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 10,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(40.0f), 10.0f,
        9,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 10,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(40.0f), 10.0f,
        9,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 20,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(60.0f), 10.0f,
        11,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        30, 20,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(60.0f), 10.0f,
        11,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
};

ShtEntry g_ReimuALevel7[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(7.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        28,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(7.0f), 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(20.0f), 10.0f,
        8,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(20.0f), 10.0f,
        8,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        15, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(40.0f), 10.0f,
        8,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        15, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(40.0f), 10.0f,
        8,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        15, 10,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(60.0f), 10.0f,
        8,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        15, 10,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(60.0f), 10.0f,
        8,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
};

ShtEntry g_ReimuALevel8[] = {
    {
        5, 0,
        { -8.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(7.0f), 12.0f,
        23,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { -8.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 8.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        24,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 8.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(7.0f), 12.0f,
        23,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        16, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(20.0f), 10.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        16, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(20.0f), 10.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        16, 4,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(40.0f), 10.0f,
        8,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        16, 4,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(40.0f), 10.0f,
        8,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        16, 8,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(60.0f), 10.0f,
        7,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        16, 8,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(60.0f), 10.0f,
        7,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        16, 12,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(80.0f), 10.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
    {
        16, 12,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(80.0f), 10.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_HOMING,
        ANM_SCRIPT_REIMU_HOMING_AMULET, NO_SOUND
    },
};

PowerData g_PowerDataReimuA[] = {
    {ARRAY_SIZE(g_ReimuALevel0), LEVEL_1, g_ReimuALevel0},
    {ARRAY_SIZE(g_ReimuALevel1), LEVEL_2, g_ReimuALevel1},
    {ARRAY_SIZE(g_ReimuALevel2), LEVEL_3, g_ReimuALevel2},
    {ARRAY_SIZE(g_ReimuALevel3), LEVEL_4, g_ReimuALevel3},
    {ARRAY_SIZE(g_ReimuALevel4), LEVEL_5, g_ReimuALevel4},
    {ARRAY_SIZE(g_ReimuALevel5), LEVEL_6, g_ReimuALevel5},
    {ARRAY_SIZE(g_ReimuALevel6), LEVEL_7, g_ReimuALevel6},
    {ARRAY_SIZE(g_ReimuALevel7), LEVEL_8, g_ReimuALevel7},
    {ARRAY_SIZE(g_ReimuALevel8), LEVEL_UNREACHABLE, g_ReimuALevel8},
};

ShtEntry g_ReimuBLevel0[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        48,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
};

ShtEntry g_ReimuBLevel1[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        48,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        15, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
};

ShtEntry g_ReimuBLevel2[] = {
    {
        5, 0,
        { -4.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(1.0f), 12.0f,
        32,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 4.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(1.0f), 12.0f,
        32,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
};

ShtEntry g_ReimuBLevel3[] = {
    {
        5, 0,
        { -4.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(1.0f), 12.0f,
        30,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 4.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(1.0f), 12.0f,
        30,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        8, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        8, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
};

ShtEntry g_ReimuBLevel4[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(7.0f), 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        28,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(7.0f), 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        8, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        8, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
};

ShtEntry g_ReimuBLevel5[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(7.0f), 12.0f,
        16,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        27,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(7.0f), 12.0f,
        16,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 8.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        5, 0,
        { 8.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        12,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        8, 0,
        { -8.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        8, 0,
        { -8.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
};

// BUG: Shooting sound effect plays twice
ShtEntry g_ReimuBLevel6[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 12.0f,
        16,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT, ANM_SCRIPT_REIMU_TALISMAN,
        SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        22,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT, ANM_SCRIPT_REIMU_TALISMAN,
        NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 12.0f,
        16,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        3, 0,
        { 8.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, SOUND_SHOOT
    },
    {
        3, 0,
        { 8.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        5, 0,
        { -8.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        5, 0,
        { -8.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
};

// BUG: Shooting sound effect plays twice
ShtEntry g_ReimuBLevel7[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(16.0f), 12.0f,
        9,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 12.0f,
        17,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 12.0f,
        17,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(16.0f), 12.0f,
        9,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        3, 0,
        { 12.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, SOUND_SHOOT
    },
    {
        3, 0,
        { 12.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        5, 0,
        { -12.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        5, 0,
        { -12.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
};

// BUG: Shooting sound effect plays twice
ShtEntry g_ReimuBLevel8[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(16.0f), 12.0f,
        9,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 12.0f,
        17,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 12.0f,
        17,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(16.0f), 12.0f,
        9,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_TALISMAN, NO_SOUND
    },
    {
        3, 0,
        { 12.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, SOUND_SHOOT
    },
    {
        3, 0,
        { 12.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        3, 0,
        { -12.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        3, 0,
        { -12.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, -16.0f }, { 12.0f, 40.0f },
        ANGLE_UP, 22.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_REIMU_NEEDLE, NO_SOUND
    },
};

PowerData g_PowerDataReimuB[] = {
    {ARRAY_SIZE(g_ReimuBLevel0), LEVEL_1, g_ReimuBLevel0},
    {ARRAY_SIZE(g_ReimuBLevel1), LEVEL_2, g_ReimuBLevel1},
    {ARRAY_SIZE(g_ReimuBLevel2), LEVEL_3, g_ReimuBLevel2},
    {ARRAY_SIZE(g_ReimuBLevel3), LEVEL_4, g_ReimuBLevel3},
    {ARRAY_SIZE(g_ReimuBLevel4), LEVEL_5, g_ReimuBLevel4},
    {ARRAY_SIZE(g_ReimuBLevel5), LEVEL_6, g_ReimuBLevel5},
    {ARRAY_SIZE(g_ReimuBLevel6), LEVEL_7, g_ReimuBLevel6},
    {ARRAY_SIZE(g_ReimuBLevel7), LEVEL_8, g_ReimuBLevel7},
    {ARRAY_SIZE(g_ReimuBLevel8), LEVEL_UNREACHABLE, g_ReimuBLevel8},
};

ShtEntry g_MarisaALevel0[] = {
    {
        5, 0,
        { 0.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        48,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
};

ShtEntry g_MarisaALevel1[] = {
    {
        5, 0,
        { 0.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        36,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 3.0f,
        18,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 3.0f,
        18,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
};

ShtEntry g_MarisaALevel2[] = {
    {
        5, 0,
        { 0.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        32,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 3.0f,
        16,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
    {
        30, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 3.0f,
        16,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
    {
        30, 15,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
    {
        30, 15,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
};

// BUG: Bullets 4 and 5 will never fire because X % 15 != 15
ShtEntry g_MarisaALevel3[] = {
    {
        5, 0,
        { 0.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        32,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 3.0f,
        15,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 3.0f,
        15,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
    {
        15, 15,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
    {
        15, 15,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_TINY, NO_SOUND
    },
};

// BUG: Bullets 4 and 5 will never fire because X % 15 != 20
ShtEntry g_MarisaALevel4[] = {
    {
        5, 0,
        { 0.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        32,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 3.0f,
        16,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_SMALL, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 3.0f,
        16,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_SMALL, NO_SOUND
    },
    {
        15, 20,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 3.0f,
        11,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_SMALL, NO_SOUND
    },
    {
        15, 20,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 3.0f,
        11,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_SMALL, NO_SOUND
    },
};

ShtEntry g_MarisaALevel5[] = {
    {
        5, 0,
        { -8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        16,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        16,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 3.0f,
        16,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_SMALL, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 3.0f,
        16,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_SMALL, NO_SOUND
    },
    {
        15, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_SMALL, NO_SOUND
    },
    {
        15, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_SMALL, NO_SOUND
    },
};

ShtEntry g_MarisaALevel6[] = {
    {
        5, 0,
        { -8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        13,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        13,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 3.0f,
        16,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 3.0f,
        16,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
    {
        10, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
    {
        10, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
};

ShtEntry g_MarisaALevel7[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(4.0f), 12.0f,
        8,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        12,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(4.0f), 12.0f,
        8,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 3.0f,
        15,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 3.0f,
        15,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
    {
        10, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
    {
        10, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(12.0f), 3.0f,
        9,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
    {
        15, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(12.0f), 3.0f,
        9,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_MEDIUM, NO_SOUND
    },
};

ShtEntry g_MarisaALevel8[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(4.0f), 12.0f,
        8,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        12,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(4.0f), 12.0f,
        8,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 3.0f,
        14,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_LARGE, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 3.0f,
        14,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_LARGE, NO_SOUND
    },
    {
        10, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(8.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_LARGE, NO_SOUND
    },
    {
        10, 5,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(8.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_LARGE, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(15.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(0), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_LARGE, NO_SOUND
    },
    {
        10, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(15.0f), 3.0f,
        10,
        SPAWN_FROM_OPTION(1), PLAYER_BULLET_ACCELERATE,
        ANM_SCRIPT_MARISA_MISSILE_LARGE, NO_SOUND
    },
};

PowerData g_PowerDataMarisaA[] = {
    {ARRAY_SIZE(g_MarisaALevel0), LEVEL_1, g_MarisaALevel0},
    {ARRAY_SIZE(g_MarisaALevel1), LEVEL_2, g_MarisaALevel1},
    {ARRAY_SIZE(g_MarisaALevel2), LEVEL_3, g_MarisaALevel2},
    {ARRAY_SIZE(g_MarisaALevel3), LEVEL_4, g_MarisaALevel3},
    {ARRAY_SIZE(g_MarisaALevel4), LEVEL_5, g_MarisaALevel4},
    {ARRAY_SIZE(g_MarisaALevel5), LEVEL_6, g_MarisaALevel5},
    {ARRAY_SIZE(g_MarisaALevel6), LEVEL_7, g_MarisaALevel6},
    {ARRAY_SIZE(g_MarisaALevel7), LEVEL_8, g_MarisaALevel7},
    {ARRAY_SIZE(g_MarisaALevel8), LEVEL_UNREACHABLE, g_MarisaALevel8},
};

ShtEntry g_MarisaBLevel0[] = {
    {
        5, 0,
        { 0.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        48,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
};

ShtEntry g_MarisaBLevel1[] = {
    {
        5, 0,
        { 0.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        32,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        120, 0,
        { 0.0f, 0.0f }, { 10.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        3,
        SPAWN_FROM_OPTION(0), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_SMALL, NO_SOUND
    },
    {
        120, 1,
        { 0.0f, 0.0f }, { 10.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        3,
        SPAWN_FROM_OPTION(1), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_SMALL, NO_SOUND
    },
};

ShtEntry g_MarisaBLevel2[] = {
    {
        5, 0,
        { 0.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP, 12.0f,
        32,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        170, 0,
        { 0.0f, 0.0f }, { 10.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        3,
        SPAWN_FROM_OPTION(0), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_SMALL, NO_SOUND
    },
    {
        170, 1,
        { 0.0f, 0.0f }, { 10.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        3,
        SPAWN_FROM_OPTION(1), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_SMALL, NO_SOUND
    },
};

ShtEntry g_MarisaBLevel3[] = {
    {
        5, 0,
        { -8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP - RADIANS(2.0f), 12.0f,
        22,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP + RADIANS(2.0f), 12.0f,
        22,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        200, 0,
        { 0.0f, 0.0f }, { 10.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        3,
        SPAWN_FROM_OPTION(0), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_SMALL, NO_SOUND
    },
    {
        200, 1,
        { 0.0f, 0.0f }, { 10.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        3,
        SPAWN_FROM_OPTION(1), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_SMALL, NO_SOUND
    },
};

ShtEntry g_MarisaBLevel4[] = {
    {
        5, 0,
        { -8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP - RADIANS(2.0f), 12.0f,
        22,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP + RADIANS(2.0f), 12.0f,
        22,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        210, 0,
        { 0.0f, 0.0f }, { 10.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        3,
        SPAWN_FROM_OPTION(0), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_MEDIUM, NO_SOUND
    },
    {
        210, 1,
        { 0.0f, 0.0f }, { 10.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        3,
        SPAWN_FROM_OPTION(1), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_MEDIUM, NO_SOUND
    },
};

ShtEntry g_MarisaBLevel5[] = {
    {
        5, 0,
        { -8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP - RADIANS(2.0f), 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 8.0f, -8.0f }, { 12.0f, 24.0f },
        ANGLE_UP + RADIANS(2.0f), 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        230, 0,
        { 0.0f, 0.0f }, { 15.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        4,
        SPAWN_FROM_OPTION(0), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_MEDIUM, NO_SOUND
    },
    {
        230, 1,
        { 0.0f, 0.0f }, { 15.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        4,
        SPAWN_FROM_OPTION(1), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_MEDIUM, NO_SOUND
    },
};

ShtEntry g_MarisaBLevel6[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 12.0f,
        15,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 12.0f,
        15,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        250, 0,
        { 0.0f, 0.0f }, { 15.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        4,
        SPAWN_FROM_OPTION(0), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_MEDIUM, NO_SOUND
    },
    {
        250, 1,
        { 0.0f, 0.0f }, { 15.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        4,
        SPAWN_FROM_OPTION(1), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_MEDIUM, NO_SOUND
    },
};

ShtEntry g_MarisaBLevel7[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 12.0f,
        15,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 12.0f,
        15,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        270, 0,
        { 0.0f, 0.0f }, { 20.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        5,
        SPAWN_FROM_OPTION(0), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_LARGE, NO_SOUND
    },
    {
        270, 1,
        { 0.0f, 0.0f }, { 20.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        5,
        SPAWN_FROM_OPTION(1), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_LARGE, NO_SOUND
    },
};

ShtEntry g_MarisaBLevel8[] = {
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(10.0f), 12.0f,
        12,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, SOUND_SHOOT
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP - RADIANS(5.0f), 12.0f,
        15,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP, 12.0f,
        20,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(5.0f), 12.0f,
        15,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        5, 0,
        { 0.0f, 0.0f }, { 12.0f, 12.0f },
        ANGLE_UP + RADIANS(10.0f), 12.0f,
        12,
        SPAWN_FROM_PLAYER, PLAYER_BULLET_DEFAULT,
        ANM_SCRIPT_MARISA_MAGIC, NO_SOUND
    },
    {
        330, 0,
        { 0.0f, 0.0f }, { 20.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        6,
        SPAWN_FROM_OPTION(0), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_LARGE, NO_SOUND
    },
    {
        330, 1,
        { 0.0f, 0.0f }, { 20.0f, GAME_WINDOW_HEIGHT },
        ANGLE_UP, 3.0f,
        6,
        SPAWN_FROM_OPTION(1), PLAYER_LASER,
        ANM_SCRIPT_MARISA_LASER_LARGE, NO_SOUND
    },
};

PowerData g_PowerDataMarisaB[] = {
    {ARRAY_SIZE(g_MarisaBLevel0), LEVEL_1, g_MarisaBLevel0},
    {ARRAY_SIZE(g_MarisaBLevel1), LEVEL_2, g_MarisaBLevel1},
    {ARRAY_SIZE(g_MarisaBLevel2), LEVEL_3, g_MarisaBLevel2},
    {ARRAY_SIZE(g_MarisaBLevel3), LEVEL_4, g_MarisaBLevel3},
    {ARRAY_SIZE(g_MarisaBLevel4), LEVEL_5, g_MarisaBLevel4},
    {ARRAY_SIZE(g_MarisaBLevel5), LEVEL_6, g_MarisaBLevel5},
    {ARRAY_SIZE(g_MarisaBLevel6), LEVEL_7, g_MarisaBLevel6},
    {ARRAY_SIZE(g_MarisaBLevel7), LEVEL_8, g_MarisaBLevel7},
    {ARRAY_SIZE(g_MarisaBLevel8), LEVEL_UNREACHABLE, g_MarisaBLevel8},
};
// clang-format on
} // namespace th06

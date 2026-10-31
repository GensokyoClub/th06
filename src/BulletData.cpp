#include "stdafx.h"

#include "SoundPlayer.hpp"

namespace th06
{
/* ----------ReimuA---------- */

/* ReimuA Rank 1 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank1[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 48, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
};

/* ReimuA Rank 2 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank2[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 48, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-120.0), 10.0f, 14, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-60.0), 10.0f, 14, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
};

/* ReimuA Rank 3 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank3[] = {
    {5, 0, -4.0f, 0.0f, 12.0f, 12.0f, RADIANS(-91.0), 12.0f, 30, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 4.0f, 0.0f, 12.0f, 12.0f, RADIANS(-89.0), 12.0f, 30, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-120.0), 10.0f, 14, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-60.0), 10.0f, 14, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
};

/* ReimuA Rank 4 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank4[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-96.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 30, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-84.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-120.0), 10.0f, 14, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-60.0), 10.0f, 14, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
};

/* ReimuA Rank 5 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank5[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-97.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 30, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-83.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-120.0), 10.0f, 12, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-60.0), 10.0f, 12, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
};

/* ReimuA Rank 6 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank6[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-97.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 29, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-83.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-120.0), 10.0f, 9, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-60.0), 10.0f, 9, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-150.0), 10.0f, 12, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-30.0), 10.0f, 12, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
};

/* ReimuA Rank 7 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank7[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-97.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 28, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-83.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-110.0), 10.0f, 10, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-70.0), 10.0f, 10, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 10, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-130.0), 10.0f, 9, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 10, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-50.0), 10.0f, 9, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 20, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-150.0), 10.0f, 11, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {30, 20, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-30.0), 10.0f, 11, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
};

/* ReimuA Rank 8 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank8[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-97.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 28, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-83.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-110.0), 10.0f, 8, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-70.0), 10.0f, 8, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {15, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-130.0), 10.0f, 8, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {15, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-50.0), 10.0f, 8, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {15, 10, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-150.0), 10.0f, 8, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {15, 10, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-30.0), 10.0f, 8, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
};

/* ReimuA Rank 9 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank9[] = {
    {5, 0, -8.0f, 0.0f, 12.0f, 12.0f, RADIANS(-97.0), 12.0f, 23, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, -8.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 8.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 24, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 8.0f, 0.0f, 12.0f, 12.0f, RADIANS(-83.0), 12.0f, 23, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {16, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-110.0), 10.0f, 10, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {16, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-70.0), 10.0f, 10, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {16, 4, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-130.0), 10.0f, 8, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {16, 4, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-50.0), 10.0f, 8, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {16, 8, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-150.0), 10.0f, 7, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {16, 8, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-30.0), 10.0f, 7, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {16, 12, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-170.0), 10.0f, 10, 1, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND},
    {16, 12, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-9.9999979), 10.0f, 10, 2, PLAYER_BULLET_TYPE_1,
     ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET, NO_SOUND}, /*IEE-754 moment*/
};

CharacterPowerData g_CharacterPowerDataReimuA[] = {
    /* Rank1   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank1), 8, g_CharacterPowerBulletDataReimuARank1},
    /* Rank2   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank2), 16, g_CharacterPowerBulletDataReimuARank2},
    /* Rank3   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank3), 32, g_CharacterPowerBulletDataReimuARank3},
    /* Rank4   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank4), 48, g_CharacterPowerBulletDataReimuARank4},
    /* Rank5   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank5), 64, g_CharacterPowerBulletDataReimuARank5},
    /* Rank6   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank6), 80, g_CharacterPowerBulletDataReimuARank6},
    /* Rank7   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank7), 96, g_CharacterPowerBulletDataReimuARank7},
    /* Rank8   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank8), 127, g_CharacterPowerBulletDataReimuARank8},
    /* Rank9   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuARank9), 999, g_CharacterPowerBulletDataReimuARank9},
};

/* ----------ReimuB---------- */

/* ReimuB Rank 1 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank1[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 48, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
};

/* ReimuB Rank 2 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank2[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 48, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {15, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {15, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
};

/* ReimuB Rank 3 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank3[] = {
    {5, 0, -4.0f, 0.0f, 12.0f, 12.0f, RADIANS(-91.0), 12.0f, 32, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 4.0f, 0.0f, 12.0f, 12.0f, RADIANS(-89.0), 12.0f, 32, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {10, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {10, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
};

/* ReimuB Rank 4 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank4[] = {
    {5, 0, -4.0f, 0.0f, 12.0f, 12.0f, RADIANS(-91.0), 12.0f, 30, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 4.0f, 0.0f, 12.0f, 12.0f, RADIANS(-89.0), 12.0f, 30, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {8, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {8, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
};

/* ReimuB Rank 5 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank5[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-97.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 28, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-83.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {8, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {8, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
};

/* ReimuB Rank 6 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank6[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-97.0), 12.0f, 16, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 27, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-83.0), 12.0f, 16, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 8.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {5, 0, 8.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 12, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {8, 0, -8.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {8, 0, -8.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
};

/* ReimuB Rank 7 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank7[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 12.0f, 16, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 22, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 12.0f, 16, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {3, 0, 8.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, SOUND_SHOOT},
    {3, 0, 8.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {5, 0, -8.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {5, 0, -8.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
};

/* ReimuB Rank 8 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank8[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-106.0), 12.0f, 9, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 12.0f, 17, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 12.0f, 17, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-74.0), 12.0f, 9, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {3, 0, 12.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, SOUND_SHOOT},
    {3, 0, 12.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {5, 0, -12.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {5, 0, -12.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {10, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {10, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
};

/* ReimuB Rank 9 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank9[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-106.0), 12.0f, 9, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 12.0f, 17, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 12.0f, 17, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-74.0), 12.0f, 9, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {3, 0, 12.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, SOUND_SHOOT},
    {3, 0, 12.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {3, 0, -12.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {3, 0, -12.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {5, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 1, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
    {5, 0, 0.0f, -16.0f, 12.0f, 40.0f, RADIANS(-90.0), 22.0f, 10, 2, PLAYER_BULLET_TYPE_0,
     ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET, NO_SOUND},
};

CharacterPowerData g_CharacterPowerDataReimuB[] = {
    /* Rank1   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank1), 8, g_CharacterPowerBulletDataReimuBRank1},
    /* Rank2   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank2), 16, g_CharacterPowerBulletDataReimuBRank2},
    /* Rank3   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank3), 32, g_CharacterPowerBulletDataReimuBRank3},
    /* Rank4   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank4), 48, g_CharacterPowerBulletDataReimuBRank4},
    /* Rank5   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank5), 64, g_CharacterPowerBulletDataReimuBRank5},
    /* Rank6   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank6), 80, g_CharacterPowerBulletDataReimuBRank6},
    /* Rank7   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank7), 96, g_CharacterPowerBulletDataReimuBRank7},
    /* Rank8   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank8), 127, g_CharacterPowerBulletDataReimuBRank8},
    /* Rank9   */ {ARRAY_SIZE(g_CharacterPowerBulletDataReimuBRank9), 999, g_CharacterPowerBulletDataReimuBRank9},
};

/* ----------MarisaA---------- */

/* MarisaA Rank 1 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank1[] = {
    {5, 0, 0.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 48, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
};

/* MarisaA Rank 2 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank2[] = {
    {5, 0, 0.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 36, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 3.0f, 18, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 3.0f, 18, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
};

/* MarisaA Rank 3 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank3[] = {
    {5, 0, 0.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 32, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 3.0f, 16, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
    {30, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 3.0f, 16, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
    {30, 15, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 3.0f, 10, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
    {30, 15, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 3.0f, 10, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
};

/* MarisaA Rank 4 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank4[] = {
    {5, 0, 0.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 32, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 3.0f, 15, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 3.0f, 15, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
    {15, 15, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 3.0f, 10, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
    {15, 15, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 3.0f, 10, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1, NO_SOUND},
};

/* MarisaA Rank 5 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank5[] = {
    {5, 0, 0.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 32, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 3.0f, 16, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2, NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 3.0f, 16, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2, NO_SOUND},
    {15, 20, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 3.0f, 11, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2, NO_SOUND},
    {15, 20, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 3.0f, 11, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2, NO_SOUND},
};

/* MarisaA Rank 6 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank6[] = {
    {5, 0, -8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 16, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 16, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 3.0f, 16, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2, NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 3.0f, 16, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2, NO_SOUND},
    {15, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 3.0f, 10, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2, NO_SOUND},
    {15, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 3.0f, 10, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2, NO_SOUND},
};

/* MarisaA Rank 7 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank7[] = {
    {5, 0, -8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 13, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 13, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 3.0f, 16, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 3.0f, 16, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
    {10, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 3.0f, 10, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
    {10, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 3.0f, 10, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
};

/* MarisaA Rank 8 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank8[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-94.0), 12.0f, 8, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 12, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-86.0), 12.0f, 8, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 3.0f, 15, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 3.0f, 15, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
    {10, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 3.0f, 10, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
    {10, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 3.0f, 10, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-78.0), 3.0f, 9, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
    {15, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-102.0), 3.0f, 9, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3, NO_SOUND},
};

/* MarisaA Rank 9 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank9[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-94.0), 12.0f, 8, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 12, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-86.0), 12.0f, 8, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 3.0f, 14, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4, NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 3.0f, 14, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4, NO_SOUND},
    {10, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-82.0), 3.0f, 10, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4, NO_SOUND},
    {10, 5, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-98.0), 3.0f, 10, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4, NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-75.0), 3.0f, 10, 1, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4, NO_SOUND},
    {10, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-105.0), 3.0f, 10, 2, PLAYER_BULLET_TYPE_2,
     ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4, NO_SOUND},
};

CharacterPowerData g_CharacterPowerDataMarisaA[] = {
    /* Rank1   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank1), 8, g_CharacterPowerBulletDataMarisaARank1},
    /* Rank2   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank2), 16, g_CharacterPowerBulletDataMarisaARank2},
    /* Rank3   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank3), 32, g_CharacterPowerBulletDataMarisaARank3},
    /* Rank4   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank4), 48, g_CharacterPowerBulletDataMarisaARank4},
    /* Rank5   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank5), 64, g_CharacterPowerBulletDataMarisaARank5},
    /* Rank6   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank6), 80, g_CharacterPowerBulletDataMarisaARank6},
    /* Rank7   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank7), 96, g_CharacterPowerBulletDataMarisaARank7},
    /* Rank8   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank8), 127, g_CharacterPowerBulletDataMarisaARank8},
    /* Rank9   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaARank9), 999, g_CharacterPowerBulletDataMarisaARank9},
};

/* ----------MarisaB---------- */

/* MarisaB Rank 1 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank1[] = {
    {5, 0, 0.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 48, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
};

/* MarisaB Rank 2 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank2[] = {
    {5, 0, 0.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 32, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {120, 0, 0.0f, 0.0f, 10.0f, 480.0f, RADIANS(-90.0), 3.0f, 3, 1, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1, NO_SOUND},
    {120, 1, 0.0f, 0.0f, 10.0f, 480.0f, RADIANS(-90.0), 3.0f, 3, 2, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1, NO_SOUND},
};

/* MarisaB Rank 3 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank3[] = {
    {5, 0, 0.0f, -8.0f, 12.0f, 24.0f, RADIANS(-90.0), 12.0f, 32, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {170, 0, 0.0f, 0.0f, 10.0f, 480.0f, RADIANS(-90.0), 3.0f, 3, 1, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1, NO_SOUND},
    {170, 1, 0.0f, 0.0f, 10.0f, 480.0f, RADIANS(-90.0), 3.0f, 3, 2, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1, NO_SOUND},
};

/* MarisaB Rank 4 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank4[] = {
    {5, 0, -8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-92.0), 12.0f, 22, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-88.0), 12.0f, 22, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {200, 0, 0.0f, 0.0f, 10.0f, 480.0f, RADIANS(-90.0), 3.0f, 3, 1, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1, NO_SOUND},
    {200, 1, 0.0f, 0.0f, 10.0f, 480.0f, RADIANS(-90.0), 3.0f, 3, 2, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1, NO_SOUND},
};

/* MarisaB Rank 5 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank5[] = {
    {5, 0, -8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-92.0), 12.0f, 22, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-88.0), 12.0f, 22, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {210, 0, 0.0f, 0.0f, 10.0f, 480.0f, RADIANS(-90.0), 3.0f, 3, 1, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2, NO_SOUND},
    {210, 1, 0.0f, 0.0f, 10.0f, 480.0f, RADIANS(-90.0), 3.0f, 3, 2, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2, NO_SOUND},
};

/* MarisaB Rank 6 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank6[] = {
    {5, 0, -8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-92.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 8.0f, -8.0f, 12.0f, 24.0f, RADIANS(-88.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {230, 0, 0.0f, 0.0f, 15.0f, 480.0f, RADIANS(-90.0), 3.0f, 4, 1, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2, NO_SOUND},
    {230, 1, 0.0f, 0.0f, 15.0f, 480.0f, RADIANS(-90.0), 3.0f, 4, 2, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2, NO_SOUND},
};

/* MarisaB Rank 7 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank7[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 12.0f, 15, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 12.0f, 15, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {250, 0, 0.0f, 0.0f, 15.0f, 480.0f, RADIANS(-90.0), 3.0f, 4, 1, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2, NO_SOUND},
    {250, 1, 0.0f, 0.0f, 15.0f, 480.0f, RADIANS(-90.0), 3.0f, 4, 2, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2, NO_SOUND},
};

/* MarisaB Rank 8 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank8[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 12.0f, 15, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 12.0f, 15, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {270, 0, 0.0f, 0.0f, 20.0f, 480.0f, RADIANS(-90.0), 3.0f, 5, 1, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_3, NO_SOUND},
    {270, 1, 0.0f, 0.0f, 20.0f, 480.0f, RADIANS(-90.0), 3.0f, 5, 2, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_3, NO_SOUND},
};

/* MarisaB Rank 9 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank9[] = {
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-100.0), 12.0f, 12, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     SOUND_SHOOT},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-95.0), 12.0f, 15, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-90.0), 12.0f, 20, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-85.0), 12.0f, 15, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {5, 0, 0.0f, 0.0f, 12.0f, 12.0f, RADIANS(-80.0), 12.0f, 12, 0, PLAYER_BULLET_TYPE_0, ANM_SCRIPT_PLAYER_BULLET,
     NO_SOUND},
    {330, 0, 0.0f, 0.0f, 20.0f, 480.0f, RADIANS(-90.0), 3.0f, 6, 1, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_3, NO_SOUND},
    {330, 1, 0.0f, 0.0f, 20.0f, 480.0f, RADIANS(-90.0), 3.0f, 6, 2, PLAYER_LASER,
     ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_3, NO_SOUND},
};

CharacterPowerData g_CharacterPowerDataMarisaB[] = {
    /* Rank1   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank1), 8, g_CharacterPowerBulletDataMarisaBRank1},
    /* Rank2   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank2), 16, g_CharacterPowerBulletDataMarisaBRank2},
    /* Rank3   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank3), 32, g_CharacterPowerBulletDataMarisaBRank3},
    /* Rank4   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank4), 48, g_CharacterPowerBulletDataMarisaBRank4},
    /* Rank5   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank5), 64, g_CharacterPowerBulletDataMarisaBRank5},
    /* Rank6   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank6), 80, g_CharacterPowerBulletDataMarisaBRank6},
    /* Rank7   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank7), 96, g_CharacterPowerBulletDataMarisaBRank7},
    /* Rank8   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank8), 127, g_CharacterPowerBulletDataMarisaBRank8},
    /* Rank9   */ {ARRAY_SIZE(g_CharacterPowerBulletDataMarisaBRank9), 999, g_CharacterPowerBulletDataMarisaBRank9},
};
} // namespace th06

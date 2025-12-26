#include "AnmIdx.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"

/* ----------ReimuA---------- */

/* ReimuA Rank 1 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank1[1] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      48,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
};

/* ReimuA Rank 2 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank2[3] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      48,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-120.0),
      10.0,
      14,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-60.0),
      10.0,
      14,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuA Rank 3 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank3[4] = {
    {
      5,
      0,
      { -4.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-91.0),
      12.0,
      30,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 4.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-89.0),
      12.0,
      30,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-120.0),
      10.0,
      14,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-60.0),
      10.0,
      14,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuA Rank 4 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank4[5] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-96.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      30,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-84.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-120.0),
      10.0,
      14,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-60.0),
      10.0,
      14,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuA Rank 5 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank5[5] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-97.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      30,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-83.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-120.0),
      10.0,
      12,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-60.0),
      10.0,
      12,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuA Rank 6 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank6[7] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-97.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      29,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-83.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-120.0),
      10.0,
      9,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-60.0),
      10.0,
      9,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-150.0),
      10.0,
      12,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-30.0),
      10.0,
      12,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuA Rank 7 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank7[9] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-97.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      28,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-83.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-110.0),
      10.0,
      10,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-70.0),
      10.0,
      10,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      10,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-130.0),
      10.0,
      9,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      10,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-50.0),
      10.0,
      9,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      20,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-150.0),
      10.0,
      11,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      30,
      20,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-30.0),
      10.0,
      11,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuA Rank 8 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank8[9] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-97.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      28,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-83.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-110.0),
      10.0,
      8,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-70.0),
      10.0,
      8,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      15,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-130.0),
      10.0,
      8,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      15,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-50.0),
      10.0,
      8,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      15,
      10,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-150.0),
      10.0,
      8,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      15,
      10,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-30.0),
      10.0,
      8,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuA Rank 9 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuARank9[12] = {
    {
      5,
      0,
      { -8.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-97.0),
      12.0,
      23,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { -8.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 8.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      24,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 8.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-83.0),
      12.0,
      23,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      16,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-110.0),
      10.0,
      10,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      16,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-70.0),
      10.0,
      10,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      16,
      4,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-130.0),
      10.0,
      8,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      16,
      4,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-50.0),
      10.0,
      8,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      16,
      8,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-150.0),
      10.0,
      7,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      16,
      8,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-30.0),
      10.0,
      7,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      16,
      12,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-170.0),
      10.0,
      10,
      1,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    },
    {
      16,
      12,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-9.9999979),
      10.0,
      10,
      2,
      BULLET_TYPE_1,
      ANM_SCRIPT_PLAYER_REIMU_A_ORB_BULLET,
      NO_SOUND
    }, /*IEE-754 moment*/
};

CharacterPowerData g_CharacterPowerDataReimuA[9] = {
    /* Rank1   */ { 1, 8, g_CharacterPowerBulletDataReimuARank1 },
    /* Rank2   */ { 3, 16, g_CharacterPowerBulletDataReimuARank2 },
    /* Rank3   */ { 4, 32, g_CharacterPowerBulletDataReimuARank3 },
    /* Rank4   */ { 5, 48, g_CharacterPowerBulletDataReimuARank4 },
    /* Rank5   */ { 5, 64, g_CharacterPowerBulletDataReimuARank5 },
    /* Rank6   */ { 7, 80, g_CharacterPowerBulletDataReimuARank6 },
    /* Rank7   */ { 9, 96, g_CharacterPowerBulletDataReimuARank7 },
    /* Rank8   */ { 9, 127, g_CharacterPowerBulletDataReimuARank8 },
    /* Rank9   */ { 12, 999, g_CharacterPowerBulletDataReimuARank9 },
};

/* ----------ReimuB---------- */

/* ReimuB Rank 1 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank1[1] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      48,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
};

/* ReimuB Rank 2 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank2[3] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      48,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      15,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuB Rank 3 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank3[4] = {
    {
      5,
      0,
      { -4.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-91.0),
      12.0,
      32,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 4.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-89.0),
      12.0,
      32,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuB Rank 4 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank4[4] = {
    {
      5,
      0,
      { -4.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-91.0),
      12.0,
      30,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 4.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-89.0),
      12.0,
      30,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      8,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      8,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuB Rank 5 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank5[5] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-97.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      28,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-83.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      8,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      8,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuB Rank 6 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank6[7] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-97.0),
      12.0,
      16,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      27,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-83.0),
      12.0,
      16,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 8.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 8.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      12,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      8,
      0,
      { -8.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      8,
      0,
      { -8.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuB Rank 7 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank7[7] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      12.0,
      16,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      22,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      12.0,
      16,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND },
    {
      3,
      0,
      { 8.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      SOUND_SHOOT
    },
    {
      3,
      0,
      { 8.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND },
    {
      5,
      0,
      { -8.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND },
    {
      5,
      0,
      { -8.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuB Rank 8 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank8[11] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-106.0),
      12.0,
      9,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      12.0,
      17,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      12.0,
      17,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-74.0),
      12.0,
      9,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      3,
      0,
      { 12.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      SOUND_SHOOT
    },
    {
      3,
      0,
      { 12.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { -12.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { -12.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
};

/* ReimuB Rank 9 */
CharacterPowerBulletData g_CharacterPowerBulletDataReimuBRank9[11] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-106.0),
      12.0,
      9,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      12.0,
      17,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      12.0,
      17,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-74.0),
      12.0,
      9,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      3,
      0,
      { 12.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      SOUND_SHOOT
    },
    {
      3,
      0,
      { 12.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      3,
      0,
      { -12.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      3,
      0,
      { -12.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      1,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, -16.0 },
      { 12.0, 40.0 },
      RADIANS(-90.0),
      22.0,
      10,
      2,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_REIMU_B_ORB_BULLET,
      NO_SOUND
    },
};

CharacterPowerData g_CharacterPowerDataReimuB[9] = {
    /* Rank1   */ { 1, 8, g_CharacterPowerBulletDataReimuBRank1 },
    /* Rank2   */ { 3, 16, g_CharacterPowerBulletDataReimuBRank2 },
    /* Rank3   */ { 4, 32, g_CharacterPowerBulletDataReimuBRank3 },
    /* Rank4   */ { 4, 48, g_CharacterPowerBulletDataReimuBRank4 },
    /* Rank5   */ { 5, 64, g_CharacterPowerBulletDataReimuBRank5 },
    /* Rank6   */ { 7, 80, g_CharacterPowerBulletDataReimuBRank6 },
    /* Rank7   */ { 7, 96, g_CharacterPowerBulletDataReimuBRank7 },
    /* Rank8   */ { 11, 127, g_CharacterPowerBulletDataReimuBRank8 },
    /* Rank9   */ { 11, 999, g_CharacterPowerBulletDataReimuBRank9 },
};

/* ----------MarisaA---------- */

/* MarisaA Rank 1 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank1[1] = {
    {
      5,
      0,
      { 0.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      48,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
};

/* MarisaA Rank 2 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank2[3] = {
    {
      5,
      0,
      { 0.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      36,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      3.0,
      18,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      3.0,
      18,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
};

/* MarisaA Rank 3 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank3[5] = {
    {
      5,
      0,
      { 0.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      32,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      3.0,
      16,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
    {
      30,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      3.0,
      16,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
    {
      30,
      15,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      3.0,
      10,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
    {
      30,
      15,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      3.0,
      10,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
};

/* MarisaA Rank 4 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank4[5] = {
    {
      5,
      0,
      { 0.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      32,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      3.0,
      15,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      3.0,
      15,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
    {
      15,
      15,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      3.0,
      10,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
    {
      15,
      15,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      3.0,
      10,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_1,
      NO_SOUND
    },
};

/* MarisaA Rank 5 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank5[5] = {
    {
      5,
      0,
      { 0.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      32,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      3.0,
      16,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      3.0,
      16,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2,
      NO_SOUND
    },
    {
      15,
      20,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      3.0,
      11,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2,
      NO_SOUND
    },
    {
      15,
      20,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      3.0,
      11,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2,
      NO_SOUND
    },
};

/* MarisaA Rank 6 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank6[6] = {
    {
      5,
      0,
      { -8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      16,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      16,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      3.0,
      16,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      3.0,
      16,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2,
      NO_SOUND
    },
    {
      15,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      3.0,
      10,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2,
      NO_SOUND
    },
    {
      15,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      3.0,
      10,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_2,
      NO_SOUND
    },
};

/* MarisaA Rank 7 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank7[6] = {
    {
      5,
      0,
      { -8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      13,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      13,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      3.0,
      16,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      3.0,
      16,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
    {
      10,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      3.0,
      10,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
    {
      10,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      3.0,
      10,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
};

/* MarisaA Rank 8 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank8[9] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-94.0),
      12.0,
      8,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      12,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-86.0),
      12.0,
      8,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      3.0,
      15,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      3.0,
      15,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
    {
      10,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      3.0,
      10,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
    {
      10,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      3.0,
      10,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-78.0),
      3.0,
      9,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
    {
      15,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-102.0),
      3.0,
      9,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_3,
      NO_SOUND
    },
};

/* MarisaA Rank 9 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaARank9[9] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-94.0),
      12.0,
      8,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      12,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-86.0),
      12.0,
      8,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      3.0,
      14,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      3.0,
      14,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4,
      NO_SOUND
    },
    {
      10,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-82.0),
      3.0,
      10,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4,
      NO_SOUND
    },
    {
      10,
      5,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-98.0),
      3.0,
      10,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-75.0),
      3.0,
      10,
      1,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4,
      NO_SOUND
    },
    {
      10,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-105.0),
      3.0,
      10,
      2,
      BULLET_TYPE_2,
      ANM_SCRIPT_PLAYER_MARISA_A_ORB_BULLET_4,
      NO_SOUND
    },
};

CharacterPowerData g_CharacterPowerDataMarisaA[9] = {
    /* Rank1   */ { 1, 8, g_CharacterPowerBulletDataMarisaARank1 },
    /* Rank2   */ { 3, 16, g_CharacterPowerBulletDataMarisaARank2 },
    /* Rank3   */ { 5, 32, g_CharacterPowerBulletDataMarisaARank3 },
    /* Rank4   */ { 5, 48, g_CharacterPowerBulletDataMarisaARank4 },
    /* Rank5   */ { 5, 64, g_CharacterPowerBulletDataMarisaARank5 },
    /* Rank6   */ { 6, 80, g_CharacterPowerBulletDataMarisaARank6 },
    /* Rank7   */ { 6, 96, g_CharacterPowerBulletDataMarisaARank7 },
    /* Rank8   */ { 9, 127, g_CharacterPowerBulletDataMarisaARank8 },
    /* Rank9   */ { 9, 999, g_CharacterPowerBulletDataMarisaARank9 },
};

/* ----------MarisaB---------- */

/* MarisaB Rank 1 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank1[1] = {
    {
      5,
      0,
      { 0.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      48,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
};

/* MarisaB Rank 2 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank2[3] = {
    {
      5,
      0,
      { 0.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      32,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      120,
      0,
      { 0.0, 0.0 },
      { 10.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      3,
      1,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1,
      NO_SOUND
    },
    {
      120,
      1,
      { 0.0, 0.0 },
      { 10.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      3,
      2,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1,
      NO_SOUND
    },
};

/* MarisaB Rank 3 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank3[3] = {
    {
      5,
      0,
      { 0.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-90.0),
      12.0,
      32,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      170,
      0,
      { 0.0, 0.0 },
      { 10.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      3,
      1,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1,
      NO_SOUND
    },
    {
      170,
      1,
      { 0.0, 0.0 },
      { 10.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      3,
      2,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1,
      NO_SOUND
    },
};

/* MarisaB Rank 4 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank4[4] = {
    {
      5,
      0,
      { -8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-92.0),
      12.0,
      22,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-88.0),
      12.0,
      22,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      200,
      0,
      { 0.0, 0.0 },
      { 10.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      3,
      1,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1,
      NO_SOUND
    },
    {
      200,
      1,
      { 0.0, 0.0 },
      { 10.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      3,
      2,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_1,
      NO_SOUND
    },
};

/* MarisaB Rank 5 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank5[4] = {
    {
      5,
      0,
      { -8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-92.0),
      12.0,
      22,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-88.0),
      12.0,
      22,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    { 210,
      0,
      { 0.0, 0.0 },
      { 10.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      3,
      1,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2,
      NO_SOUND
    },
    {
      210,
      1,
      { 0.0, 0.0 },
      { 10.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      3,
      2,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2,
      NO_SOUND
    },
};

/* MarisaB Rank 6 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank6[4] = {
    {
      5,
      0,
      { -8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-92.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 8.0, -8.0 },
      { 12.0, 24.0 },
      RADIANS(-88.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      230,
      0,
      { 0.0, 0.0 },
      { 15.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      4,
      1,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2,
      NO_SOUND
    },
    {
      230,
      1,
      { 0.0, 0.0 },
      { 15.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      4,
      2,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2,
      NO_SOUND
    },
};

/* MarisaB Rank 7 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank7[5] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      12.0,
      15,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      12.0,
      15,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      250,
      0,
      { 0.0, 0.0 },
      { 15.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      4,
      1,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2,
      NO_SOUND
    },
    {
      250,
      1,
      { 0.0, 0.0 },
      { 15.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      4,
      2,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_2,
      NO_SOUND
    },
};

/* MarisaB Rank 8 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank8[5] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      12.0,
      15,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      12.0,
      15,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      270,
      0,
      { 0.0, 0.0 },
      { 20.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      5,
      1,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_3,
      NO_SOUND
    },
    {
      270,
      1,
      { 0.0, 0.0 },
      { 20.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      5,
      2,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_3,
      NO_SOUND
    },
};

/* MarisaB Rank 9 */
CharacterPowerBulletData g_CharacterPowerBulletDataMarisaBRank9[7] = {
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-100.0),
      12.0,
      12,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      SOUND_SHOOT
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-95.0),
      12.0,
      15,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-90.0),
      12.0,
      20,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-85.0),
      12.0,
      15,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      5,
      0,
      { 0.0, 0.0 },
      { 12.0, 12.0 },
      RADIANS(-80.0),
      12.0,
      12,
      0,
      BULLET_TYPE_0,
      ANM_SCRIPT_PLAYER_BULLET,
      NO_SOUND
    },
    {
      330,
      0,
      { 0.0, 0.0 },
      { 20.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      6,
      1,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_3,
      NO_SOUND
    },
    {
      330,
      1,
      { 0.0, 0.0 },
      { 20.0, 480.0 },
      RADIANS(-90.0),
      3.0,
      6,
      2,
      BULLET_TYPE_LASER,
      ANM_SCRIPT_PLAYER_MARISA_B_ORB_LASER_3,
      NO_SOUND
    },
};

CharacterPowerData g_CharacterPowerDataMarisaB[9] = {
    /* Rank1   */ { 1, 8, g_CharacterPowerBulletDataMarisaBRank1 },
    /* Rank2   */ { 3, 16, g_CharacterPowerBulletDataMarisaBRank2 },
    /* Rank3   */ { 3, 32, g_CharacterPowerBulletDataMarisaBRank3 },
    /* Rank4   */ { 4, 48, g_CharacterPowerBulletDataMarisaBRank4 },
    /* Rank5   */ { 4, 64, g_CharacterPowerBulletDataMarisaBRank5 },
    /* Rank6   */ { 4, 80, g_CharacterPowerBulletDataMarisaBRank6 },
    /* Rank7   */ { 5, 96, g_CharacterPowerBulletDataMarisaBRank7 },
    /* Rank8   */ { 5, 127, g_CharacterPowerBulletDataMarisaBRank8 },
    /* Rank9   */ { 7, 999, g_CharacterPowerBulletDataMarisaBRank9 },
};

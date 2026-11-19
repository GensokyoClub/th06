#pragma once
#include "AnmVm.hpp"
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

namespace th06
{
struct EnemyShooter;

enum BulletAimMode
{
    FAN_AIMED,
    FAN,
    CIRCLE_AIMED,
    CIRCLE,
    OFFSET_CIRCLE_AIMED,
    OFFSET_CIRCLE,
    RANDOM_ANGLE,
    RANDOM_SPEED,
    RANDOM,
};
enum LaserAimMode
{
    LASER_AIMED,
    LASER_UNAIMED
};

// Pellets
// 16 colors
#define BULLET_SIZE_TINY 8
// Everything else
// 16 colors
#define BULLET_SIZE_SMALL 16
// Fireballs
// 4 "colors"
#define BULLET_SIZE_MEDIUM 30
// Big balls, daggers
// 8 colors
#define BULLET_SIZE_LARGE 32
// Bubbles
// 4 colors
#define BULLET_SIZE_HUGE 64

enum BulletType
{
    // 16 colors
    BULLET_PELLET,
    BULLET_RING_BALL,
    BULLET_RICE,
    BULLET_BALL,
    BULLET_KUNAI,
    BULLET_SHARD,
    // 8 colors
    BULLET_BIG_BALL,
    // 4 colors
    BULLET_FIREBALL,
    // 8 colors
    BULLET_DAGGER,
    // 4 colors
    BULLET_BUBBLE,
    ENUM_VALUE_COUNT_DECLARE(BulletType)
};
enum LaserSprite
{
    LASER_LINE, // Dedicated laser sprite
    LASER_BEAM, // Stretched ball sprite
};

enum BulletColor16
{
    BULLET_GRAY,
    BULLET_DARK_RED,
    BULLET_RED,
    BULLET_DARK_PURPLE,
    BULLET_PURPLE,
    BULLET_DARK_BLUE,
    BULLET_BLUE,
    BULLET_DARK_CYAN,
    BULLET_CYAN,
    BULLET_DARK_GREEN,
    BULLET_GREEN,
    BULLET_LIME,
    BULLET_DARK_YELLOW,
    BULLET_YELLOW,
    BULLET_ORANGE,
    BULLET_WHITE
};
enum BulletColor8
{
    BULLET_GRAY8,
    BULLET_RED8,
    BULLET_PURPLE8,
    BULLET_BLUE8,
    BULLET_CYAN8,
    BULLET_GREEN8,
    BULLET_YELLOW8,
    BULLET_WHITE8
};
enum BulletColor4
{
    BULLET_RED4,
    BULLET_BLUE4,
    BULLET_GREEN4,
    BULLET_YELLOW4
};

struct BulletBase
{
    AnmVm vm;
    AnmVm spawnEffectFast;
    AnmVm spawnEffectNormal;
    AnmVm spawnEffectSlow;
    AnmVm despawnEffect;

    D3DXVECTOR3 grazeSize;
    u8 unk_55c; // possibly an old width field?
    u8 height;
    alignment_padding(0x2);
};
ZUN_ASSERT_TYPE(BulletBase, 0x560, 4);

enum BulletState
{
    BULLET_STATE_INACTIVE,
    BULLET_STATE_FIRED,
    BULLET_STATE_SPAWNING_FAST,
    BULLET_STATE_SPAWNING_NORMAL,
    BULLET_STATE_SPAWNING_SLOW,
    BULLET_STATE_DESPAWNING,
};
enum LaserState
{
    LASER_STATE_START_DELAY,
    LASER_STATE_ACTIVE,
    LASER_STATE_DESPAWNING
};

enum BulletEffectType
{
    EX_SPEEDUP = 0x1,
    EX_SPAWN_EFFECT_SHORT = 0x2,
    EX_SPAWN_EFFECT = 0x4,
    EX_SPAWN_EFFECT_LONG = 0x8,
    EX_ACCELERATION = 0x10,
    EX_VELOCITY = 0x20,
    EX_ANGLE_ADD = 0x40,
    EX_ANGLE_PLAYER = 0x80,
    EX_ANGLE_SET = 0x100,
    EX_SPAWN_SOUND = 0x200,
    EX_BOUNCE_TBLR = 0x400,
    EX_BOUNCE_TLR = 0x800
};
#define EX_ANGLE (EX_ANGLE_ADD | EX_ANGLE_PLAYER | EX_ANGLE_SET)
#define EX_BOUNCE (EX_BOUNCE_TBLR | EX_BOUNCE_TLR)

enum LaserFlags
{
    LASER_FLAG_FADE_IN_OUT = 0x1,
    LASER_FLAG_UNUSED = 0x2
};

struct Bullet : BulletBase
{
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 exAcceleration; // EX_ACCELERATION
    f32 speed;
    f32 exVelocity; // EX_VELOCITY
    f32 exSpeed; // EX_ANGLE, EX_BOUNCE
    f32 angle;
    f32 exAngularVelocity; // EX_VELOCITY
    f32 exAngle; // EX_ANGLE
    ZunTimer timer;
    i32 exDuration; // EX_ACCELERATION, EX_VELOCITY
    i32 exInterval; // EX_ANGLE
    i32 exCount; // EX_ANGLE, EX_BOUNCE
    i32 exCountMax; // EX_ANGLE, EX_BOUNCE
    u16 exFlags;
    i16 color;
    unreferenced_fields(0x2);
    u16 state;
    u16 outOfBoundsTime;
    u8 unk_5c2;
    u8 isGrazed;
};
ZUN_ASSERT_TYPE(Bullet, 0x5c4, 4);

struct Laser
{
    AnmVm vm;
    AnmVm baseGlowVm;
    D3DXVECTOR3 pos;
    f32 angle;
    f32 startOffset;
    f32 endOffset;
    f32 length;
    f32 width;
    f32 speed;
    i32 startTime;
    i32 hitboxStartTime;
    i32 duration;
    i32 despawnDuration;
    i32 hitboxEndDelay;
    ZunBool inUse;
    ZunTimer timer;
    u16 flags;
    i16 color;
    u8 state;
    alignment_padding(0x3);
};
ZUN_ASSERT_TYPE(Laser, 0x270, 4);

#define MAX_ENEMY_BULLET_TYPES 16
#define MAX_ENEMY_BULLETS 640
#define MAX_ENEMY_LASERS 64

enum CancelType
{
    BULLET_CLEAR,
    BULLET_CANCEL
};

struct BulletManager
{
    BulletManager();

    void DespawnBullets(ZunBool spawnCancelItems);
    void InitializeToZero();

    void CancelAllBullets();

    i32 DespawnBulletsForScore(i32 maxBonusScore, ZunBool spawnCancelItems);

    ZunResult SpawnBulletPattern(EnemyShooter *bulletProps);
    Laser *SpawnLaserPattern(EnemyShooter *bulletProps);
    u32 SpawnSingleBullet(EnemyShooter *bulletProps, i32 bulletIdx1, i32 bulletIdx2, f32 angle);

    BulletBase bulletTemplates[MAX_ENEMY_BULLET_TYPES];
    Bullet bullets[MAX_ENEMY_BULLETS];
    Laser lasers[MAX_ENEMY_LASERS];
    i32 nextBulletIndex;
    i32 bulletCount;
    ZunTimer time;
    const char *bulletAnmPath;
};
ZUN_ASSERT_TYPE(BulletManager, 0xf5c18, 4);

ZunResult BulletManager_RegisterChain(const char *bulletAnmPath);
void BulletManager_CutChain();

extern D3DCOLOR *g_EffectsColor;
extern BulletManager g_BulletManager;
} // namespace th06

#pragma once

#include "AnmVm.hpp"
#include "ZunResult.hpp"
#include "inttypes.hpp"

struct EnemyBulletShooter;
struct EnemyLaserShooter;

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

struct BulletTypeSprites
{
    AnmVm spriteBullet;
    AnmVm spriteSpawnEffectFast;
    AnmVm spriteSpawnEffectNormal;
    AnmVm spriteSpawnEffectSlow;
    AnmVm spriteSpawnEffectDonut;

    ZunVec3 grazeSize;
    u8 unk_55c;
    u8 bulletHeight;
};

struct Bullet
{
    BulletTypeSprites sprites;
    ZunVec3 pos;
    ZunVec3 velocity;
    ZunVec3 ex4Acceleration;
    f32 speed;
    f32 ex5Float0;
    f32 dirChangeSpeed;
    f32 angle;
    f32 ex5Float1;
    f32 dirChangeRotation;
    ZunTimer timer;
    i32 ex5Int0;
    i32 dirChangeInterval;
    i32 dirChangeNumTimes;
    i32 dirChangeMaxTimes;
    u16 exFlags;
    i16 spriteOffset;
    u16 unk_5bc;
    u16 state;
    u16 unk_5c0;
    u8 unk_5c2;
    u8 isGrazed;
};

struct Laser
{
    AnmVm vm0;
    AnmVm vm1;
    ZunVec3 pos;
    f32 angle;
    f32 startOffset;
    f32 endOffset;
    f32 startLength;
    f32 width;
    f32 speed;
    i32 startTime;
    i32 grazeDelay;
    i32 duration;
    i32 endTime;
    i32 grazeInterval;
    i32 inUse;
    ZunTimer timer;
    u16 flags;
    i16 color;
    u8 state;
};

struct BulletManager
{
    BulletManager();
    static ZunResult RegisterChain(char *bulletAnmPath);
    static void CutChain();
    static ZunResult AddedCallback(BulletManager *mgr);
    static ZunResult DeletedCallback(BulletManager *mgr);
    static ChainCallbackResult OnUpdate(BulletManager *mgr);
    static ChainCallbackResult OnDraw(BulletManager *mgr);

    static void DrawBulletNoHwVertex(Bullet *bullet);
    static void DrawBullet(Bullet *bullet);

    void RemoveAllBullets(bool turnIntoItem);
    void InitializeToZero();

    void TurnAllBulletsIntoPoints();

    i32 DespawnBullets(i32 maxBonusScore, bool awardPoints);
    ZunResult SpawnBulletPattern(EnemyBulletShooter *bulletProps);
    Laser *SpawnLaserPattern(EnemyLaserShooter *bulletProps);
    u32 SpawnSingleBullet(EnemyBulletShooter *bulletProps, i32 bulletIdx1, i32 bulletIdx2, f32 angle);
    BulletTypeSprites bulletTypeTemplates[16];
    Bullet bullets[640];
    Laser lasers[64];
    i32 nextBulletIndex;
    i32 bulletCount;
    ZunTimer time;
    char *bulletAnmPath;
};

extern u32 *g_EffectsColor;
extern BulletManager g_BulletManager;

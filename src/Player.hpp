#pragma once

#include "AnmManager.hpp"
#include "AnmVm.hpp"
#include "BulletManager.hpp"
#include "Chain.hpp"
#include "GameManager.hpp"
#include "ZunMath.hpp"
#include "inttypes.hpp"

namespace th06
{
struct Player;

enum PlayerDirection
{
    MOVEMENT_NONE,
    MOVEMENT_UP,
    MOVEMENT_DOWN,
    MOVEMENT_LEFT,
    MOVEMENT_RIGHT,
    MOVEMENT_UP_LEFT,
    MOVEMENT_UP_RIGHT,
    MOVEMENT_DOWN_LEFT,
    MOVEMENT_DOWN_RIGHT
};

enum Character
{
    CHARA_REIMU,
    CHARA_MARISA,
};

enum ShotType
{
    SHOT_TYPE_A,
    SHOT_TYPE_B,
};

enum BulletType
{
    BULLET_TYPE_0,
    BULLET_TYPE_1,
    BULLET_TYPE_2,
    BULLET_TYPE_LASER
};

enum PlayerState
{
    PLAYER_STATE_ALIVE,
    PLAYER_STATE_SPAWNING,
    PLAYER_STATE_DEAD,
    PLAYER_STATE_INVULNERABLE,
};

enum OrbState
{
    ORB_HIDDEN,
    ORB_UNFOCUSED,
    ORB_FOCUSING,
    ORB_FOCUSED,
    ORB_UNFOCUSING,
};

enum BulletState
{
    BULLET_STATE_UNUSED,
    BULLET_STATE_FIRED,
    BULLET_STATE_COLLIDED,
};
struct PlayerRect
{
    f32 posX;
    f32 posY;
    f32 sizeX;
    f32 sizeY;
};
ZUN_ASSERT_SIZE(PlayerRect, 0x10);

struct PlayerBullet
{
    AnmVm sprite;
    ZunVec3 position;
    ZunVec3 size;
    ZunVec2 velocity;
    f32 sidewaysMotion;
    ZunVec3 unk_134;
    ZunTimer unk_140;
    i16 damage;
    i16 bulletState;
    i16 bulletType;
    i16 unk_152;
    i16 spawnPositionIdx;

    void MoveHorizontal(f32 *position)
    {
        *position += this->velocity.x * g_Supervisor.effectiveFramerateMultiplier;
        this->sprite.pos.x = *position;
    }

    void MoveVertical(f32 *position)
    {
        *position += this->velocity.y * g_Supervisor.effectiveFramerateMultiplier;
        this->sprite.pos.y = *position;
    }
};
ZUN_ASSERT_SIZE(PlayerBullet, 0x158);

struct PlayerBombInfo
{
    u32 isInUse;
    i32 duration;
    ZunTimer timer;
    void (*calc)(Player *p);
    void (*draw)(Player *p);
    i32 reimuABombProjectilesState[8];
    f32 reimuABombProjectilesRelated[8];
    ZunVec3 bombRegionPositions[8];
    ZunVec3 bombRegionVelocities[8];
    AnmVm sprites[8][4];
};
ZUN_ASSERT_SIZE(PlayerBombInfo, 0x231c);

typedef i32 FireBulletResult;
#define FBR_STOP_SPAWNING (-2)
#define FBR_SPAWN_MORE (-1)

typedef FireBulletResult (*FireBulletCallback)(Player *, PlayerBullet *, u32, u32);
struct CharacterData
{
    f32 orthogonalMovementSpeed;
    f32 orthogonalMovementSpeedFocus;
    f32 diagonalMovementSpeed;
    f32 diagonalMovementSpeedFocus;
    FireBulletCallback fireBulletCallback;
    FireBulletCallback fireBulletFocusCallback;
};
ZUN_ASSERT_SIZE(CharacterData, 0x18);

struct CharacterPowerBulletData
{
    i16 waitBetweenBullets;
    i16 bulletFrame;
    ZunVec2 motion;
    ZunVec2 size;
    f32 direction;
    f32 velocity;
    u16 unk_1c;
    u8 spawnPositionIdx;
    u8 bulletType;
    i16 anmFileIdx;
    i16 bulletSoundIdx;
};
ZUN_ASSERT_SIZE(CharacterPowerBulletData, 0x24);

struct CharacterPowerData
{
    i32 numBullets;
    i32 power;
    CharacterPowerBulletData *bullets;
};
ZUN_ASSERT_SIZE(CharacterPowerData, 0xc);

struct Player
{
    Player();

    static bool RegisterChain(u8 unk);
    static void CutChain();
    static ChainCallbackResult OnUpdate(Player *p);
    static ChainCallbackResult OnDrawHighPrio(Player *p);
    static ChainCallbackResult OnDrawLowPrio(Player *p);
    static bool AddedCallback(Player *p);
    static bool DeletedCallback(Player *p);

    static FireBulletResult FireSingleBullet(Player *, PlayerBullet *bullet, i32 bullet_idx, i32 framesSinceLastBullet,
                                             CharacterPowerData *powerData);

    static FireBulletResult FireBulletReimuA(Player *, PlayerBullet *, u32, u32);
    static FireBulletResult FireBulletReimuB(Player *, PlayerBullet *, u32, u32);
    static FireBulletResult FireBulletMarisaA(Player *, PlayerBullet *, u32, u32);
    static FireBulletResult FireBulletMarisaB(Player *, PlayerBullet *, u32, u32);

    static void StartFireBulletTimer(Player *);
    bool HandlePlayerInputs();
    static void UpdatePlayerBullets(Player *);
    static bool UpdateFireBulletsTimer(Player *);

    static void SpawnBullets(Player *, u32 timer);
    static void DrawBullets(Player *p);
    static void DrawBulletExplosions(Player *p);

    f32 AngleFromPlayer(ZunVec3 *pos);
    f32 AngleToPlayer(ZunVec3 *pos);
    i32 CheckGraze(ZunVec3 *center, ZunVec3 *size);
    i32 CalcKillBoxCollision(ZunVec3 *bulletCenter, ZunVec3 *bulletSize);
    i32 CalcLaserHitbox(ZunVec3 *laserCenter, ZunVec3 *laserSize, ZunVec3 *rotation, f32 angle, i32 canGraze);
    i32 CalcDamageToEnemy(ZunVec3 *enemyPos, ZunVec3 *enemySize, bool *hitWithLazerDuringBomb);
    i32 CalcItemBoxCollision(ZunVec3 *center, ZunVec3 *size);
    void ScoreGraze(ZunVec3 *center);
    void Die();

    AnmVm playerSprite;
    AnmVm orbsSprite[3];
    ZunVec3 positionCenter;
    ZunVec3 unk_44c;
    ZunVec3 hitboxTopLeft;
    ZunVec3 hitboxBottomRight;
    ZunVec3 grabItemTopLeft;
    ZunVec3 grabItemBottomRight;
    ZunVec3 hitboxSize;
    ZunVec3 grabItemSize;
    ZunVec3 orbsPosition[2];
    ZunVec3 bombRegionPositions[32];
    ZunVec3 bombRegionSizes[32];
    i32 bombRegionDamages[32];
    i32 unk_838[32];
    PlayerRect bombProjectiles[16];
    ZunTimer laserTimer[2];
    f32 horizontalMovementSpeedMultiplierDuringBomb;
    f32 verticalMovementSpeedMultiplierDuringBomb;
    i32 respawnTimer;
    i32 bulletGracePeriod;
    i8 playerState;
    u8 unk_9e1;
    i8 orbState;
    i8 isFocus;
    u8 unk_9e4;
    ZunTimer focusMovementTimer;
    CharacterData characterData;
    PlayerDirection playerDirection;
    f32 previousHorizontalSpeed;
    f32 previousVerticalSpeed;
    i16 previousFrameInput;
    ZunVec3 positionOfLastEnemyHit;
    PlayerBullet bullets[80];
    ZunTimer fireBulletTimer;
    ZunTimer invulnerabilityTimer;
    FireBulletCallback fireBulletCallback;
    FireBulletCallback fireBulletFocusCallback;
    PlayerBombInfo bombInfo;
    ChainElem *chainCalc;
    ChainElem *chainDraw1;
    ChainElem *chainDraw2;

    void inline SetToTopLeftPos(AnmVm *sprite)
    {

        f32 *x = &sprite->pos.x;
        *x += g_GameManager.arcadeRegionTopLeftPos.x;
        f32 *y = &sprite->pos.y;
        *y += g_GameManager.arcadeRegionTopLeftPos.y;
        sprite->pos.z = 0.0;
    };
};
ZUN_ASSERT_SIZE(Player, 0x98f0);

extern Player g_Player;
}; // namespace th06

#pragma once
#include <d3dx8math.h>
#include <math.h>

#include "AnmVm.hpp"
#include "Chain.hpp"
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"

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
    REIMU,
    MARISA,
};

enum ShotType
{
    SHOT_TYPE_A,
    SHOT_TYPE_B,
};

enum PlayerBulletType
{
    PLAYER_BULLET_TYPE_0,
    PLAYER_BULLET_TYPE_1,
    PLAYER_BULLET_TYPE_2,
    PLAYER_LASER
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

enum PlayerBulletState
{
    PLAYER_BULLET_STATE_INACTIVE,
    PLAYER_BULLET_STATE_FIRED,
    PLAYER_BULLET_STATE_COLLIDED,
};

enum CollisionResult
{
    COLLISION_NONE,
    COLLISION_HIT,
    COLLISION_GRAZE
};
enum GrazeResult
{
    GRAZE_FALSE,
    GRAZE_TRUE,
    CANCELED_BY_BOMB
};

struct PlayerRect
{
    ZunVec2 pos;
    ZunVec2 size;
};
ZUN_ASSERT_TYPE(PlayerRect, 0x10, 4);

struct PlayerBullet
{
    AnmVm sprite;
    D3DXVECTOR3 position;
    D3DXVECTOR3 size;
    ZunVec2 velocity;
    ZunVec2 motion;
    f32 speed;
    f32 angle;
    ZunTimer lifetime;
    i16 damage;
    i16 bulletState;
    i16 bulletType;
    i16 bulletFrame;
    i16 spawnPositionIdx;
    alignment_padding(0x2);
};
ZUN_ASSERT_TYPE(PlayerBullet, 0x158, 4);

struct PlayerBombInfo
{
    ZunBool isInUse;
    i32 duration;
    ZunTimer timer;
    void (*calc)(Player *p);
    void (*draw)(Player *p);
    i32 reimuABombProjectilesState[8];
    f32 reimuABombProjectilesRelated[8];
    D3DXVECTOR3 bombRegionPositions[8];
    D3DXVECTOR3 bombRegionVelocities[8];
    AnmVm sprites[8][4];
};
ZUN_ASSERT_TYPE(PlayerBombInfo, 0x231c, 4);

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
ZUN_ASSERT_TYPE(CharacterData, 0x18, 4);

struct CharacterPowerBulletData
{
    i16 waitBetweenBullets;
    i16 bulletFrame;
    ZunVec2 motion;
    ZunVec2 size;
    f32 direction;
    f32 velocity;
    u16 damage;
    u8 spawnPositionIdx;
    u8 bulletType;
    i16 anmFileIdx;
    i16 bulletSoundIdx;
};
ZUN_ASSERT_TYPE(CharacterPowerBulletData, 0x24, 4);

struct CharacterPowerData
{
    i32 numBullets;
    i32 power;
    CharacterPowerBulletData *bullets;
};
ZUN_ASSERT_TYPE(CharacterPowerData, 0xc, 4);

#define MAX_PLAYER_BULLETS 80
#define PLAYER_BOMB_REGION_COUNT 32
#define PLAYER_BOMB_PROJECTILE_COUNT 16

struct Player
{
    ZunResult HandlePlayerInputs();

    f32 AngleFromPlayer(D3DXVECTOR3 *pos);
    f32 AngleToPlayer(D3DXVECTOR3 *pos);
    i32 CheckGraze(D3DXVECTOR3 *center, D3DXVECTOR3 *size);
    i32 CheckCollision(D3DXVECTOR3 *bulletCenter, D3DXVECTOR3 *bulletSize);
    i32 CalcLaserHitbox(D3DXVECTOR3 *laserCenter, D3DXVECTOR3 *laserSize, D3DXVECTOR3 *rotation, f32 angle,
                        ZunBool canGraze);
    i32 CalcDamageToEnemy(D3DXVECTOR3 *enemyPos, D3DXVECTOR3 *enemySize, ZunBool *hitByBomb);
    ZunBool CalcItemBoxCollision(D3DXVECTOR3 *center, D3DXVECTOR3 *size);
    void ScoreGraze(D3DXVECTOR3 *center);
    void Die();

    AnmVm playerSprite;
    AnmVm orbsSprite[3]; // why is this 3?
    D3DXVECTOR3 positionCenter;
    unused_field(D3DXVECTOR3);
    D3DXVECTOR3 hitboxTopLeft;
    D3DXVECTOR3 hitboxBottomRight;
    D3DXVECTOR3 grabItemTopLeft;
    D3DXVECTOR3 grabItemBottomRight;
    D3DXVECTOR3 hitboxSize;
    D3DXVECTOR3 grabItemSize;
    D3DXVECTOR3 orbsPosition[2];
    D3DXVECTOR3 bombRegionPositions[PLAYER_BOMB_REGION_COUNT];
    D3DXVECTOR3 bombRegionSizes[PLAYER_BOMB_REGION_COUNT];
    i32 bombRegionDamages[PLAYER_BOMB_REGION_COUNT];
    i32 bombRegionTotalDamages[PLAYER_BOMB_REGION_COUNT];
    PlayerRect bombProjectiles[PLAYER_BOMB_PROJECTILE_COUNT];
    ZunTimer laserTimer[2];
    ZunVec2 speedMultiplierDuringBomb;
    i32 respawnTimer;
    i32 bulletGracePeriod;
    i8 playerState;
    u8 unk_9e1;
    i8 orbState;
    i8 isFocus;
    u8 particleTimer;
    alignment_padding(0x3);
    ZunTimer focusMovementTimer;
    CharacterData characterData;
    PlayerDirection playerDirection;
    ZunVec2 previousSpeed;
    i16 previousFrameInput;
    alignment_padding(0x2);
    D3DXVECTOR3 positionOfLastEnemyHit;
    PlayerBullet bullets[MAX_PLAYER_BULLETS];
    ZunTimer fireBulletTimer;
    ZunTimer invulnerabilityTimer;
    FireBulletCallback fireBulletCallback;
    FireBulletCallback fireBulletFocusCallback;
    PlayerBombInfo bombInfo;
    ChainElem *chainCalc;
    ChainElem *chainDraw1;
    ChainElem *chainDraw2;
};
ZUN_ASSERT_TYPE(Player, 0x98f0, 4);

ZunResult Player_RegisterChain(u8 unk);
void Player_CutChain();

extern Player g_Player;
} // namespace th06

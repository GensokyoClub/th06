#pragma once
#include <d3dx8math.h>
#include <math.h>

#include "AnmVm.hpp"
#include "Chain.hpp"

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
    PLAYER_BULLET_DEFAULT,
    PLAYER_BULLET_HOMING,
    PLAYER_BULLET_ACCELERATE,
    PLAYER_LASER
};

enum PlayerState
{
    PLAYER_STATE_ALIVE,
    PLAYER_STATE_SPAWNING,
    PLAYER_STATE_DEAD,
    PLAYER_STATE_INVULNERABLE,
};

enum OptionState
{
    OPTION_HIDDEN,
    OPTION_UNFOCUSED,
    OPTION_FOCUSING,
    OPTION_FOCUSED,
    OPTION_UNFOCUSING,
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
    Float2 pos;
    Float2 size;
};
ZUN_ASSERT_TYPE(PlayerRect, 0x10, 4);

struct PlayerBullet
{
    AnmVm vm;
    D3DXVECTOR3 position;
    D3DXVECTOR3 size;
    Float2 velocity;
    Float2 offset;
    f32 speed;
    f32 angle;
    ZunTimer lifetime;
    i16 damage;
    i16 bulletState;
    i16 bulletType;
    i16 delayTimerIdx;
    i16 spawnPositionIdx;
    alignment_padding(0x2);
};
ZUN_ASSERT_TYPE(PlayerBullet, 0x158, 4);

#define BULLET_OPTION_INDEX(bullet) ((bullet)->spawnPositionIdx - 1)

typedef void (*BombCallback)(Player *);

#define MAX_BOMB_PROJECTILES 8
#define VMS_PER_BOMB_PROJECTILE 4

// These don't all actually have "projectiles"
// but the naming convention still makes sense
// for how the macros are used.
#define REIMU_A_BOMB_PROJECTILES 8
#define REIMU_B_BOMB_PROJECTILES 4
#define MARISA_A_BOMB_PROJECTILES 8
#define MARISA_B_BOMB_PROJECTILES 4

STATIC_ASSERT(REIMU_A_BOMB_PROJECTILES <= MAX_BOMB_PROJECTILES);
STATIC_ASSERT(REIMU_B_BOMB_PROJECTILES <= MAX_BOMB_PROJECTILES);
STATIC_ASSERT(MARISA_A_BOMB_PROJECTILES <= MAX_BOMB_PROJECTILES);
STATIC_ASSERT(MARISA_B_BOMB_PROJECTILES <= MAX_BOMB_PROJECTILES);

struct BombInfo
{
    BOOL isActive;
    i32 duration;
    ZunTimer timer;
    BombCallback calc;
    BombCallback draw;
    i32 projectileState[MAX_BOMB_PROJECTILES];
    f32 projectileCustom[MAX_BOMB_PROJECTILES];
    D3DXVECTOR3 projectilePosition[MAX_BOMB_PROJECTILES];
    D3DXVECTOR3 projectileVelocity[MAX_BOMB_PROJECTILES];
    AnmVm vms[MAX_BOMB_PROJECTILES * VMS_PER_BOMB_PROJECTILE];
};
ZUN_ASSERT_TYPE(BombInfo, 0x231c, 4);

typedef i32 FireBulletResult;
#define FBR_STOP_SPAWNING (-2)
#define FBR_SPAWN_MORE (-1)

typedef FireBulletResult (*FireBulletCallback)(Player *, PlayerBullet *, i32, i32);

struct CharacterData
{
    f32 linearMovementSpeed;
    f32 linearMovementSpeedFocus;
    f32 diagonalMovementSpeed;
    f32 diagonalMovementSpeedFocus;
    FireBulletCallback fireBulletCallback;
    FireBulletCallback fireBulletFocusCallback;
};
ZUN_ASSERT_TYPE(CharacterData, 0x18, 4);

struct ShtEntry
{
    i16 fireRate;
    i16 delay;
    Float2 offset;
    Float2 size;
    f32 angle;
    f32 speed;
    u16 damage;
    u8 spawnPositionIdx;
    u8 bulletType;
    i16 anmScript;
    i16 shootSfx;
};
ZUN_ASSERT_TYPE(ShtEntry, 0x24, 4);

#define SPAWN_FROM_PLAYER 0
#define SPAWN_FROM_OPTION(idx) ((idx) + 1)

struct PowerData
{
    i32 numEntries;
    i32 nextPowerThreshold;
    ShtEntry *entries;
};
ZUN_ASSERT_TYPE(PowerData, 0xc, 4);

#define MAX_PLAYER_BULLETS 80
#define MAX_DAMAGE_REGION_COUNT 32
#define MAX_CANCEL_REGION_COUNT 16
#define PLAYER_SHOT_TIMER_COUNT 2
#define MAX_PLAYER_OPTION_COUNT 2

enum PlayerBulletSpawnIndex
{
    BULLET_SPAWN_PLAYER = 0,
    BULLET_SPAWN_OPTIONS = 1,
};

#define SHOT_TIMER_MAX 30

struct Player
{
    ZunResult HandlePlayerInputs();

    f32 AngleFromPlayer(D3DXVECTOR3 *pos);
    f32 AngleToPlayer(D3DXVECTOR3 *pos);
    i32 CheckGraze(D3DXVECTOR3 *position, D3DXVECTOR3 *size);
    i32 CheckCollision(D3DXVECTOR3 *position, D3DXVECTOR3 *size);
    i32 CalcLaserHitbox(D3DXVECTOR3 *laserCenter, D3DXVECTOR3 *laserSize, D3DXVECTOR3 *laserPosition, f32 angle,
                        BOOL canGraze);
    i32 CalcDamageToEnemy(D3DXVECTOR3 *enemyPos, D3DXVECTOR3 *enemySize, BOOL *hitByBomb);
    BOOL CalcItemBoxCollision(D3DXVECTOR3 *itemPosition, D3DXVECTOR3 *itemSize);
    void ScoreGraze(D3DXVECTOR3 *grazedPosition);
    void Die();

    AnmVm vm;
    AnmVm optionVms[MAX_PLAYER_OPTION_COUNT + 1]; // why is this +1?
    D3DXVECTOR3 position;
    unused_field(D3DXVECTOR3);
    D3DXVECTOR3 hitboxTopLeft;
    D3DXVECTOR3 hitboxBottomRight;
    D3DXVECTOR3 grabItemTopLeft;
    D3DXVECTOR3 grabItemBottomRight;
    D3DXVECTOR3 hitboxSize;
    D3DXVECTOR3 grabItemSize;
    D3DXVECTOR3 optionPosition[MAX_PLAYER_OPTION_COUNT];
    D3DXVECTOR3 damageRegionPosition[MAX_DAMAGE_REGION_COUNT];
    D3DXVECTOR3 damageRegionSize[MAX_DAMAGE_REGION_COUNT];
    i32 damageRegionDamage[MAX_DAMAGE_REGION_COUNT];
    i32 damageRegionDamageDealt[MAX_DAMAGE_REGION_COUNT];
    PlayerRect cancelRegions[MAX_CANCEL_REGION_COUNT];
    ZunTimer shotTimer[PLAYER_SHOT_TIMER_COUNT]; // TODO: Better name so that fireBulletTimer -> shotTimer
    Float2 speedMultiplierDuringBomb;
    i32 respawnTimer;
    i32 bulletGracePeriod;
    i8 playerState;
    u8 unk_9e1;
    i8 orbState;
    i8 isFocused;
    u8 particleTimer;
    alignment_padding(0x3);
    ZunTimer focusMovementTimer;
    CharacterData characterData;
    PlayerDirection playerDirection;
    Float2 previousSpeed;
    i16 previousFrameInput;
    alignment_padding(0x2);
    D3DXVECTOR3 positionOfLastEnemyHit;
    PlayerBullet bullets[MAX_PLAYER_BULLETS];
    ZunTimer fireBulletTimer;
    ZunTimer stateTimer;
    FireBulletCallback fireBulletCallback;
    FireBulletCallback fireBulletFocusCallback;
    BombInfo bomb;
    ChainElem *chainCalc;
    ChainElem *chainDraw;
    ChainElem *chainDrawEffects;
};
ZUN_ASSERT_TYPE(Player, 0x98f0, 4);

ZunResult Player_RegisterChain(u8 unk);
void Player_CutChain();

extern Player g_Player;
} // namespace th06

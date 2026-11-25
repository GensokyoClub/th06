#pragma once
#include <Windows.h>
#include <string.h>

#include "ZunResult.hpp"
#include "decomp.hpp"
#include "zwave.hpp"

namespace th06
{
enum SoundIdx
{
    NO_SOUND = -1,
    SOUND_SHOOT,
    SOUND_SHOOT_2, // UNUSED: quieter variant sound
    SOUND_ENEMY_DIE,
    SOUND_ENEMY_DIE_2, // quieter variant sound
    SOUND_PICHUN,
    SOUND_CHARGE,
    SOUND_CHARGE_2,
    SOUND_BULLET,
    SOUND_BULLET_2,
    SOUND_BULLET_3,
    SOUND_MENU_SELECT,
    SOUND_MENU_CANCEL,
    SOUND_MOVE_CURSOR,
    SOUND_FANTASY_SEAL_START,
    SOUND_SPELLCARD,
    SOUND_FANTASY_SEAL_ATTACK,
    SOUND_LASER,
    SOUND_LASER_2,
    SOUND_BOSS_DIE,
    SOUND_MASTER_SPARK,
    SOUND_ENEMY_HIT,
    SOUND_ITEM_COLLECT,
    SOUND_BULLET_4,
    SOUND_BULLET_5,
    SOUND_BULLET_6,
    SOUND_SPARKLE,
    SOUND_SPARKLE_2,
    SOUND_SPARKLE_3,
    SOUND_EXTEND,
    SOUND_TIMER_WARNING,
#if BUILT_SINCE(100)
    SOUND_GRAZE,
    SOUND_POWERUP,
#endif
};

#define IS_VALID_SFX(sfx) ((sfx) >= 0)

struct SoundEffectData
{
    i32 fileIdx;
    i16 volume;
    i16 unk;
};
ZUN_ASSERT_TYPE(SoundEffectData, 0x8, 4);

// This is more than the actual number of sound effects
#define SOUND_EFFECT_COUNT 128

#define SFX_QUEUE_LENGTH 3

// 0.08 uses 64 notifications; 1.00 uses eight. 1.02h halved the later buffer and notification count.
#if BUILT_BEFORE(100)
#define BACKGROUND_MUSIC_STREAM_SECONDS 4
#define BACKGROUND_MUSIC_STREAM_NOTIFICATIONS 64
#elif BUILT_SINCE(102h)
#define BACKGROUND_MUSIC_STREAM_SECONDS 2
#define BACKGROUND_MUSIC_STREAM_NOTIFICATIONS 4
#else
#define BACKGROUND_MUSIC_STREAM_SECONDS 4
#define BACKGROUND_MUSIC_STREAM_NOTIFICATIONS 8
#endif

struct SoundPlayer
{
    SoundPlayer()
    {
        memset(this, 0, sizeof(SoundPlayer));
        for (i32 i = 0; i < SOUND_EFFECT_COUNT; i++)
        {
            this->unk408[i] = -1;
        }
    }

    ZunResult InitializeDSound(HWND window);
    ZunResult InitSoundBuffers();
    ZunResult Release();

    ZunResult LoadSound(i32 idx, const char *path);
#if BUILT_SINCE(100)
    void PlaySounds();
#endif
    void PlaySoundId(i32 sound, i32 unused = 0);
    ZunResult PlayBGM(BOOL isLooping);
    void StopBGM();
    void UpdateFades()
    {
        if (this->backgroundMusic != NULL)
        {
            this->backgroundMusic->UpdateFadeOut();
        }
    }

    void FadeOut(float seconds)
    {
        if (this->backgroundMusic != NULL)
        {
            this->backgroundMusic->StartFadeOut(seconds);
        }
    }

    ZunResult LoadWav(char *path);
    ZunResult LoadPos(const char *path);

    LPDIRECTSOUND dsoundHdl;
    unreferenced_fields(0x4); // possibly LPDIRECTSOUNDBUFFER primarySoundBuffer based on PBG code and later games
    LPDIRECTSOUNDBUFFER soundBuffers[SOUND_EFFECT_COUNT];
    LPDIRECTSOUNDBUFFER duplicateSoundBuffers[SOUND_EFFECT_COUNT];
    i32 unk408[SOUND_EFFECT_COUNT];
#if BUILT_SINCE(100)
    LPDIRECTSOUNDBUFFER initSoundBuffer;
#endif
    HWND gameWindow;
    CSoundManager *manager;
    DWORD backgroundMusicThreadId;
    HANDLE backgroundMusicThreadHandle;
    unreferenced_fields(0x4);
#if BUILT_SINCE(100)
    i32 soundBuffersToPlay[SFX_QUEUE_LENGTH];
#endif
    CStreamingSound *backgroundMusic;
    HANDLE backgroundMusicUpdateEvent;
    BOOL isLooping;
};
#if BUILT_SINCE(100)
ZUN_ASSERT_TYPE(SoundPlayer, 0x638, 4);
#else
ZUN_ASSERT_TYPE(SoundPlayer, 0x628, 4);
#endif

extern SoundPlayer g_SoundPlayer;
} // namespace th06

#pragma once

#include "inttypes.hpp"
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_audio.h>
#include "thirdparty/miniaudio.h"
#include <atomic>
#include <mutex>
#include <thread>

enum SoundIdx {
    NO_SOUND = -1,
    SOUND_SHOOT = 0,
    SOUND_1 = 1,
    SOUND_2 = 2,
    SOUND_3 = 3,
    SOUND_PICHUN = 4,
    SOUND_5 = 5,
    SOUND_BOMB_REIMARI = 6,
    SOUND_7 = 7,
    SOUND_8 = 8,
    SOUND_SHOOT_BOSS = 9,
    SOUND_SELECT = 10,
    SOUND_BACK = 11,
    SOUND_MOVE_MENU = 12,
    SOUND_BOMB_REIMU_A = 13,
    SOUND_BOMB = 14,
    SOUND_F = 15,
    SOUND_BOSS_LASER = 16,
    SOUND_BOSS_LASER_2 = 17,
    SOUND_12 = 18,
    SOUND_BOMB_MARISA_B = 19,
    SOUND_TOTAL_BOSS_DEATH = 20,
    SOUND_15 = 21,
    SOUND_16 = 22,
    SOUND_17 = 23,
    SOUND_18 = 24,
    SOUND_WTF_IS_THAT_LMAO = 25,
    SOUND_1A = 26,
    SOUND_1B = 27,
    SOUND_1UP = 28,
    SOUND_1D = 29,
    SOUND_GRAZE = 30,
    SOUND_POWERUP = 31,
};

struct SoundBufferIdxVolume {
    i32 bufferIdx;
    i16 volume;
};

struct SoundData {
    ma_sound sound;
    bool isLoaded;
    bool isPlaying;
    ma_uint64 pos;
};

struct LoopingDataSource {
    ma_data_source_base base;
    ma_decoder* decoder;
    ma_uint64 loopStartFrame;
    ma_uint64 loopEndFrame;
    ma_uint64 totalFrames;
    bool shouldLoop;
    ma_format format;
    ma_uint32 channels;
    ma_uint32 sampleRate;
    void* fileData;
    ma_uint8* pcmData;
    ma_uint64 pcmSize;
    ma_uint64 cursor;
};

struct MusicStream {
    LoopingDataSource loopingSource;
    ma_sound sound;
    bool isLoaded;
};

struct SoundPlayer {
    SoundPlayer();

    bool InitializeDSound();
    bool InitSoundBuffers();
    bool Release(void);

    bool LoadSound(i32 idx, const char *path, f32 volumeMultiplier);
    void PlaySounds();
    void PlaySoundByIdx(SoundIdx idx);
    bool PlayBGM(bool isLooping);
    void StopBGM();
    void FadeOut(f32 seconds);

    bool LoadWav(char *path);
    bool LoadPos(char *path);
    void StopLooping();

    SoundData soundBuffers[64];
    std::mutex soundBufMutex;
    ma_engine engine;
    ma_context context;
    std::atomic_bool terminateFlag;
    MusicStream backgroundMusic;
    bool isLooping;
    i32 soundBuffersToPlay[64];
    SDL_AudioDeviceID audioDev;
};

extern SoundBufferIdxVolume g_SoundBufferIdxVol[32];
extern const char *g_SFXList[26];
extern SoundPlayer g_SoundPlayer;

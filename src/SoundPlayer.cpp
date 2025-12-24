#define MINIAUDIO_IMPLEMENTATION
#include "SoundPlayer.hpp"

#include "FileSystem.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <vorbis/vorbisfile.h>
#include <array>
#include <cmath>
#include <cstring>
#include <new>
#include <vector>

static ma_result looping_data_source_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
    LoopingDataSource* pSource = (LoopingDataSource*)pDataSource;
    
    if (pFramesRead) {
        *pFramesRead = 0;
    }
    
    ma_uint64 totalFramesRead = 0;
    ma_uint64 framesToRead = frameCount;
    
    while (framesToRead > 0) {
        ma_uint64 currentPos;
        if (pSource->pcmData) {
            currentPos = pSource->totalFrames - (pSource->pcmSize - pSource->cursor) / ma_get_bytes_per_frame(pSource->format, pSource->channels);
        } else {
            ma_decoder_get_cursor_in_pcm_frames(pSource->decoder, &currentPos);
        }
        
        ma_uint64 framesUntilLoopEnd = pSource->loopEndFrame - currentPos;
        if (!pSource->shouldLoop || currentPos >= pSource->loopEndFrame) {
            framesUntilLoopEnd = pSource->totalFrames - currentPos;
        }
        
        ma_uint64 framesToReadThisTime = framesToRead;
        if (pSource->shouldLoop && currentPos < pSource->loopEndFrame) {
            framesToReadThisTime = framesToRead < framesUntilLoopEnd ? framesToRead : framesUntilLoopEnd;
        }
        
        ma_uint64 framesReadThisTime = 0;
        if (pSource->pcmData) {
            // Read from buffer
            ma_uint64 bytesToRead = framesToReadThisTime * ma_get_bytes_per_frame(pSource->format, pSource->channels);
            ma_uint64 bytesAvailable = pSource->pcmSize - pSource->cursor;
            ma_uint64 bytesRead = bytesToRead < bytesAvailable ? bytesToRead : bytesAvailable;
            memcpy(pFramesOut, pSource->pcmData + pSource->cursor, bytesRead);
            framesReadThisTime = bytesRead / ma_get_bytes_per_frame(pSource->format, pSource->channels);
            pSource->cursor += bytesRead;
    } else {
        ma_result result = ma_decoder_read_pcm_frames(pSource->decoder, 
            (ma_uint8*)pFramesOut + totalFramesRead * ma_get_bytes_per_frame(pSource->format, pSource->channels), 
            framesToReadThisTime, &framesReadThisTime);
            
        if (result != MA_SUCCESS) {
            return result;
        }
    }        
        totalFramesRead += framesReadThisTime;
        framesToRead -= framesReadThisTime;
        
        // do we need to loop?
        if (pSource->shouldLoop && framesReadThisTime > 0) {
            ma_uint64 newPos;
            if (pSource->pcmData) {
                newPos = pSource->cursor / ma_get_bytes_per_frame(pSource->format, pSource->channels);
            } else {
                ma_decoder_get_cursor_in_pcm_frames(pSource->decoder, &newPos);
            }
            if (newPos >= pSource->loopEndFrame) {
                if (pSource->pcmData) {
                    pSource->cursor = pSource->loopStartFrame * ma_get_bytes_per_frame(pSource->format, pSource->channels);
                } else {
                    ma_decoder_seek_to_pcm_frame(pSource->decoder, pSource->loopStartFrame);
                }
            }
        }
        
        if (framesReadThisTime == 0) {
            break; // probably EOF
        }
    }
    
    if (pFramesRead) {
        *pFramesRead = totalFramesRead;
    }
    
    return MA_SUCCESS;
}

static ma_result looping_data_source_seek(ma_data_source* pDataSource, ma_uint64 frameIndex) {
    LoopingDataSource* pSource = (LoopingDataSource*)pDataSource;
    if (pSource->pcmData) {
        pSource->cursor = frameIndex * ma_get_bytes_per_frame(pSource->format, pSource->channels);
        return MA_SUCCESS;
    } else {
        return ma_decoder_seek_to_pcm_frame(pSource->decoder, frameIndex);
    }
}

static ma_result looping_data_source_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap) {
    LoopingDataSource* pSource = (LoopingDataSource*)pDataSource;
    *pFormat = pSource->format;
    *pChannels = pSource->channels;
    *pSampleRate = pSource->sampleRate;
    return MA_SUCCESS;
}

static ma_result looping_data_source_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor) {
    LoopingDataSource* pSource = (LoopingDataSource*)pDataSource;
    if (pSource->pcmData) {
        *pCursor = pSource->cursor / ma_get_bytes_per_frame(pSource->format, pSource->channels);
    } else {
        return ma_decoder_get_cursor_in_pcm_frames(pSource->decoder, pCursor);
    }
    return MA_SUCCESS;
}

static ma_result looping_data_source_get_length(ma_data_source* pDataSource, ma_uint64* pLength) {
    LoopingDataSource* pSource = (LoopingDataSource*)pDataSource;
    *pLength = pSource->totalFrames;
    return MA_SUCCESS;
}

static ma_data_source_vtable looping_data_source_vtable = {
    looping_data_source_read,
    looping_data_source_seek,
    looping_data_source_get_data_format,
    looping_data_source_get_cursor,
    looping_data_source_get_length,
    nullptr,  // onSetLooping
    0 // flags
};

// This would all be a lot easier with SDL_mixer, but SDL_mixer doesn't permit
// any way of doing custom
//   loop points that would be accurate to the sample like EoSD needs. So
//   instead we get to read WAVs and mix everything by hand. Yay

#define BACKGROUND_MUSIC_WAV_NUM_CHANNELS 2
#define BACKGROUND_MUSIC_WAV_SAMPLE_RATE 44100
#define BACKGROUND_MUSIC_WAV_BITS_PER_SAMPLE 16
#define BACKGROUND_MUSIC_WAV_BLOCK_ALIGN        \
    (BACKGROUND_MUSIC_WAV_BITS_PER_SAMPLE / 8 * \
     BACKGROUND_MUSIC_WAV_NUM_CHANNELS)
#define BACKGROUND_MUSIC_WAV_BYTE_RATE \
    (BACKGROUND_MUSIC_WAV_BLOCK_ALIGN * BACKGROUND_MUSIC_WAV_SAMPLE_RATE)

// DirectSound deals with volume by subtracting a number measured in hundredths
// of decibels from the source sound.
//   The scale is from 0 (no volume modification) to -10,000 (subtraction of 100
//   decibels, and basically silent). 20 decibels affects wave amplitude by a
//   factor of 10

SoundBufferIdxVolume g_SoundBufferIdxVol[32] = {
    { 0, -1500 },
    { 0, -2000 },
    { 1, -1200 },
    { 1, -1400 },
    { 2, -1000 },
    { 3, -500 },
    { 4, -500 },
    { 5, -1700 },
    { 6, -1700 },
    { 7, -1700 },
    { 8, -1000 },
    { 9, -1000 },
    { 10, -1900 },
    { 11, -1200 },
    { 12, -900 },
    { 5, -1500 },
    { 13, -900 },
    { 14, -900 },
    { 15, -600 },
    { 16, -400 },
    { 17, -1100 },
    { 18, -900 },
    { 5, -1800 },
    { 6, -1800 },
    { 7, -1800 },
    { 19, -300 },
    { 20, -600 },
    { 21, -800 },
    { 22, -100 },
    { 23, -500 },
    { 24, -1000 },
    { 25, -1000 },
};
const char *g_SFXList[26] = {
    "data/wav/plst00.wav",
    "data/wav/enep00.wav",
    "data/wav/pldead00.wav",
    "data/wav/power0.wav",
    "data/wav/power1.wav",
    "data/wav/tan00.wav",
    "data/wav/tan01.wav",
    "data/wav/tan02.wav",
    "data/wav/ok00.wav",
    "data/wav/cancel00.wav",
    "data/wav/select00.wav",
    "data/wav/gun00.wav",
    "data/wav/cat00.wav",
    "data/wav/lazer00.wav",
    "data/wav/lazer01.wav",
    "data/wav/enep01.wav",
    "data/wav/nep00.wav",
    "data/wav/damage00.wav",
    "data/wav/item00.wav",
    "data/wav/kira00.wav",
    "data/wav/kira01.wav",
    "data/wav/kira02.wav",
    "data/wav/extend.wav",
    "data/wav/timeout.wav",
    "data/wav/graze.wav",
    "data/wav/powerup.wav",
};
SoundPlayer g_SoundPlayer;

SoundPlayer::SoundPlayer() {
    std::memset(this, 0, sizeof(SoundPlayer));
}

bool SoundPlayer::InitializeDSound() {
    ma_result result;
    ma_engine_config engineConfig;

    // Initialize miniaudio context
    result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) {
        goto fail;
    }

    // Initialize miniaudio engine
    engineConfig = ma_engine_config_init();
    engineConfig.pContext = &context;
    result = ma_engine_init(&engineConfig, &engine);
    if (result != MA_SUCCESS) {
        ma_context_uninit(&context);
        goto fail;
    }

    GameErrorContext::Log(&g_GameErrorContext, TH_DBG_SOUNDPLAYER_INIT_SUCCESS);
    return true;

fail:
    GameErrorContext::Log(&g_GameErrorContext, TH_ERR_SOUNDPLAYER_FAILED_TO_INITIALIZE_OBJECT);
    return false;
}

bool SoundPlayer::Release(void) {
    StopBGM();

    for (int i = 0; i < ARRAY_SIZE_SIGNED(this->soundBuffers); i++) {
        if (this->soundBuffers[i].isLoaded) {
            ma_sound_uninit(&this->soundBuffers[i].sound);
            this->soundBuffers[i].isLoaded = false;
        }
    }

    ma_engine_uninit(&this->engine);
    ma_context_uninit(&this->context);

    return true;
}

void SoundPlayer::StopBGM() {
    if (this->backgroundMusic.isLoaded) {
        ma_sound_stop(&this->backgroundMusic.sound);
        ma_sound_uninit(&this->backgroundMusic.sound);
        ma_data_source_uninit(&this->backgroundMusic.loopingSource.base);
        if (this->backgroundMusic.loopingSource.pcmData) {
            delete[] this->backgroundMusic.loopingSource.pcmData;
            this->backgroundMusic.loopingSource.pcmData = nullptr;
        }
        if (this->backgroundMusic.loopingSource.decoder) {
            ma_decoder_uninit(this->backgroundMusic.loopingSource.decoder);
            delete this->backgroundMusic.loopingSource.decoder;
            this->backgroundMusic.loopingSource.decoder = nullptr;
        }
        this->backgroundMusic.isLoaded = false;
        utils::DebugPrint2("stop BGM\n");
    }
}

void SoundPlayer::FadeOut(f32 seconds) {
    if (this->backgroundMusic.isLoaded) {
        ma_sound_set_fade_in_milliseconds(&this->backgroundMusic.sound, -1.0f, 0.0f, seconds * 1000.0f);
    }
}

bool SoundPlayer::LoadWav(char *path) {
    if (g_Supervisor.cfg.playSounds == 0) {
        return false;
    }

    this->StopBGM();

    utils::DebugPrint2("load BGM\n");

    std::string filePath(path);
    bool isOgg = filePath.find(".ogg") != std::string::npos;

    if (isOgg) {
        // Load OGG file using libvorbisfile
        FILE* file = fopen(path, "rb");
        if (!file) {
            utils::DebugPrint2("error : ogg file load error %s\n", path);
            return false;
        }

        OggVorbis_File vf;
        if (ov_open(file, &vf, NULL, 0) < 0) {
            fclose(file);
            utils::DebugPrint2("error : ogg file open error %s\n", path);
            return false;
        }

        vorbis_info* vi = ov_info(&vf, -1);
        if (!vi) {
            ov_clear(&vf);
            utils::DebugPrint2("error : ogg info error %s\n", path);
            return false;
        }

        if (vi->channels != 2 || vi->rate != 44100) {
            ov_clear(&vf);
            utils::DebugPrint2("error : format of ogg file not supported %s\n", path);
            return false;
        }

        // Decode entire file to PCM
        std::vector<char> pcmBuffer;
        char buffer[4096];
        int current_section;
        long bytes_read;
        while ((bytes_read = ov_read(&vf, buffer, sizeof(buffer), 0, 2, 1, &current_section)) > 0) {
            pcmBuffer.insert(pcmBuffer.end(), buffer, buffer + bytes_read);
        }

        if (bytes_read < 0) {
            ov_clear(&vf);
            utils::DebugPrint2("error : ogg decode error %s\n", path);
            return false;
        }

        ov_clear(&vf);

        // Set up LoopingDataSource with PCM data
        this->backgroundMusic.loopingSource.pcmData = new ma_uint8[pcmBuffer.size()];
        memcpy(this->backgroundMusic.loopingSource.pcmData, pcmBuffer.data(), pcmBuffer.size());
        this->backgroundMusic.loopingSource.pcmSize = pcmBuffer.size();
        this->backgroundMusic.loopingSource.format = ma_format_s16;
        this->backgroundMusic.loopingSource.channels = 2;
        this->backgroundMusic.loopingSource.sampleRate = 44100;
        this->backgroundMusic.loopingSource.totalFrames = pcmBuffer.size() / (2 * sizeof(int16_t));
        this->backgroundMusic.loopingSource.loopStartFrame = 0;
        this->backgroundMusic.loopingSource.loopEndFrame = this->backgroundMusic.loopingSource.totalFrames;
        this->backgroundMusic.loopingSource.shouldLoop = false;
        this->backgroundMusic.loopingSource.decoder = nullptr;
        this->backgroundMusic.loopingSource.fileData = nullptr;
        this->backgroundMusic.loopingSource.cursor = 0;

        // Initialize the data source base
        ma_data_source_config baseConfig = ma_data_source_config_init();
        baseConfig.vtable = &looping_data_source_vtable;
        ma_result result = ma_data_source_init(&baseConfig, &this->backgroundMusic.loopingSource.base);
        if (result != MA_SUCCESS) {
            delete[] this->backgroundMusic.loopingSource.pcmData;
            utils::DebugPrint2("error : data source init error %s\n", path);
            return false;
        }

        // Initialize the sound
        ma_sound_config soundConfig = ma_sound_config_init();
        soundConfig.pDataSource = &this->backgroundMusic.loopingSource;
        soundConfig.pEndCallbackUserData = nullptr;
        result = ma_sound_init_ex(&this->engine, &soundConfig, &this->backgroundMusic.sound);
        if (result != MA_SUCCESS) {
            ma_data_source_uninit(&this->backgroundMusic.loopingSource.base);
            delete[] this->backgroundMusic.loopingSource.pcmData;
            utils::DebugPrint2("error : sound init error %s\n", path);
            return false;
        }

        this->backgroundMusic.isLoaded = true;
    } else {
        // Load WAV file using miniaudio decoder
        ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_s16, 2, 44100);
        this->backgroundMusic.loopingSource.decoder = new ma_decoder();
        ma_result result = ma_decoder_init_file(path, &decoderConfig, this->backgroundMusic.loopingSource.decoder);
        if (result != MA_SUCCESS) {
            delete this->backgroundMusic.loopingSource.decoder;
            utils::DebugPrint2("error : wav decoder init error %s\n", path);
            return false;
        }

        // Get length
        ma_uint64 lengthInFrames;
        result = ma_decoder_get_length_in_pcm_frames(this->backgroundMusic.loopingSource.decoder, &lengthInFrames);
        if (result != MA_SUCCESS) {
            ma_decoder_uninit(this->backgroundMusic.loopingSource.decoder);
            delete this->backgroundMusic.loopingSource.decoder;
            utils::DebugPrint2("error : wav get length error %s\n", path);
            return false;
        }

        this->backgroundMusic.loopingSource.totalFrames = lengthInFrames;
        this->backgroundMusic.loopingSource.loopStartFrame = 0;
        this->backgroundMusic.loopingSource.loopEndFrame = lengthInFrames;
        this->backgroundMusic.loopingSource.shouldLoop = false;
        this->backgroundMusic.loopingSource.format = ma_format_s16;
        this->backgroundMusic.loopingSource.channels = 2;
        this->backgroundMusic.loopingSource.sampleRate = 44100;
        this->backgroundMusic.loopingSource.pcmData = nullptr;
        this->backgroundMusic.loopingSource.pcmSize = 0;
        this->backgroundMusic.loopingSource.cursor = 0;

        // Initialize the data source base
        ma_data_source_config baseConfig = ma_data_source_config_init();
        baseConfig.vtable = &looping_data_source_vtable;
        result = ma_data_source_init(&baseConfig, &this->backgroundMusic.loopingSource.base);
        if (result != MA_SUCCESS) {
            ma_decoder_uninit(this->backgroundMusic.loopingSource.decoder);
            delete this->backgroundMusic.loopingSource.decoder;
            this->backgroundMusic.loopingSource.decoder = nullptr;
            utils::DebugPrint2("error : data source init error %s\n", path);
            return false;
        }

        // Initialize the sound
        ma_sound_config soundConfig = ma_sound_config_init();
        soundConfig.pDataSource = &this->backgroundMusic.loopingSource;
        soundConfig.pEndCallbackUserData = nullptr;
        result = ma_sound_init_ex(&this->engine, &soundConfig, &this->backgroundMusic.sound);
        if (result != MA_SUCCESS) {
            ma_data_source_uninit(&this->backgroundMusic.loopingSource.base);
            ma_decoder_uninit(this->backgroundMusic.loopingSource.decoder);
            delete this->backgroundMusic.loopingSource.decoder;
            utils::DebugPrint2("error : sound init error %s\n", path);
            return false;
        }

        this->backgroundMusic.isLoaded = true;
    }

    return true;
}

bool SoundPlayer::LoadPos(char *path) {
    u8 *fileData;

    if (g_Supervisor.cfg.playSounds == 0 || !this->backgroundMusic.isLoaded) {
        return false;
    }

    fileData = FileSystem::OpenPath(path);

    if (fileData == NULL) {
        return false;
    }

    u32 loopStartSamples = SDL_SwapLE32(*((u32 *)fileData));
    u32 loopEndSamples = SDL_SwapLE32(*(u32 *)(fileData + 4));

    free(fileData);

    // Loop points are in samples, convert to frames (stereo: 2 samples per frame)
    this->backgroundMusic.loopingSource.loopStartFrame = loopStartSamples;
    this->backgroundMusic.loopingSource.loopEndFrame = loopEndSamples;

    if (this->backgroundMusic.loopingSource.loopStartFrame >= this->backgroundMusic.loopingSource.loopEndFrame ||
        this->backgroundMusic.loopingSource.loopEndFrame > this->backgroundMusic.loopingSource.totalFrames) {
        this->backgroundMusic.loopingSource.loopStartFrame = 0;
        this->backgroundMusic.loopingSource.loopEndFrame = this->backgroundMusic.loopingSource.totalFrames;
        return false;
    }

    this->backgroundMusic.loopingSource.shouldLoop = true;
    return true;
}

bool SoundPlayer::InitSoundBuffers() {
    std::fill_n(this->soundBuffersToPlay, ARRAY_SIZE(this->soundBuffersToPlay),
                -1);

    for (int idx = 0; idx < ARRAY_SIZE_SIGNED(g_SoundBufferIdxVol); idx++) {
        if (!this->LoadSound(
                idx, g_SFXList[g_SoundBufferIdxVol[idx].bufferIdx],
                1.0f / std::powf(10.0f, (float)g_SoundBufferIdxVol[idx].volume /
                                            -2000))) {
            GameErrorContext::Log(&g_GameErrorContext,
                                  TH_ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE,
                                  g_SFXList[idx]);
            return false;
        }

        this->soundBuffers[idx].isPlaying = false;
        this->soundBuffers[idx].pos = 0;
    }

    return true;
}

bool SoundPlayer::LoadSound(i32 idx, const char *path, f32 volumeMultiplier) {
    soundBufMutex.lock();

    if (this->soundBuffers[idx].isLoaded) {
        ma_sound_uninit(&this->soundBuffers[idx].sound);
        this->soundBuffers[idx].isLoaded = false;
    }

    // Load sound file using miniaudio decoder
    ma_result result = ma_sound_init_from_file(&this->engine, path, 0, NULL, NULL, &this->soundBuffers[idx].sound);
    if (result != MA_SUCCESS) {
        GameErrorContext::Log(&g_GameErrorContext, TH_ERR_SOUNDPLAYER_FAILED_TO_LOAD_SOUND_FILE, path);
        goto fail;
    }

    // Set volume
    ma_sound_set_volume(&this->soundBuffers[idx].sound, volumeMultiplier);

    this->soundBuffers[idx].isLoaded = true;
    this->soundBuffers[idx].isPlaying = false;
    this->soundBuffers[idx].pos = 0;

    soundBufMutex.unlock();
    return true;

fail:
    soundBufMutex.unlock();
    return false;
}

bool SoundPlayer::PlayBGM(bool isLooping) {
    utils::DebugPrint2("play BGM\n");

    if (!this->backgroundMusic.isLoaded) {
        return false;
    }

    this->backgroundMusic.loopingSource.shouldLoop = isLooping;
    this->isLooping = isLooping;

    ma_sound_start(&this->backgroundMusic.sound);
    utils::DebugPrint2("comp\n");
    return true;
}

void SoundPlayer::PlaySounds() {
    i32 idx;
    i32 sndBufIdx;

    if (!g_Supervisor.cfg.playSounds) {
        return;
    }

    soundBufMutex.lock();

    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->soundBuffersToPlay); idx++) {
        if (this->soundBuffersToPlay[idx] < 0) {
            break;
        }

        sndBufIdx = this->soundBuffersToPlay[idx];
        this->soundBuffersToPlay[idx] = -1;

        if (!this->soundBuffers[sndBufIdx].isLoaded) {
            continue;
        }

        // Start playing the sound
        ma_sound_start(&this->soundBuffers[sndBufIdx].sound);
        this->soundBuffers[sndBufIdx].isPlaying = true;
    }

    // Update playing status
    for (int i = 0; i < 64; i++) {
        if (this->soundBuffers[i].isLoaded && this->soundBuffers[i].isPlaying) {
            ma_bool32 isPlaying = ma_sound_is_playing(&this->soundBuffers[i].sound);
            if (!isPlaying) {
                this->soundBuffers[i].isPlaying = false;
                ma_sound_seek_to_pcm_frame(&this->soundBuffers[i].sound, 0); // Reset to beginning
            }
        }
    }

    soundBufMutex.unlock();
}

void SoundPlayer::PlaySoundByIdx(SoundIdx idx) {
    u32 i;

    for (i = 0; i < ARRAY_SIZE(this->soundBuffersToPlay); i++) {
        if (this->soundBuffersToPlay[i] < 0) {
            break;
        }

        if (this->soundBuffersToPlay[i] == idx) {
            return;
        }
    }

    if (i >= 3) {
        return;
    }

    this->soundBuffersToPlay[i] = idx;
}



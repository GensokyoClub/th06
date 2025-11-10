#pragma once

#include "ZunResult.hpp"
#include "inttypes.hpp"

#include <SDL2/SDL_timer.h>

#ifdef _WIN32
#include "midi/MidiWin32.hpp"
#elif defined(LIBASOUND_MIDI_SUPPORT)
#include "midi/MidiAlsa.hpp"
#else
#include "midi/MidiDefault.hpp"
#endif

namespace th06
{
struct MidiTimer
{
    MidiTimer();
    ~MidiTimer();

    virtual void OnTimerElapsed() = 0;

    i32 StopTimer();
    void StartTimer(u32 delay, SDL_TimerCallback cb, void *data);

    //    static void CALLBACK DefaultTimerCallback(u32 uTimerID, u32 uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR
    //    dw2);
    static u32 SDLCALL DefaultTimerCallback(u32 interval, MidiTimer *timer);

    SDL_TimerID timerId;
    u32 lastTimerTicks;
};

enum MidiOpcode
{
    MIDI_OPCODE_CHANNEL_1 = 0x01,
    MIDI_OPCODE_CHANNEL_2 = 0x02,
    MIDI_OPCODE_CHANNEL_3 = 0x03,
    MIDI_OPCODE_CHANNEL_4 = 0x04,
    MIDI_OPCODE_CHANNEL_5 = 0x05,
    MIDI_OPCODE_CHANNEL_6 = 0x06,
    MIDI_OPCODE_CHANNEL_7 = 0x07,
    MIDI_OPCODE_CHANNEL_8 = 0x08,
    MIDI_OPCODE_CHANNEL_9 = 0x09,
    MIDI_OPCODE_CHANNEL_A = 0x0A,
    MIDI_OPCODE_CHANNEL_B = 0x0B,
    MIDI_OPCODE_CHANNEL_C = 0x0C,
    MIDI_OPCODE_CHANNEL_D = 0x0D,
    MIDI_OPCODE_CHANNEL_E = 0x0E,
    MIDI_OPCODE_CHANNEL_F = 0x0F,
    MIDI_OPCODE_NOTE_OFF = 0x80,
    MIDI_OPCODE_NOTE_ON = 0x90,
    MIDI_OPCODE_POLYPHONIC_AFTERTOUCH = 0xA0,
    MIDI_OPCODE_MODE_CHANGE = 0xB0,
    MIDI_OPCODE_PROGRAM_CHANGE = 0xC0,
    MIDI_OPCODE_CHANNEL_AFTERTOUCH = 0xD0,
    MIDI_OPCODE_PITCH_BEND_CHANGE = 0xE0,
    MIDI_OPCODE_SYSTEM_EXCLUSIVE = 0xF0,
    MIDI_OPCODE_MIDI_TIME_CODE_QTR_FRAME = 0xF1,
    MIDI_OPCODE_SONG_POSITION_POINTER = 0xF2,
    MIDI_OPCODE_SONG_SELECT = 0xF3,
    MIDI_OPCODE_RESERVED_F4 = 0xF4,
    MIDI_OPCODE_RESERVED_F5 = 0xF5,
    MIDI_OPCODE_TUNE_REQUEST = 0xF6,
    MIDI_OPCODE_END_OF_SYSEX = 0xF7,
    MIDI_OPCODE_TIMING_CLOCK = 0xF8,
    MIDI_OPCODE_RESERVED_F9 = 0xF9,
    MIDI_OPCODE_START = 0xFA,
    MIDI_OPCODE_CONTINUE = 0xFB,
    MIDI_OPCODE_STOP = 0xFC,
    MIDI_OPCODE_RESERVED_FD = 0xFD,
    MIDI_OPCODE_ACTIVE_SENSING = 0xFE,
    MIDI_OPCODE_SYSTEM_RESET = 0xFF,
};

struct MidiTrack
{
    bool trackPlaying;
    u32 nextMessageTimePos;
    u32 trackLength;
    u8 opcode;
    u8 *trackData;
    u8 *curTrackDataCursor;
    u8 *loopPointTarget;
    u32 loopPointTimePos;
};

struct MidiChannel
{
    u8 keyPressedFlags[16];
    u8 instrument;
    u8 instrumentBank;
    u8 pan;
    u8 effectOneDepth;
    u8 effectThreeDepth;
    u8 channelVolume;
};

struct MidiOutput : MidiTimer
{
    MidiOutput();
    ~MidiOutput();

    void OnTimerElapsed();

    ZunResult StopPlayback();
    void LoadTracks();
    void ClearTracks();
    bool ReadFileData(u32 idx, char *path);
    void ReleaseFileData(u32 idx);
    void ParseFile(u32 idx);
    void ProcessMsg(MidiTrack *track);

    bool ParseFile(i32 idx);
    bool LoadFile(char *midiPath);
    bool Play();

    u32 SetFadeOut(u32 ms);
    void FadeOutSetVolume(i32 volume);

    static u32 ReadVariableLength(u8 **curTrackDataCursor);

    u8 *midiFileData[32];
    i32 numTracks;
    u32 format;
    i32 divisions;
    i32 tempo;
    u64 elapsedMS;
    u64 tickBase;
    MidiTrack *tracks;
    MidiDevice midiOutDev;
    MidiChannel channels[16];
    f32 fadeOutVolumeMultiplier;
    u32 fadeOutLastSetVolume;
    bool fadeOutFlag;
    i32 fadeOutInterval;
    i32 fadeOutElapsedMS;
    u32 loopPointTempo;
    u64 loopPointMSCount;
    u64 loopPointBaseTicks;
};
}; // namespace th06

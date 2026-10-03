#pragma once
#include "ZunBool.hpp"
#include "ZunResult.hpp"
#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
struct MidiTimer
{
    MidiTimer();
    ~MidiTimer();

    virtual void OnTimerElapsed();

    i32 StopTimer();
    u32 StartTimer(u32 delay, LPTIMECALLBACK cb, DWORD_PTR data);

    static void CALLBACK DefaultTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

    u32 timerId;
    TIMECAPS timeCaps;
};
ZUN_ASSERT_TYPE(MidiTimer, 0x10, 4);

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
    u32 trackPlaying;
    i32 trackLengthOther;
    u32 trackLength;
    u8 opcode;
    alignment_padding(0x3);
    u8 *trackData;
    u8 *curTrackDataCursor;
    u8 *startTrackDataMaybe;
    i32 unk1c;
};
ZUN_ASSERT_TYPE(MidiTrack, 0x20, 4);

struct MidiDevice
{
    MidiDevice();
    ~MidiDevice();

    ZunResult Close();
    ZunBool OpenDevice(u32 uDeviceId);
    ZunBool SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte);
    ZunBool SendLongMsg(LPMIDIHDR pmh);

    HMIDIOUT handle;
    u32 deviceId;
};
ZUN_ASSERT_TYPE(MidiDevice, 0x8, 4);

struct MidiChannel
{
    u8 keyPressedFlags[16];
    u8 instrument;
    u8 instrumentBank;
    u8 pan;
    u8 effectOneDepth;
    u8 effectThreeDepth;
    u8 channelVolume;
    u8 modifiedVolume;
};
ZUN_ASSERT_TYPE(MidiChannel, 0x17, 1);

struct MidiOutput : MidiTimer
{
    MidiOutput();
    ~MidiOutput();

    virtual void OnTimerElapsed();

    ZunResult UnprepareHeader(LPMIDIHDR pmh);

    ZunResult StopPlayback();
    void LoadTracks();
    void ClearTracks();
    ZunResult ReadFileData(u32 idx, const char *path);
    void ReleaseFileData(u32 idx);
    void ParseFile(u32 idx);
    void ProcessMsg(MidiTrack *track);

    ZunResult ParseFile(i32 idx);
    ZunResult LoadFile(const char *midiPath);
    ZunResult Play();

    ZunResult LoadAndPlay(const char *midiPath)
    {
        this->StopPlayback();
        this->LoadFile(midiPath);
        return this->Play();
    }
    ZunResult ParseAndPlay(i32 idx)
    {
        this->StopPlayback();
        this->ParseFile(idx);
        return this->Play();
    }

    u32 SetFadeOut(u32 ms);
    void FadeOutSetVolume(i32 volume);

    static u16 Ntohs(u16 val);
    static u32 SkipVariableLength(u8 **curTrackDataCursor);

    static u32 Ntohl(u32 val)
    {
        u8 tmp[4];

        tmp[0] = ((u8 *)&val)[3];
        tmp[1] = ((u8 *)&val)[2];
        tmp[2] = ((u8 *)&val)[1];
        tmp[3] = ((u8 *)&val)[0];

        return *(const u32 *)(&tmp);
    }

    MIDIHDR *midiHeaders[32];
    i32 midiHeadersCursor;
    u8 *midiFileData[32];
    i32 numTracks;
    u32 format;
    i32 divisions;
    i32 tempo;
    unreferenced_fields(0x4);
    ULONGLONG volume;
    LONGLONG unk130;
    MidiTrack *tracks;
    MidiDevice midiOutDev;
    unreferenced_fields(0x10);
    MidiChannel channels[16];
    i8 unk2c4;
    alignment_padding(0x1);
    f32 fadeOutVolumeMultiplier;
    u32 fadeOutLastSetVolume;
    u32 unk2d0;
    u32 unk2d4;
    u32 unk2d8;
    u32 unk2dc;
    ZunBool fadeOutFlag;
    i32 fadeOutInterval;
    i32 fadeOutElapsedMS;
    i32 unk2ec;
    ULONGLONG unk2f0;
    ULONGLONG unk2f8;
};
ZUN_ASSERT_TYPE(MidiOutput, 0x300, 8);
} // namespace th06

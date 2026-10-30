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

    u32 timerId;
    TIMECAPS timeCaps;
};
ZUN_ASSERT_TYPE(MidiTimer, 0x10, 4);

void CALLBACK MidiTimer_DefaultTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

struct MidiTrack;

struct MidiDevice
{
    MidiDevice();
    ~MidiDevice();

    ZunResult Close();
    ZunBool OpenDevice(u32 uDeviceId);
    ZunBool SendShortMsg(DWORD msg);
    ZunBool SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte);
    void GetDeviceName(u32 uDeviceId, char *name);
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
    ZunResult PlayFile(i32 idx)
    {
        this->StopPlayback();
        this->ParseFile(idx);
        return this->Play();
    }

    u32 SetFadeOut(u32 ms);
    void FadeOutSetVolume(i32 volume);

    MIDIHDR *midiHeaders[32];
    i32 midiHeadersCursor;
    u8 *midiFileData[32];

    i32 numTracks;
    u32 format;
    i32 timebase;
    i32 tempo;
    alignment_padding(0x4);
    ULONGLONG playcount1;
    LONGLONG playcount2;
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
    i32 loopTempo;
    ULONGLONG loopPlaycount1;
    ULONGLONG loopPlaycount2;
};
ZUN_ASSERT_TYPE(MidiOutput, 0x300, 8);
} // namespace th06

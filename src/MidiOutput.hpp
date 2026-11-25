#pragma once

#include "ZunResult.hpp"
#include "decomp.hpp"
#include <Windows.h>

namespace th06
{
struct MidiTimer
{
  public:
    MidiTimer();
    ~MidiTimer();

    virtual void OnTimerElapsed();

    i32 StopTimer();
    u32 StartTimer(u32 delay, LPTIMECALLBACK cb, DWORD_PTR data);

  private:
    u32 timerId;
    TIMECAPS timeCaps;
};
ZUN_ASSERT_TYPE(MidiTimer, 0x10, 4);

void CALLBACK MidiTimer_DefaultTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

struct MidiTrack;

class MidiDevice
{
  public:
    MidiDevice();
    ~MidiDevice();

    ZunResult Close();
    BOOL OpenDevice(u32 uDeviceId);
    BOOL SendShortMsg(DWORD msg);
    BOOL SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte);
    void GetDeviceName(u32 uDeviceId, char *name);
    BOOL SendLongMsg(LPMIDIHDR pmh);

    HMIDIOUT handle;

  private:
    u32 deviceId;
};
ZUN_ASSERT_TYPE(MidiDevice, 0x8, 4);

struct MidiChannel
{
    u8 keysPressed[128 / CHAR_BIT];
    u8 program;
    u8 bankMSB;
    u8 pan;
    u8 reverbDepth;
    u8 chorusDepth;
    u8 rawVolume;
    u8 mixedVolume;
};
ZUN_ASSERT_TYPE(MidiChannel, 0x17, 1);

#define MAX_MIDI_FILES 32
#define MIDI_CHANNEL_COUNT 16

class MidiOutput : MidiTimer
{
  public:
    MidiOutput();
    ~MidiOutput();

    virtual void OnTimerElapsed();

    ZunResult UnprepareHeader(LPMIDIHDR pmh);

    ZunResult StopPlayback();
    void PlayTracks();
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
    void SetVolume(i32 volume);

  private:
    MIDIHDR *midiHeaders[MAX_MIDI_FILES];
    i32 midiHeadersCursor;
    u8 *midiFileData[MAX_MIDI_FILES];
    i32 numTracks;
    u32 format;
    i32 divisions;
    i32 tempo;
    alignment_padding(0x4);
    u64 msSinceLastTempoEvent;
    u64 msOfLastTempoEvent;
    MidiTrack *tracks;
    MidiDevice midiOutDev;
    unreferenced_fields(0x10);
    MidiChannel channels[MIDI_CHANNEL_COUNT];
    i8 transpose;
    alignment_padding(0x1);
    f32 volumeMultiplier;
    u32 prevFadeOutVolume;
    u32 unk2d0;
    BOOL isVolumeLocked;
    u32 unk2d8;
    u32 unk2dc;
    BOOL isFadingOut;
    i32 fadeOutDuration;
    i32 fadeOutTimer;
    i32 loopTempo;
    u64 loopMsSinceLastTempoEvent;
    u64 loopMsOfLastTempoEvent;
};
ZUN_ASSERT_TYPE(MidiOutput, 0x300, 8);
} // namespace th06

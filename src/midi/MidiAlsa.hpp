#pragma once

#include "ZunResult.hpp"
#include "inttypes.hpp"
#include <alsa/asoundlib.h>

struct MidiDevice
{
  public:
    MidiDevice();
    ~MidiDevice();

    ZunResult Close();
    bool OpenDevice(u32 uDeviceId);
    bool SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte);
    bool SendLongMsg(u8 *buf, u32 len);

  private:
    void Reset();
    bool GetDestPort();

    snd_seq_t *sequencer;

    snd_midi_event_t *encoder;
    u32 encoderBufferSize;

    int sourcePort;

    int destClient;
    int destPort;

    bool hasConnection;
};

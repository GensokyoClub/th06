#pragma once

#include "ZunResult.hpp"
#include "inttypes.hpp"

#include <AudioToolbox/AudioToolbox.h>

// MIDI output through Apple's built-in DLS software synthesizer.

struct MidiDevice
{
  public:
    MidiDevice();
    ~MidiDevice();

    ZunResult Close();
    bool OpenDevice(u32 uDeviceId);
    bool SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte);
    bool SendLongMsg(const u8 *buf, u32 len);

  private:
    AudioUnit synthUnit;
    AudioUnit outputUnit;
};

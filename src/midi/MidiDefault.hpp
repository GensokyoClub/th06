#pragma once

#include "ZunResult.hpp"
#include "inttypes.hpp"

// The MIDI interface used if a specific platform MIDI API is not supported
// Obviously can't do much, but something needs to be linked

namespace th06
{
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
    bool printedWarning;
};
}; // namespace th06

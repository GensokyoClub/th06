#pragma once

#include "inttypes.hpp"
#include <mmsystem.h>
#include <windows.h>

namespace th06
{
struct MidiDevice
{
  public:
    MidiDevice();
    ~MidiDevice();

    bool Close();
    bool OpenDevice(u32 uDeviceId);
    bool SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte);
    bool SendLongMsg(u8 *buf, u32 len);

  private:
    bool UnprepareHeader(LPMIDIHDR pmh);

    HMIDIOUT handle;
    u32 deviceId;

    // EoSD stores prepared MIDI headers but never does anything with them. Microsoft's documentation
    //   isn't very clear on what preparing / unpreparing a header actually does, but does note that
    //   sending MIDI messages can be asynchronous. Therefore I've left the MIDI header array in, just
    //   in case unpreparing a header immediately can cause a block while waiting for a flush or something.

    MIDIHDR *midiHeaders[32];
    u32 midiHeadersCursor;
};
}; // namespace th06

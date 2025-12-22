#include "MidiWin32.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <cstring>

MidiDevice::MidiDevice()
{
    this->handle = NULL;
    this->deviceId = 0;

    for (int i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        this->midiHeaders[i] = NULL;
    }

    this->midiHeadersCursor = 0;
}

MidiDevice::~MidiDevice()
{
    this->Close();
}

bool MidiDevice::OpenDevice(u32 uDeviceId)
{
    if (this->handle != 0)
    {
        if (this->deviceId != uDeviceId)
        {
            this->Close();
        }
        else
        {
            return false;
        }
    }

    this->deviceId = uDeviceId;

    // TODO: Write callback function. Windows EoSD used WndProc for this, but we obviously can't do that here
    return midiOutOpen(&this->handle, uDeviceId, (DWORD_PTR)NULL, (DWORD_PTR)NULL, CALLBACK_NULL) == MMSYSERR_NOERROR;
}

ZunResult MidiDevice::Close()
{
    if (this->handle == 0)
    {
        return ZUN_ERROR;
    }

    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        if (this->midiHeaders[this->midiHeadersCursor] != NULL)
        {
            this->UnprepareHeader(this->midiHeaders[this->midiHeadersCursor]);
        }
    }

    midiOutReset(this->handle);
    midiOutClose(this->handle);
    this->handle = 0;

    return ZUN_SUCCESS;
}

union MidiShortMsg {
    struct
    {
        u8 midiStatus;
        i8 firstByte;
        i8 secondByte;
        i8 unused;
    } msg;
    u32 dwMsg;
};

bool MidiDevice::SendLongMsg(u8 *buf, u32 len)
{
    if (this->handle == 0)
    {
        return true;
    }

    if (this->midiHeaders[this->midiHeadersCursor] != NULL)
    {
        this->UnprepareHeader(this->midiHeaders[this->midiHeadersCursor]);
    }

    MIDIHDR *midiHdr = this->midiHeaders[this->midiHeadersCursor] = (MIDIHDR *)std::malloc(sizeof(MIDIHDR));

    std::memset(midiHdr, 0, sizeof(*midiHdr));
    midiHdr->lpData = (LPSTR)std::malloc(len);
    std::memcpy(midiHdr->lpData, buf, len);
    midiHdr->dwFlags = 0;
    midiHdr->dwBufferLength = len;

    if (midiOutPrepareHeader(this->handle, midiHdr, sizeof(*midiHdr)) != MMSYSERR_NOERROR)
    {
        return false;
    }

    this->midiHeadersCursor++;
    this->midiHeadersCursor = this->midiHeadersCursor % ARRAY_SIZE(this->midiHeaders);

    return midiOutLongMsg(this->handle, midiHdr, sizeof(*midiHdr)) == MMSYSERR_NOERROR;
}

bool MidiDevice::SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte)
{
    MidiShortMsg pkt;

    if (this->handle == 0)
    {
        return true;
    }
    else
    {
        pkt.msg.midiStatus = midiStatus;
        pkt.msg.firstByte = firstByte;
        pkt.msg.secondByte = secondByte;
        return midiOutShortMsg(this->handle, pkt.dwMsg) == MMSYSERR_NOERROR;
    }
}

ZunResult MidiDevice::UnprepareHeader(LPMIDIHDR pmh)
{
    if (pmh == NULL)
    {
        utils::DebugPrint2("error :\n");
    }

    if (this->handle == 0)
    {
        utils::DebugPrint2("error :\n");
    }

    // The reason for this weird linear search here is that this is supposed to be able
    //   to run after Windows sends an MM_MOM_DONE message indicating that a long message
    //   was sent. To save ourselves from a possible double free we have to make sure that
    //   the header hasn't yet been freed.

    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        if (this->midiHeaders[i] == pmh)
        {
            this->midiHeaders[i] = NULL;
            goto success;
        }
    }

    return ZUN_ERROR;

success:
    MMRESULT res = midiOutUnprepareHeader(this->handle, pmh, sizeof(*pmh));
    if (res != MMSYSERR_NOERROR)
    {
        utils::DebugPrint2("error :\n");
    }

    std::free(pmh->lpData);
    std::free(pmh);

    return ZUN_SUCCESS;
}


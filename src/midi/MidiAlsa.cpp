#include "MidiAlsa.hpp"
#include "i18n.hpp"
#include "utils.hpp"

MidiDevice::MidiDevice()
{
    this->sequencer = NULL;
    this->encoder = NULL;
    this->sourcePort = -1;
    this->hasConnection = false;
}

MidiDevice::~MidiDevice()
{
    this->Close();
}

bool MidiDevice::OpenDevice(u32 uDeviceId)
{
    (void)uDeviceId;

    this->Reset();

    if (this->sequencer == NULL && snd_seq_open(&this->sequencer, "default", SND_SEQ_OPEN_OUTPUT, 0) != 0)
    {
        this->sequencer = NULL;
        goto fail;
    }

    if (this->encoder == NULL && snd_midi_event_new(1024, &this->encoder) != 0)
    {
        this->encoder = NULL;
        goto fail;
    }

    if (this->sourcePort < 0)
    {
        this->sourcePort = snd_seq_create_simple_port(this->sequencer, "TH06 MIDI Out", SND_SEQ_PORT_CAP_WRITE,
                                                      SND_SEQ_PORT_TYPE_MIDI_GENERIC);

        if (this->sourcePort < 0)
        {
            goto fail;
        }
    }

    if (!this->GetDestPort())
    {
        goto fail;
    }

    // Failing to open a connection here isn't necessarily a failure, but may cause issues
    //   with playback for certain clients. We still treat it as a success, in case the
    //   client we're outputting to can work without a connection without issues.
    this->hasConnection = snd_seq_connect_to(this->sequencer, this->sourcePort, this->destClient, this->destPort) == 0;

    this->encoderBufferSize = 1024;

    snd_seq_set_client_name(this->sequencer, TH_WINDOW_TITLE);

    return true;

fail:
    this->Close();
    return false;
}

ZunResult MidiDevice::Close()
{
    this->Reset();

    if (this->encoder != NULL)
    {
        snd_midi_event_free(this->encoder);
        this->encoder = NULL;
    }

    if (this->sourcePort >= 0)
    {
        snd_seq_delete_simple_port(this->sequencer, this->sourcePort);
        this->sourcePort = -1;
    }

    if (this->sequencer != NULL)
    {
        snd_seq_close(this->sequencer);
        this->sequencer = NULL;
    }

    return ZUN_SUCCESS;
}

bool MidiDevice::SendLongMsg(u8 *buf, u32 len)
{
    snd_seq_event_t event;

    if (this->sourcePort < 0)
    {
        return false;
    }

    snd_seq_ev_clear(&event);

    // None of EoSD's MIDIs hit this condition, but it's good to be safe
    if (len > this->encoderBufferSize)
    {
        snd_midi_event_resize_buffer(this->encoder, len);
        this->encoderBufferSize = len;
    }

    snd_midi_event_encode(this->encoder, buf, len, &event);
    snd_seq_ev_set_direct(&event);
    snd_seq_ev_set_source(&event, this->sourcePort);
    snd_seq_ev_set_dest(&event, this->destClient, this->destPort);
    snd_seq_event_output_direct(this->sequencer, &event);

    return true;
}

bool MidiDevice::SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte)
{
    u8 command[3] = {midiStatus, firstByte, secondByte};

    return this->SendLongMsg(command, 3);
}

void MidiDevice::Reset()
{
    if (this->sourcePort >= 0)
    {
        snd_seq_event_t event;

        snd_seq_ev_clear(&event);
        snd_seq_ev_set_direct(&event);
        snd_seq_ev_set_source(&event, this->sourcePort);
        snd_seq_ev_set_dest(&event, this->destClient, this->destPort);

        for (i32 i = 0; i < MIDI_CHANNELS; i++)
        {
            snd_seq_ev_set_controller(&event, i, MIDI_CTL_ALL_SOUNDS_OFF, 0);
            snd_seq_event_output_direct(this->sequencer, &event);
        }

        if (this->hasConnection)
        {
            snd_seq_disconnect_to(this->sequencer, this->sourcePort, this->destClient, this->destPort);
            this->hasConnection = false;
        }
    }
}

bool MidiDevice::GetDestPort()
{
    snd_seq_client_info_t *clientInfo;
    snd_seq_port_info_t *portInfo;

    snd_seq_client_info_alloca(&clientInfo);
    snd_seq_port_info_alloca(&portInfo);

    snd_seq_client_info_set_client(clientInfo, -1);

    // Grab the first client / port combination that's either in hardware or a software synthesizer
    //   This is really not the best way to do things, but anything better would require some sort
    //   of UI to select the ouput port(s)

    while (snd_seq_query_next_client(this->sequencer, clientInfo) == 0)
    {
        snd_seq_port_info_set_client(portInfo, snd_seq_client_info_get_client(clientInfo));
        snd_seq_port_info_set_port(portInfo, -1);

        while (snd_seq_query_next_port(this->sequencer, portInfo) == 0)
        {
            unsigned int portCaps = snd_seq_port_info_get_capability(portInfo);
            unsigned int portType = snd_seq_port_info_get_type(portInfo);

            if (!(portCaps & SND_SEQ_PORT_CAP_WRITE) || !(portType & SND_SEQ_PORT_TYPE_MIDI_GENERIC) ||
                !(portType & (SND_SEQ_PORT_TYPE_HARDWARE | SND_SEQ_PORT_TYPE_SYNTHESIZER)))
            {
                continue;
            }

            this->destClient = snd_seq_client_info_get_client(clientInfo);
            this->destPort = snd_seq_port_info_get_port(portInfo);

            utils::DebugPrint2("Playing midi on address %i:%i (%s : %s)", this->destClient, this->destPort,
                               snd_seq_client_info_get_name(clientInfo), snd_seq_port_info_get_name(portInfo));

            return true;
        }
    }

    return false;
}

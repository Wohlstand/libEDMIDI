#include <cstdio>
#include <limits.h>
#include "CSMFPlay.hpp"
#include "COpllDevice.hpp"
#include "CSccDevice.hpp"

#include "sequencer/midi_sequencer.hpp"
#include "emu_de_midi.h"
#include <assert.h>

#if defined (_MSC_VER)
#   if defined (_DEBUG)
#       define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#   endif
#endif

using namespace dsa;

CSMFPlay::CSMFPlay(DWORD rate, int mods)
{
    m_sequencer = NULL;
    m_sequencerInterface = NULL;
    m_rate = rate;
    m_mods = mods;
    for(int i = 0; i < m_mods; i++)
    {
        if(i & 1)
            m_module[i].AttachDevice(new CSccDevice(rate, 2));
        else
            m_module[i].AttachDevice(new COpllDevice(rate, 2));
    }

    initSequencerInterface();
}

CSMFPlay::~CSMFPlay()
{
    for(int i = 0; i < m_mods; i++)
        delete m_module[i].DetachDevice();
    if(m_sequencer)
        delete m_sequencer;
    if(m_sequencerInterface)
        delete m_sequencerInterface;
}

bool CSMFPlay::Load(const void *buf, int size)
{
    return m_sequencer->loadMIDI(buf, size);
}

bool CSMFPlay::Open(const char *filename)
{
    return m_sequencer->loadMIDI(filename);
}

void CSMFPlay::Start(bool reset)
{
    if(reset)
    {
        for(int i = 0; i < m_mods; i++)
            m_module[i].Reset();
    }

    m_sequencer->seek(0, 0);
}

void CSMFPlay::SetLoop(bool enabled)
{
    m_sequencer->setLoopEnabled(enabled);
}

DWORD CSMFPlay::Render(short *buf, DWORD length)
{
    return DWORD(m_sequencer->playStream(reinterpret_cast<uint8_t *>(buf), static_cast<size_t>(length * 8)));
}


#define GET_MIDI_PLAYER(device) reinterpret_cast<CSMFPlay *>((device)->opn2_midiPlayer)
typedef CSMFPlay MidiPlayer;


OPNMIDI_EXPORT struct OPN2_MIDIPlayer *opn2_init(long sample_rate)
{
    OPN2_MIDIPlayer *midi_device;
    midi_device = (OPN2_MIDIPlayer *)malloc(sizeof(OPN2_MIDIPlayer));
    if(!midi_device)
        return NULL;

    CSMFPlay *player = new(std::nothrow) CSMFPlay(static_cast<DWORD>(sample_rate), 8);
    if(!player)
    {
        free(midi_device);
        return NULL;
    }
    midi_device->opn2_midiPlayer = player;
    return midi_device;
}

OPNMIDI_EXPORT void opn2_close(OPN2_MIDIPlayer *device)
{
    if(!device)
        return;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    delete play;
    device->opn2_midiPlayer = NULL;
    free(device);
    device = NULL;
}


OPNMIDI_EXPORT int opn2_openFile(OPN2_MIDIPlayer *device, const char *filePath)
{
    if(device)
    {
        MidiPlayer *play = GET_MIDI_PLAYER(device);
        assert(play);
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
        if(!play->Open(filePath))
        {
            //            std::string err = play->getErrorString();
            //            if(err.empty())
            //                play->setErrorString("OPN2 MIDI: Can't load file");
            return -1;
        }
        else return 0;
#else
        ADL_UNUSED(filePath);
        //        play->setErrorString("OPNMIDI: MIDI Sequencer is not supported in this build of library!");
        return -1;
#endif
    }

    return -1;
}

OPNMIDI_EXPORT int opn2_openData(OPN2_MIDIPlayer *device, const void *mem, unsigned long size)
{
    if(device)
    {
        MidiPlayer *play = GET_MIDI_PLAYER(device);
        assert(play);
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
        if(!play->Load(mem, static_cast<size_t>(size)))
        {
            //            std::string err = play->getErrorString();
            //            if(err.empty())
            //                play->setErrorString("OPN2 MIDI: Can't load data from memory");
            return -1;
        }
        else return 0;
#else
        ADL_UNUSED(mem);
        ADL_UNUSED(size);
        play->setErrorString("OPNMIDI: MIDI Sequencer is not supported in this build of library!");
        return -1;
#endif
    }

    return -1;
}


OPNMIDI_EXPORT void opn2_reset(OPN2_MIDIPlayer *device)
{
    if(!device)
        return;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    play->Start(true);
}


OPNMIDI_EXPORT double opn2_totalTimeLength(struct OPN2_MIDIPlayer *device)
{
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(!device)
        return -1.0;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    return play->m_sequencer->timeLength();
#else
    ADL_UNUSED(device);
    return -1.0;
#endif
}

OPNMIDI_EXPORT double opn2_loopStartTime(struct OPN2_MIDIPlayer *device)
{
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(!device)
        return -1.0;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    return play->m_sequencer->getLoopStart();
#else
    ADL_UNUSED(device);
    return -1.0;
#endif
}

OPNMIDI_EXPORT double opn2_loopEndTime(struct OPN2_MIDIPlayer *device)
{
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(!device)
        return -1.0;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    return play->m_sequencer->getLoopEnd();
#else
    ADL_UNUSED(device);
    return -1.0;
#endif
}

OPNMIDI_EXPORT double opn2_positionTell(struct OPN2_MIDIPlayer *device)
{
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(!device)
        return -1.0;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    return play->m_sequencer->tell();
#else
    ADL_UNUSED(device);
    return -1.0;
#endif
}

OPNMIDI_EXPORT void opn2_positionSeek(struct OPN2_MIDIPlayer *device, double seconds)
{
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(seconds < 0.0)
        return;//Seeking negative position is forbidden! :-P
    if(!device)
        return;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    //    play->realTime_panic();
    play->m_delay = play->m_sequencer->seek(seconds, play->m_mindelay);
    play->m_time_rest = 0.0;
#else
    ADL_UNUSED(device);
    ADL_UNUSED(seconds);
#endif
}

OPNMIDI_EXPORT void opn2_positionRewind(struct OPN2_MIDIPlayer *device)
{
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(!device)
        return;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    //    play->realTime_panic();
    play->m_sequencer->rewind();
#else
    ADL_UNUSED(device);
#endif
}

OPNMIDI_EXPORT void opn2_setTempo(struct OPN2_MIDIPlayer *device, double tempo)
{
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(!device || (tempo <= 0.0))
        return;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    play->m_sequencer->setTempo(tempo);
#else
    ADL_UNUSED(device);
    ADL_UNUSED(tempo);
#endif
}


OPNMIDI_EXPORT size_t opn2_metaMarkerCount(struct OPN2_MIDIPlayer *device)
{
#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(!device)
        return 0;
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    return play->m_sequencer->getMarkers().size();
#else
    ADL_UNUSED(device);
    return 0;
#endif
}

OPNMIDI_EXPORT Opn2_MarkerEntry opn2_metaMarker(struct OPN2_MIDIPlayer *device, size_t index)
{
    struct Opn2_MarkerEntry marker;

#ifndef OPNMIDI_DISABLE_MIDI_SEQUENCER
    if(!device)
    {
        marker.label = "INVALID";
        marker.pos_time = 0.0;
        marker.pos_ticks = 0;
        return marker;
    }

    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);

    const std::vector<MidiSequencer::MIDI_MarkerEntry> &markers = play->m_sequencer->getMarkers();
    if(index >= markers.size())
    {
        marker.label = "INVALID";
        marker.pos_time = 0.0;
        marker.pos_ticks = 0;
        return marker;
    }

    const MidiSequencer::MIDI_MarkerEntry &mk = markers[index];
    marker.label = mk.label.c_str();
    marker.pos_time = mk.pos_time;
    marker.pos_ticks = (unsigned long)mk.pos_ticks;
#else
    (void)device; (void)index;
    marker.label = "NOT SUPPORTED";
    marker.pos_time = 0.0;
    marker.pos_ticks = 0;
#endif
    return marker;
}


OPNMIDI_EXPORT int opn2_play(struct OPN2_MIDIPlayer *device, int sampleCount, short *out)
{
    MidiPlayer *play = GET_MIDI_PLAYER(device);
    assert(play);
    return static_cast<int>(play->Render(out, sampleCount));
}


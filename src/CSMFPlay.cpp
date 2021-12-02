#include <cstdio>
#include <limits.h>
#include "CSMFPlay.hpp"
#include "COpllDevice.hpp"
#include "CSccDevice.hpp"

#include "sequencer/midi_sequencer.hpp"

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
    bool ret = m_sequencer->loadMIDI(buf, size);
    Reset();
    return ret;
}

bool CSMFPlay::Open(const char *filename)
{
    bool ret = m_sequencer->loadMIDI(filename);
    Reset();
    return ret;
}

void CSMFPlay::Start(bool reset)
{
    if(reset)
        Reset();
    m_sequencer->rewind();
}

void CSMFPlay::Reset()
{
    for(int i = 0; i < m_mods; i++)
        m_module[i].Reset();
}

void CSMFPlay::Rewind()
{
    m_sequencer->rewind();
}

void CSMFPlay::Panic()
{
    for(int i = 0; i < m_mods; i++)
        m_module[i].SendPanic();
}

void CSMFPlay::Seek(double seconds)
{
    m_sequencer->seek(seconds, 1.0);
}

double CSMFPlay::Tell()
{
    return m_sequencer->tell();
}

double CSMFPlay::Duration()
{
    return m_sequencer->timeLength();
}

double CSMFPlay::loopStart()
{
    return m_sequencer->getLoopStart();
}

double CSMFPlay::loopEnd()
{
    return m_sequencer->getLoopEnd();
}

void CSMFPlay::SetLoop(bool enabled)
{
    m_sequencer->setLoopEnabled(enabled);
}

void CSMFPlay::SetLoopsNumber(int loops)
{
    m_sequencer->setLoopsCount(loops);
}

bool CSMFPlay::GetLoop()
{
    return m_sequencer->getLoopEnabled();
}

bool CSMFPlay::SeqEof()
{
    return m_sequencer->positionAtEnd();
}

void CSMFPlay::setTempo(double tempo)
{
    m_sequencer->setTempo(tempo);
}

double CSMFPlay::getTempo()
{
    return m_sequencer->getTempoMultiplier();
}

int CSMFPlay::tracksCount()
{
    return (int)m_sequencer->getTrackCount();
}

int CSMFPlay::setTrackEnabled(int track, bool en)
{
    return (int)m_sequencer->setTrackEnabled(track, en);
}

int CSMFPlay::setChannelEnabled(int chan, bool en)
{
    return (int)m_sequencer->setChannelEnabled(chan, en);
}

void CSMFPlay::setTriggerHandler(EDMIDI_TriggerHandler handler, void *userData)
{
    m_sequencer->setTriggerHandler(handler, userData);
}

const std::string &CSMFPlay::getMusicTitle()
{
    return m_sequencer->getMusicTitle();
}

const std::string &CSMFPlay::getMusicCopyright()
{
    return m_sequencer->getMusicCopyright();
}

const std::vector<std::string> &CSMFPlay::getTrackTitles()
{
    return m_sequencer->getTrackTitles();
}

size_t CSMFPlay::getMarkersCount()
{
    return m_sequencer->getMarkers().size();
}

EdMidi_MarkerEntry CSMFPlay::getMarker(size_t index)
{
    struct EdMidi_MarkerEntry marker;
    const std::vector<MidiSequencer::MIDI_MarkerEntry> &markers = m_sequencer->getMarkers();
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

    return marker;
}

void CSMFPlay::setDebugMessageHook(EDMIDI_DebugMessageHook debugMessageHook, void *userData)
{
    m_sequencerInterface->onDebugMessage = debugMessageHook;
    m_sequencerInterface->onDebugMessage_userData = userData;
}

const std::string &CSMFPlay::getErrorString() const
{
    return m_error;
}

void CSMFPlay::setErrorString(const std::string &err)
{
    m_error = err;
}

namespace dsa
{
extern void playSynth(void *userdata, uint8_t *stream, size_t length);
extern void playSynthS16(void *userdata, uint8_t *stream, size_t length);
extern void playSynthF32(void *userdata, uint8_t *stream, size_t length);
}

DWORD CSMFPlay::Render(int *buf, DWORD length)
{
    if(m_sequencerInterface->onPcmRender != playSynth)
    {
        m_sequencerInterface->onPcmRender = playSynth;
        m_sequencerInterface->pcmFrameSize = 2 /*channels*/ * 4 /*size of one sample*/;
    }

    return DWORD(m_sequencer->playStream(reinterpret_cast<uint8_t *>(buf), static_cast<size_t>(length * 8)));
}

int CSMFPlay::RenderS16(short *buf, DWORD length)
{
    if(m_sequencerInterface->onPcmRender != playSynthS16)
    {
        m_sequencerInterface->onPcmRender = playSynthS16;
        m_sequencerInterface->pcmFrameSize = 2 /*channels*/ * 2 /*size of one sample*/;
    }
    return m_sequencer->playStream(reinterpret_cast<uint8_t *>(buf), static_cast<size_t>(length * 4));
}

int CSMFPlay::RenderF32(float *buf, DWORD length)
{
    if(m_sequencerInterface->onPcmRender != playSynthF32)
    {
        m_sequencerInterface->onPcmRender = playSynthF32;
        m_sequencerInterface->pcmFrameSize = 2 /*channels*/ * 4 /*size of one sample*/;
    }
    return m_sequencer->playStream(reinterpret_cast<uint8_t *>(buf), static_cast<size_t>(length * 8));
}

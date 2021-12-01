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
    return m_sequencer->loadMIDI(buf, size);
}

bool CSMFPlay::Open(char *filename)
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

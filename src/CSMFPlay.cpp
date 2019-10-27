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
    rendered_ticks = 0;
    wanted_ticks = 0;
    for(int i = 0; i < m_mods; i++)
    {
        if(i & 1)
            m_module[i].AttachDevice(new CSccDevice(rate, 2));
        else
            m_module[i].AttachDevice(new COpllDevice(rate, 2));
    }

    m_mindelay = 1.0 / static_cast<double>(m_rate);
    m_maxdelay = 512.0 / static_cast<double>(m_rate);
    m_delay = 0.0;
    m_carry = 0.0;
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

    m_tempo = (60 * 1000000) / 120;
    m_time_rest = 0.0;

    rendered_ticks = 0;
    wanted_ticks = 0;

    m_delta.clear();
    m_end_flag.clear();

    m_sequencer->seek(0, 0);
}

void CSMFPlay::SetLoop(bool enabled)
{
    m_sequencer->setLoopEnabled(enabled);
}

void CSMFPlay::SetEndPoint(int tick)
{
    wanted_ticks = tick;
}

DWORD CSMFPlay::Render(int *buf, DWORD length)
{
    DWORD idx = 0;
    length -= length % 2; // サンプルリクエストでさえ避ける
    int left = length;
    long n_periodCountStereo = 0;

    while(left > 0)
    {
        const double leftDelay = left / double(m_rate);
        const double maxDelay = m_carry < leftDelay ? m_carry : leftDelay;
        if((m_sequencer->positionAtEnd()) && (m_delay <= 0.0))
            break;//Stop to fetch samples at reaching the song end with disabled loop

        m_carry -= maxDelay;
        n_periodCountStereo = double(m_rate) * maxDelay;

        if(buf)
        {
            size_t to_generate = 0;
            size_t in_generatedStereo = n_periodCountStereo > left ? left : n_periodCountStereo;
            while(to_generate < in_generatedStereo)
            {
                buf[idx * 2] = buf[idx * 2 + 1] = 0;
                for(int i = 0; i < m_mods; i++)
                {
                    INT32 b[2];
                    m_module[i].Render(b);
                    buf[idx * 2] += b[0];
                    buf[idx * 2 + 1] += b[1];
                }
                idx++;
                left--;
                to_generate++;
            }
        }

        if(m_carry <= 0.0)
        {
            m_delay = m_sequencer->Tick(m_delay, m_mindelay);
            m_carry += m_delay;
        }
    } // end while (left > 0)

    return idx;
}

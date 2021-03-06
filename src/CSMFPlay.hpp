#ifndef __CSMFPLAY_HPP__
#define __CSMFPLAY_HPP__
#include <vector>

#include "CMIDIModule.hpp"

// クラスの名前を変更してABIの衝突を回避する
#define BW_MidiSequencer EmuDeMidiMidiSequencer
class BW_MidiSequencer;
typedef BW_MidiSequencer MidiSequencer;
typedef struct BW_MidiRtInterface BW_MidiRtInterface;

namespace dsa
{

class CSMFPlay
{
    friend CMIDIModule &getModule(void *userdata, uint8_t channel);
    friend CMIDIModule &getModule2(void *userdata, uint8_t channel);
    friend void playSynth(void *userdata, uint8_t *stream, size_t length);
    CMIDIModule m_module[16];

    int m_mods;
    int m_rate;

    MidiSequencer *m_sequencer;
    BW_MidiRtInterface *m_sequencerInterface;
    void initSequencerInterface();

    double Tick(double s, double granularity);

public:
    CSMFPlay(DWORD rate, int mods = 4);
    ~CSMFPlay();
    bool Open(char *filename);
    bool Load(const void *buf, int size);
    DWORD Render(int *buf, DWORD length);
    void Start(bool reset = true);

    void SetLoop(bool enabled);
};

} // namespace dsa

#endif

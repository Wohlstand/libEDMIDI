#include <stdio.h>
#include <stdlib.h>

#include <conio.h>
#include <ctype.h>

#define NOMINMAX

#include <windows.h>
#include <mmsystem.h>

#include <algorithm>

#include "CSMFPlay.hpp"

#if defined (_MSC_VER) && defined (_DEBUG)
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif

static const int MaxModules = 8;

static const WORD MaxChannels = 2;
static const DWORD SampleRate = 96'000;

static int32_t SrcFrames[2][SampleRate * MaxChannels];
static int16_t DstFrames[2][SampleRate * MaxChannels];

static void CALLBACK Callback(HWAVEOUT /*hWaveOut*/, UINT msg, DWORD_PTR dwInstance, DWORD_PTR /*dwParam1*/, DWORD /*dwParam2*/)
{
    switch (msg)
    {
        case WOM_OPEN:
            break;

        case WOM_CLOSE:
            break;

        case WOM_DONE:
            *((DWORD_PTR *) dwInstance) = 1;
            break;

        default:
            break;
    }

    return;
};

static bool Render(dsa::CSMFPlay * player, WAVEHDR * wh, size_t waveHeaderIndex, HWAVEOUT hWaveOut)
{
    auto & Src = SrcFrames[waveHeaderIndex];
    auto & Dst = DstFrames[waveHeaderIndex];

    if (player->Render(Src, SampleRate) == 0)
        return false;

    for (size_t i = 0; i < _countof(Dst); ++i)
    {
        auto Sample = (double) Src[i];

        if (Sample < INT16_MIN) Sample = INT16_MIN;
        if (Sample > INT16_MAX) Sample = INT16_MAX;

        Dst[i] = (int16_t) Sample;
    }

    wh[waveHeaderIndex].lpData         = (LPSTR) Dst;
    wh[waveHeaderIndex].dwBufferLength = (DWORD) ((size_t) SampleRate * (size_t) MaxChannels * sizeof(Dst[0]));
    wh[waveHeaderIndex].dwFlags        = WHDR_DONE;

    ::waveOutPrepareHeader  (hWaveOut, &wh[waveHeaderIndex], sizeof(WAVEHDR));
    ::waveOutWrite          (hWaveOut, &wh[waveHeaderIndex], sizeof(WAVEHDR));
    ::waveOutUnprepareHeader(hWaveOut, &wh[waveHeaderIndex], sizeof(WAVEHDR));

    return true;
}

int main(int argc, char * argv[])
{
    if (argc <= 1)
    {
        ::puts("Usage: midiplay filename.mid");

        return 0;
    }

#if defined(_MSC_VER)
#ifdef _DEBUG
    HANDLE hDbgfile = ::CreateFileA(R"(midiplay.log)", GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, (HANDLE) hDbgfile);
//  _CrtCrtSetBreakAlloc(0);
#endif
#endif

    auto * Player = new dsa::CSMFPlay(SampleRate, MaxModules); 

    if (!Player->Open(argv[1]))
    {
        ::printf("Unable to open \"%s\"!\n", argv[1]);

        return 1;
    }

    Player->Start();

    WAVEFORMATEX wfx = { };

    wfx.wFormatTag      = WAVE_FORMAT_PCM; // Required for 1 or 2-channel PCM data.
    wfx.nChannels       = MaxChannels;
    wfx.wBitsPerSample  = (WORD) (sizeof(DstFrames[0][0]) * 8);
    wfx.nSamplesPerSec  = SampleRate;
    wfx.nBlockAlign     = wfx.nChannels * (wfx.wBitsPerSample >> 3);
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    HWAVEOUT hWaveOut;
    static volatile DWORD IsDone = 0;

    auto Result = ::waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR) Callback, (DWORD_PTR) &IsDone, CALLBACK_FUNCTION);

    if (Result != MMSYSERR_NOERROR)
    {
        perror("Unable to open a wave device.");
        exit(1);
    }

    ::puts("Press q to quit...");

    WAVEHDR wh[2] = { };

    size_t WaveHeaderIndex = 0;

    Render(Player, wh, WaveHeaderIndex, hWaveOut);

    bool StopRendering = false;

    while (!StopRendering)
    {
        ++WaveHeaderIndex;

        if (WaveHeaderIndex == _countof(wh))
            WaveHeaderIndex = 0;

        if (!Render(Player, wh, WaveHeaderIndex, hWaveOut))
            break;

        if (IsDone)
            break;

        while (!IsDone)
        {
            if (!_kbhit())
                continue;

            int c = _getch();

            if (c == 'q')
            {
                StopRendering = true;
                break;
            }
        }

        IsDone = 0;
    }

    ::waveOutClose(hWaveOut);

    delete Player;

#if defined(_MSC_VER) && defined(_DEBUG)
    if (_CrtDumpMemoryLeaks())
        ::puts("Memory leaks detected.");

    ::CloseHandle(hDbgfile);
#endif

    ::puts("Done.");

    return 0;
}

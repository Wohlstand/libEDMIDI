#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <SDL2/SDL.h>

#include <emu_de_midi.h>

/* prototype for our audio callback */
/* see the implementation for more information */
static void my_audio_callback(void *midi_player, Uint8 *stream, int len);

/* variable declarations */
static SDL_bool s_is_playing = SDL_FALSE; /* remaining length of the sample we have to play */

static void stop_playing(int sig)
{
    (void)sig;
    s_is_playing = SDL_FALSE;
}

#if defined(__WATCOMC__)
#include <stdio.h> // snprintf is here!
#define flushout(stream)
#else
#define flushout(stream) fflush(stream)
#endif

static void debugPrint(void *userdata, const char *fmt, ...)
{
    char buffer[4096];
    va_list args;
    int rc;

    (void)userdata;

    va_start(args, fmt);
    rc = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if(rc > 0)
    {
        fprintf(stdout, " - Debug: %s\n", buffer);
        flushout(stdout);
    }
}

static inline void secondsToHMSM(double seconds_full, char *hmsm_buffer, size_t hmsm_buffer_size)
{
    double seconds_integral;
    double seconds_fractional = modf(seconds_full, &seconds_integral);
    unsigned int milliseconds = (unsigned int)(seconds_fractional * 1000.0);
    unsigned int seconds = (unsigned int)(SDL_fmod(seconds_full, 60.0));
    unsigned int minutes = (unsigned int)(SDL_fmod(seconds_full / 60, 60.0));
    unsigned int hours   = (unsigned int)(seconds_full / 3600);

    SDL_memset(hmsm_buffer, 0, hmsm_buffer_size);
    if (hours > 0)
        sprintf(hmsm_buffer, "%02u:%02u:%02u,%03u", hours, minutes, seconds, milliseconds);
    else
        sprintf(hmsm_buffer, "%02u:%02u,%03u", minutes, seconds, milliseconds);
}


int main(int argc, char **argv)
{
    /* local variables */
    static SDL_AudioSpec            spec, obtained; /* the specs of our piece of music */
    static struct EDMIDIPlayer      *midi_player = NULL; /* Instance of Emu De Midi player */
    static const char               *music_path = NULL; /* Path to music file */
    SDL_bool loopEnabled = SDL_TRUE;
    char totalHMS[25];
    char loopStartHMS[25];
    char loopEndHMS[25];
    char posHMS[25];
    double loopStart, loopEnd, total;
    uint64_t milliseconds_prev = ~0u;
    int printsCounter = 0;
    int printsCounterPeriod = 1;
    double time_pos;
    uint64_t milliseconds;

    SDL_memset(posHMS, 0, 25);
    SDL_memset(totalHMS, 0, 25);
    SDL_memset(loopStartHMS, 0, 25);
    SDL_memset(loopEndHMS, 0, 25);

    signal(SIGINT, stop_playing);
    signal(SIGTERM, stop_playing);

    fprintf(stdout, "==========================================\n"
                    "         libEDMIDI demo utility\n"
                    "==========================================\n\n");
    flushout(stdout);

    if (argc < 2)
    {
        fprintf(stderr, "\n"
                        "\n"
                        "No given files to play!\n"
                        "\n"
                        "Syntax: %s <path-to-MIDI-file>\n"
                        "\n", argv[0]);
        return 2;
    }

    music_path = argv[1];

    fprintf(stdout, " - Library version %s\n", edmidi_linkedLibraryVersion());

    /* Initialize EmuDeMidi */
    midi_player = edmidi_initEx(44100, 16);
    if (!midi_player)
    {
        fprintf(stderr, "Couldn't initialize EDMIDI: %s\n", edmidi_errorString());
        return 1;
    }

    //Set internal debug messages hook to print all libADLMIDI's internal debug messages
    edmidi_setDebugMessageHook(midi_player, debugPrint, NULL);

    /* Open the MIDI (or MUS, IMF or CMF) file to play */
    if (edmidi_openFile(midi_player, music_path) < 0)
    {
        fprintf(stderr, "Couldn't open music file: %s\n", edmidi_errorInfo(midi_player));
        edmidi_close(midi_player);
        return 1;
    }

    edmidi_setLoopEnabled(midi_player, loopEnabled);

    total               = edmidi_totalTimeLength(midi_player);
    loopStart           = edmidi_loopStartTime(midi_player);
    loopEnd             = edmidi_loopEndTime(midi_player);

    secondsToHMSM(total, totalHMS, 25);
    if(loopStart >= 0.0 && loopEnd >= 0.0)
    {
        secondsToHMSM(loopStart, loopStartHMS, 25);
        secondsToHMSM(loopEnd, loopEndHMS, 25);
    }


    fprintf(stdout, " - Track count: %lu\n", (unsigned long)(edmidi_trackCount(midi_player)));

    fprintf(stdout, " - Loop is turned %s\n", loopEnabled ? "ON" : "OFF");
    if(loopStart >= 0.0 && loopEnd >= 0.0)
        fprintf(stdout, " - Has loop points: %s ... %s\n", loopStartHMS, loopEndHMS);
    fprintf(stdout, "\n==========================================\n");
    flushout(stdout);

    /* Initialize SDL.*/
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        edmidi_close(midi_player);
        return 1;
    }

    SDL_memset(&spec, 0, sizeof(SDL_AudioSpec));
    spec.freq = 44100;
    spec.format = AUDIO_F32SYS;
    spec.channels = 2;
    spec.samples = 4096;

    /* set the callback function */
    spec.callback = my_audio_callback;
    /* set ADLMIDI's descriptor as userdata to use it for sound generation */
    spec.userdata = midi_player;

    /* Open the audio device */
    if(SDL_OpenAudio(&spec, &obtained) < 0)
    {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        edmidi_close(midi_player);
        return 1;
    }

    fprintf(stdout, "                                               \r");

    s_is_playing = SDL_TRUE;
    /* Start playing */
    SDL_PauseAudio(0);

    /* wait until we're don't playing */
    while(s_is_playing)
    {
        SDL_LockAudio();
        time_pos = edmidi_positionTell(midi_player);
        SDL_UnlockAudio();
        milliseconds = (uint64_t)(time_pos * 1000.0);

        if(milliseconds != milliseconds_prev)
        {
            if(printsCounter >= printsCounterPeriod)
            {
                printsCounter = -1;
                secondsToHMSM(time_pos, posHMS, 25);
                fprintf(stdout, "                                               \r");
                fprintf(stdout, "Time position: %s / %s\r", posHMS, totalHMS);
                flushout(stdout);
                milliseconds_prev = milliseconds;
            }
            printsCounter++;
        }
        SDL_Delay(100);
    }

    fprintf(stdout, "                                               \n\n");
    flushout(stdout);

    /* shut everything down */
    SDL_CloseAudio();
    edmidi_close(midi_player);

    return 0 ;
}


/*
 audio callback function
 here you have to copy the data of your audio buffer into the
 requesting audio buffer (stream)
 you should only copy as much as the requested length (len)
*/
static void my_audio_callback(void *midi_player, Uint8 *stream, int len)
{
    struct EDMIDIPlayer *mp = (struct EDMIDIPlayer*)midi_player;
    int count = len / sizeof(float);
    float* dest = (float*)stream;
    int got = edmidi_playF32(mp, count, dest);
    if(got == 0)
    {
        s_is_playing = SDL_FALSE;
        SDL_memset(stream, 0, len);
    }
}

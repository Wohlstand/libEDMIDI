#ifndef EMU_DE_MIDI_H
#define EMU_DE_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
typedef uint8_t         OPN2_UInt8;
typedef uint16_t        OPN2_UInt16;
typedef int8_t          OPN2_SInt8;
typedef int16_t         OPN2_SInt16;
#else
typedef unsigned char   OPN2_UInt8;
typedef unsigned short  OPN2_UInt16;
typedef char            OPN2_SInt8;
typedef short           OPN2_SInt16;
#endif

#ifdef EDMIDI_BUILD
#   ifndef OPNMIDI_DECLSPEC
#       if defined (_WIN32) && defined(OPNMIDI_BUILD_DLL)
#           define OPNMIDI_DECLSPEC __declspec(dllexport)
#       else
#           define OPNMIDI_DECLSPEC
#       endif
#   endif
#else
#   define OPNMIDI_DECLSPEC
#endif

/**
 * @brief Instance of the library
 */
struct OPN2_MIDIPlayer
{
    /*! Private context descriptor */
    void *opn2_midiPlayer;
};

/* ======== Initialization ======== */

/**
 * @brief Initialize OPNMIDI Player device
 *
 * Tip 1: You can initialize multiple instances and run them in parallel
 * Tip 2: Library is NOT thread-safe, therefore don't use same instance in different threads or use mutexes
 * Tip 3: Changing of sample rate on the fly is not supported. Re-create the instance again.
 * Top 4: To generate output in OPN2 or OPNA chip native sample rate, please initialize it with sample rate
 *        value as `OPN_OPN2_SAMPLE_RATE` or `OPN_OPNA_SAMPLE_RATE` in dependence on the chip
 *
 * @param sample_rate Output sample rate
 * @return Instance of the library. If NULL was returned, check the `adl_errorString` message for more info.
 */
extern OPNMIDI_DECLSPEC struct OPN2_MIDIPlayer *opn2_init(long sample_rate);

/**
 * @brief Close and delete OPNMIDI device
 * @param device Instance of the library
 */
extern OPNMIDI_DECLSPEC void opn2_close(struct OPN2_MIDIPlayer *device);


/* ======== MIDI Sequencer ======== */

/**
 * @brief Load MIDI (or any other supported format) file from File System
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @param filePath Absolute or relative path to the music file. UTF8 encoding is required, even on Windows.
 * @return 0 on success, <0 when any error has occurred
 */
extern OPNMIDI_DECLSPEC int opn2_openFile(struct OPN2_MIDIPlayer *device, const char *filePath);

/**
 * @brief Load MIDI (or any other supported format) file from memory data
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @param mem Pointer to memory block where is raw data of music file is stored
 * @param size Size of given memory block
 * @return 0 on success, <0 when any error has occurred
 */
extern OPNMIDI_DECLSPEC int opn2_openData(struct OPN2_MIDIPlayer *device, const void *mem, unsigned long size);

/**
 * @brief Resets MIDI player (per-channel setup) into initial state
 * @param device Instance of the library
 */
extern OPNMIDI_DECLSPEC void opn2_reset(struct OPN2_MIDIPlayer *device);


/**
 * @brief Get total time length of current song
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @return Total song length in seconds
 */
extern OPNMIDI_DECLSPEC double opn2_totalTimeLength(struct OPN2_MIDIPlayer *device);

/**
 * @brief Get loop start time if presented.
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @return Time position in seconds of loop start point, or -1 when file has no loop points
 */
extern OPNMIDI_DECLSPEC double opn2_loopStartTime(struct OPN2_MIDIPlayer *device);

/**
 * @brief Get loop endtime if presented.
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @return Time position in seconds of loop end point, or -1 when file has no loop points
 */
extern OPNMIDI_DECLSPEC double opn2_loopEndTime(struct OPN2_MIDIPlayer *device);

/**
 * @brief Get current time position in seconds
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @return Current time position in seconds
 */
extern OPNMIDI_DECLSPEC double opn2_positionTell(struct OPN2_MIDIPlayer *device);

/**
 * @brief Jump to absolute time position in seconds
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @param seconds Destination time position in seconds to seek
 */
extern OPNMIDI_DECLSPEC void opn2_positionSeek(struct OPN2_MIDIPlayer *device, double seconds);

/**
 * @brief Reset MIDI track position to begin
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 */
extern OPNMIDI_DECLSPEC void opn2_positionRewind(struct OPN2_MIDIPlayer *device);

/**
 * @brief Set tempo multiplier
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @param tempo Tempo multiplier value: 1.0 - original tempo, >1 - play faster, <1 - play slower
 */
extern OPNMIDI_DECLSPEC void opn2_setTempo(struct OPN2_MIDIPlayer *device, double tempo);

/**
 * @brief Returns 1 if music position has reached end
 * @param device Instance of the library
 * @return 1 when end of sing has been reached, otherwise, 0 will be returned. <0 is returned on any error
 */
extern OPNMIDI_DECLSPEC int opn2_atEnd(struct OPN2_MIDIPlayer *device);

/**
 * @brief Returns the number of tracks of the current sequence
 * @param device Instance of the library
 * @return Count of tracks in the current sequence
 */
extern OPNMIDI_DECLSPEC size_t opn2_trackCount(struct OPN2_MIDIPlayer *device);

/* ======== Meta-Tags ======== */

/**
 * @brief Returns string which contains a music title
 * @param device Instance of the library
 * @return A string that contains music title
 */
extern OPNMIDI_DECLSPEC const char *opn2_metaMusicTitle(struct OPN2_MIDIPlayer *device);

/**
 * @brief Returns string which contains a copyright string*
 * @param device Instance of the library
 * @return A string that contains copyright notice, otherwise NULL
 */
extern OPNMIDI_DECLSPEC const char *opn2_metaMusicCopyright(struct OPN2_MIDIPlayer *device);

/**
 * @brief Returns count of available track titles
 *
 * NOTE: There are CAN'T be associated with channel in any of event or note hooks
 *
 * @param device Instance of the library
 * @return Count of available MIDI tracks, otherwise NULL
 */
extern OPNMIDI_DECLSPEC size_t opn2_metaTrackTitleCount(struct OPN2_MIDIPlayer *device);

/**
 * @brief Get track title by index
 * @param device Instance of the library
 * @param index Index of the track to retreive the title
 * @return A string that contains track title, otherwise NULL.
 */
extern OPNMIDI_DECLSPEC const char *opn2_metaTrackTitle(struct OPN2_MIDIPlayer *device, size_t index);

/**
 * @brief MIDI Marker structure
 */
struct Opn2_MarkerEntry
{
    /*! MIDI Marker title */
    const char      *label;
    /*! Absolute time position of the marker in seconds */
    double          pos_time;
    /*! Absolute time position of the marker in MIDI ticks */
    unsigned long   pos_ticks;
};

/**
 * @brief Returns count of available markers
 * @param device Instance of the library
 * @return Count of available MIDI markers
 */
extern OPNMIDI_DECLSPEC size_t opn2_metaMarkerCount(struct OPN2_MIDIPlayer *device);

/**
 * @brief Returns the marker entry
 * @param device Instance of the library
 * @param index Index of the marker to retreive it.
 * @return MIDI Marker description structure.
 */
extern OPNMIDI_DECLSPEC struct Opn2_MarkerEntry opn2_metaMarker(struct OPN2_MIDIPlayer *device, size_t index);


/* ======== Audio output Generation ======== */

/**
 * @brief Generate PCM signed 16-bit stereo audio output and iterate MIDI timers
 *
 * Use this function when you are playing MIDI file loaded by `adl_openFile` or by `adl_openData`
 * with using of built-in MIDI sequencer.
 *
 * Don't use count of frames, use instead count of samples. One frame is two samples.
 * So, for example, if you want to take 10 frames, you must to request amount of 20 samples!
 *
 * Available when library is built with built-in MIDI Sequencer support.
 *
 * @param device Instance of the library
 * @param sampleCount Count of samples (not frames!)
 * @param out Pointer to output with 16-bit stereo PCM output
 * @return Count of given samples, otherwise, 0 or when catching an error while playing
 */
extern OPNMIDI_DECLSPEC int  opn2_play(struct OPN2_MIDIPlayer *device, int sampleCount, short *out);

#ifdef __cplusplus
}
#endif

#endif /* EMU_DE_MIDI_H */

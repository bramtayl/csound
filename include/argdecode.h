#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set a single csound option (flag). Returns CSOUND_SUCCESS on success.
 * NB: blank spaces are not allowed
 */
PUBLIC int csoundSetOption(CSOUND *csound, const char *option);

/**
 *  Configure Csound with a given set of parameters defined in
 *  the CSOUND_PARAMS structure. These parameters are the part of the
 *  OPARMS struct that are configurable through command line flags.
 *  The CSOUND_PARAMS structure can be obtained using csoundGetParams().
 *  These options should only be changed before performance has started.
 */
PUBLIC void csoundSetParams(CSOUND *csound, CSOUND_PARAMS *p);

/**
 *  Get the current set of parameters from a CSOUND instance in
 *  a CSOUND_PARAMS structure. See csoundSetParams().
 */
PUBLIC void csoundGetParams(CSOUND *csound, CSOUND_PARAMS *p);

/**
 *  Set output destination, type and format
 *  type can be one of  "wav","aiff", "au","raw", "paf", "svx", "nist", "voc",
 *  "ircam","w64","mat4", "mat5", "pvf","xi", "htk","sds","avr","wavex","sd2",
 *  "flac", "caf","wve","ogg","mpc2k","rf64", or NULL (use default or
 *  realtime IO).
 *  format can be one of "alaw", "schar", "uchar", "float", "double", "long",
 *  "short", "ulaw", "24bit", "vorbis", or NULL (use default or realtime IO).
 *   For RT audio, use device_id from CS_AUDIODEVICE for a given audio device.
 *
 */
PUBLIC void csoundSetOutput(CSOUND *csound, const char *name, const char *type,
                            const char *format);

/**
 *  Get output type and format.
 *  type should have space for at least 5 chars excluding termination,
 *  and format should have space for at least 7 chars.
 *  On return, these will hold the current values for
 *  these parameters.
 */
PUBLIC void csoundGetOutputFormat(CSOUND *csound, char *type, char *format);

/**
 *  Set input source
 */
PUBLIC void csoundSetInput(CSOUND *csound, const char *name);

/**
 *  Set MIDI input device name/number
 */
PUBLIC void csoundSetMIDIInput(CSOUND *csound, const char *name);

/**
 *  Set MIDI file input name
 */
PUBLIC void csoundSetMIDIFileInput(CSOUND *csound, const char *name);

/**
 *  Set MIDI output device name/number
 */
PUBLIC void csoundSetMIDIOutput(CSOUND *csound, const char *name);

/**
 *  Set MIDI file utput name
 */
PUBLIC void csoundSetMIDIFileOutput(CSOUND *csound, const char *name);

#ifdef __cplusplus
}
#endif
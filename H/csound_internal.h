#pragma once

#include "csoundCore_common.h"

typedef struct opcodeDeinit_s {
  void    *p;
  int     (*func)(CSOUND *, void *);
  void    *nxt;
} opcodeDeinit_t;

void print_engine_parameters(CSOUND *csound);

void    csoundErrorMsgS(CSOUND *, int attr, const char *, ...);

/**
 * Set release time in control periods (1 / csound->ekr second units)
 * for opcode 'p' to 'n'. If the current release time is longer than
 * the specified value, it is not changed.
 * Returns the new release time.
 */
int csoundSetReleaseLength(void *p, int n);

/**
 * Set release time in seconds for opcode 'p' to 'n'.
 * If the current release time is longer than the specified value,
 * it is not changed.
 * Returns the new release time in seconds.
 */
MYFLT csoundSetReleaseLengthSeconds(void *p, MYFLT n);

/**
 * Returns pointer to a string constant storing an error massage
 * for error code 'errcode'.
 */
const char *csoundExternalMidiErrorString(CSOUND *, int errcode);

int32_t csoundReadScoreInternal(CSOUND *csound, const char *str);

/* call the opcode deinitialisation routines of an instrument instance */
/* called from deact() in insert.c */

int csoundDeinitialiseOpcodes(CSOUND *csound, INSDS *ip);

int playopen_dummy(CSOUND *csound, const csRtAudioParams *parm);

double *get_dummy_rtaudio_globals(CSOUND *csound);

void dummy_rtaudio_timer(CSOUND *csound, double *p);

void rtplay_dummy(CSOUND *csound, const MYFLT *outBuf, int nbytes);

int recopen_dummy(CSOUND *csound, const csRtAudioParams *parm);

int rtrecord_dummy(CSOUND *csound, MYFLT *inBuf, int nbytes);

void rtclose_dummy(CSOUND *);

int  audio_dev_list_dummy(CSOUND *, CS_AUDIODEVICE *, int);

int  midi_dev_list_dummy(CSOUND *csound, CS_MIDIDEVICE *list, int isOutput);

int DummyMidiInOpen(CSOUND *csound, void **userData,
                           const char *devName);

int DummyMidiRead(CSOUND *csound, void *userData,
                         unsigned char *buf, int nbytes);

int DummyMidiOutOpen(CSOUND *csound, void **userData,
                     const char *devName);

int DummyMidiWrite(CSOUND *csound, void *userData,
                   const unsigned char *buf, int nbytes);
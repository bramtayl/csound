#pragma once

#include "csoundCore_common.h"

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

typedef struct pvx_memfile_ {
  char *filename;
  struct pvx_memfile_ *nxt;
  float *data;
  uint32 nframes;
  int format;
  int fftsize;
  int overlap;
  int winsize;
  int wintype;
  int chans;
  MYFLT srate;
} PVOCEX_MEMFILE;

typedef struct SNDMEMFILE_ {
  /** file ID (short name)          */
  char *name;
  struct SNDMEMFILE_ *nxt;
  /** full path filename            */
  char *fullName;
  /** file length in sample frames  */
  size_t nFrames;
  /** sample rate in Hz             */
  double sampleRate;
  /** number of channels            */
  int nChannels;
  /** AE_SHORT, AE_FLOAT, etc.      */
  int sampleFormat;
  /** TYP_WAV, TYP_AIFF, etc.       */
  int fileType;
  /**
   * loop mode:
   *   0: no loop information
   *   1: off
   *   2: forward
   *   3: backward
   *   4: bidirectional
   */
  int loopMode;
  /** playback start offset frames  */
  double startOffs;
  /** loop start (sample frames)    */
  double loopStart;
  /** loop end (sample frames)      */
  double loopEnd;
  /** base frequency (in Hz)        */
  double baseFreq;
  /** amplitude scale factor        */
  double scaleFac;
  /** interleaved sample data       */
  MYFLT data[1];
} SNDMEMFILE;

PUBLIC int PVOCEX_LoadFile(CSOUND *, const char *fname, PVOCEX_MEMFILE *p);

PUBLIC SNDMEMFILE *csoundLoadSoundFile(CSOUND *, const char *name,
                                       void *sfinfo);

PUBLIC MEMFIL *ldmemfile2withCB(CSOUND *csound, const char *filnam,
                                int csFileType,
                                int (*callback)(CSOUND *, MEMFIL *));

#ifdef __cplusplus
}
#endif /* __cplusplus */
#pragma once

#include "csoundCore_internal.h"

#define STDINASSIGN_SNDFILE     1
#define STDINASSIGN_LINEIN      2
#define STDINASSIGN_MIDIFILE    4
#define STDINASSIGN_MIDIDEV     8

#define STDOUTASSIGN_SNDFILE    1
#define STDOUTASSIGN_MIDIOUT    2

typedef struct {
    char    *format;
    int     type;
} SOUNDFILE_TYPE_ENTRY;


typedef struct  {
    char    *longformat;
    char    shortformat;
} SAMPLE_FORMAT_ENTRY;

extern const SOUNDFILE_TYPE_ENTRY file_type_map[];

extern const SAMPLE_FORMAT_ENTRY sample_format_map[];

inline void set_stdout_assign(CSOUND *csound, int type, int state)
{
    if (state)
      csound->stdout_assign_flg |= type;
    else
      csound->stdout_assign_flg &= (~type);
}
inline void set_stdin_assign(CSOUND *csound, int type, int state)
{
    if (state)
      csound->stdin_assign_flg |= type;
    else
      csound->stdin_assign_flg &= (~type);
}
void set_output_format(OPARMS *O, char c);

PUBLIC int     argdecode(CSOUND *, int, const char **);
CS_NORETURN void dieu(CSOUND *csound, char *s, ...);
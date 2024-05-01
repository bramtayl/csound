/*
    prototyp.h:

    Copyright (C) 1991-2005 Barry Vercoe, John ffitch

    This file is part of Csound.

    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301 USA
*/
/*  PROTOTYP.H  */
#if !defined(_CSOUND_PROTO_H)
#define _CSOUND_PROTO_H
#include <sysdep.h>
#ifdef __cplusplus
extern "C" {
#endif

void    cscore_(CSOUND *);
void    *mmallocDebug(CSOUND *, size_t, char*, int);
void    *mcallocDebug(CSOUND *, size_t, char*, int);
void    *mreallocDebug(CSOUND *, void *, size_t, char*, int);
void    mfreeDebug(CSOUND *, void *, char*, int);
char    *cs_strndup(CSOUND*, char*, size_t);
CS_PRINTF2  void    synterr(CSOUND *, const char *, ...);
void    csoundErrorMsgS(CSOUND *, int attr, const char *, ...);
TEXT    *getoptxt(CSOUND *, int *);
void    reverbinit(CSOUND *);
int     init0(CSOUND *);
void    scsort(CSOUND *, FILE *, FILE *);
char    *scsortstr(CSOUND *, CORFIL *);
int     scxtract(CSOUND *, CORFIL *, FILE *);
int     rdscor(CSOUND *, EVTBLK *);
int     musmon(CSOUND *);
void    list_opcodes(CSOUND *, int);
#if 0
int     readOptions_file(CSOUND *, FILE *, int);
#else
int     readOptions(CSOUND *, CORFIL *, int);
#endif
PUBLIC int     argdecode(CSOUND *, int, const char **);
void    remove_tmpfiles(CSOUND *);
void    add_tmpfile(CSOUND *, char *);
void    xturnoff(CSOUND *, INSDS *);
void    xturnoff_now(CSOUND *, INSDS *);
//MEMFIL  *ldmemfile(CSOUND *, const char *);
//MEMFIL  *ldmemfile2(CSOUND *, const char *, int);
void    rlsmemfiles(CSOUND *);
int     delete_memfile(CSOUND *, const char *);
char    *csoundTmpFileName(CSOUND *, const char *);
void    dbfs_init(CSOUND *, MYFLT dbfs);
int     csoundLoadExternals(CSOUND *);
void    print_opcodedir_warning(CSOUND *);
int     check_rtaudio_name(char *fName, char **devName, int isOutput);
int     csoundLoadOpcodeDB(CSOUND *, const char *);
void    csoundDestroyOpcodeDB(CSOUND *);
int     csoundCheckOpcodePluginFile(CSOUND *, const char *);
//int     csoundLoadAllPluginOpcodes(CSOUND *);
int     csoundLoadAndInitModule(CSOUND *, const char *);

/**
 * Returns a binary value of which bit 0 is set if the first input
 * argument is a string, bit 1 is set if the second input argument is
 * a string, and so on.
 * Only the first 31 arguments are guaranteed to be reported correctly.
 */
unsigned long csoundGetInputArgSMask(void *p);

/**
 * Returns a binary value of which bit 0 is set if the first output
 * argument is a string, bit 1 is set if the second output argument is
 * a string, and so on.
 * Only the first 31 arguments are guaranteed to be reported correctly.
 */
unsigned long csoundGetOutputArgSMask(void *p);

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

#ifdef __cplusplus
}
#endif

#endif  /* !_CSOUND_PROTO_H */

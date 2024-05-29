#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compile the given TREE node into structs for Csound to use
 * this can be called during performance to compile a new TREE
 */
PUBLIC int csoundCompileTree(CSOUND *csound, TREE *root);

/**
 * Parse, and compile the given orchestra from an ASCII string,
 * also evaluating any global space code (i-time only)
 * this can be called during performance to compile a new orchestra.
 * /code
 *       char *orc = "instr 1 \n a1 rand 0dbfs/4 \n out a1 \n";
 *       csoundCompileOrc(csound, orc);
 * /endcode
 */
PUBLIC int csoundCompileOrc(CSOUND *csound, const char *str);

/**
 *  Async version of csoundCompileOrc(). The code is parsed and
 *  compiled, then placed on a queue for
 *  asynchronous merge into the running engine, and evaluation.
 *  The function returns following parsing and compilation.
 */
PUBLIC int csoundCompileOrcAsync(CSOUND *csound, const char *str);

/**
 *   Parse and compile an orchestra given on an string,
 *   evaluating any global space code (i-time only).
 *   On SUCCESS it returns a value passed to the
 *   'return' opcode in global space
 * /code
 *       char *code = "i1 = 2 + 2 \n return i1 \n";
 *       MYFLT retval = csoundEvalCode(csound, code);
 * /endcode
 */
PUBLIC MYFLT csoundEvalCode(CSOUND *csound, const char *str);

/**
 *  Read, preprocess, and load a score from an ASCII string
 *  It can be called repeatedly, with the new score events
 *  being added to the currently scheduled ones.
 */
PUBLIC int csoundReadScore(CSOUND *csound, const char *str);

/**
 *  Asynchronous version of csoundReadScore().
 */
PUBLIC void csoundReadScoreAsync(CSOUND *csound, const char *str);

/**
 * retrieves the value of control channel identified by *name.
 * If the err argument is not NULL, the error (or success) code
 * finding or accessing the channel is stored in it.
 */
PUBLIC MYFLT csoundGetControlChannel(CSOUND *csound, const char *name,
                                     int *err);

/**
 * sets the value of control channel identified by *name
 */
PUBLIC void csoundSetControlChannel(CSOUND *csound, const char *name,
                                    MYFLT val);

/**
 * copies the audio channel identified by *name into array
 * *samples which should contain enough memory for ksmps MYFLTs
 */
PUBLIC void csoundGetAudioChannel(CSOUND *csound, const char *name,
                                  MYFLT *samples);

/**
 * sets the audio channel identified by *name with data from array
 * *samples which should contain at least ksmps MYFLTs
 */
PUBLIC void csoundSetAudioChannel(CSOUND *csound, const char *name,
                                  MYFLT *samples);

/**
 * copies the string channel identified by *name into *string
 * which should contain enough memory for the string
 * (see csoundGetChannelDatasize() below)
 */
PUBLIC void csoundGetStringChannel(CSOUND *csound, const char *name,
                                   char *string);

/**
 * sets the string channel identified by *name with *string
 */
PUBLIC void csoundSetStringChannel(CSOUND *csound, const char *name,
                                   char *string);

/**
 * Sends a PVSDATEX fin to the pvsin opcode (f-rate) for channel 'name'.
 * Returns zero on success, CSOUND_ERROR if the index is invalid or
 * fsig framesizes are incompatible.
 * CSOUND_MEMORY if there is not enough memory to extend the bus.
 */
PUBLIC int csoundSetPvsChannel(CSOUND *, const PVSDATEXT *fin,
                               const char *name);

/**
 * Receives a PVSDAT fout from the pvsout opcode (f-rate) at channel 'name'
 * Returns zero on success, CSOUND_ERROR if the index is invalid or
 * if fsig framesizes are incompatible.
 * CSOUND_MEMORY if there is not enough memory to extend the bus
 */
PUBLIC int csoundGetPvsChannel(CSOUND *csound, PVSDATEXT *fout,
                               const char *name);

/**
 * Send a new score event. 'type' is the score event type ('a', 'i', 'q',
 * 'f', or 'e').
 * 'numFields' is the size of the pFields array.  'pFields' is an array of
 * floats with all the pfields for this event, starting with the p1 value
 * specified in pFields[0].
 */
PUBLIC int csoundScoreEvent(CSOUND *, char type, const MYFLT *pFields,
                            long numFields);

/**
 *  Asynchronous version of csoundScoreEvent().
 */
PUBLIC void csoundScoreEventAsync(CSOUND *, char type, const MYFLT *pFields,
                                  long numFields);

/**
 * Like csoundScoreEvent(), this function inserts a score event, but
 * at absolute time with respect to the start of performance, or from an
 * offset set with time_ofs
 */
PUBLIC int csoundScoreEventAbsolute(CSOUND *, char type, const MYFLT *pfields,
                                    long numFields, double time_ofs);

/**
 *  Asynchronous version of csoundScoreEventAbsolute().
 */
PUBLIC void csoundScoreEventAbsoluteAsync(CSOUND *, char type,
                                          const MYFLT *pfields, long numFields,
                                          double time_ofs);
/**
 * Input a NULL-terminated string (as if from a console),
 * used for line events.
 */
PUBLIC void csoundInputMessage(CSOUND *, const char *message);

/**
 * Asynchronous version of csoundInputMessage().
 */
PUBLIC void csoundInputMessageAsync(CSOUND *, const char *message);

/**
 * Kills off one or more running instances of an instrument identified
 * by instr (number) or instrName (name). If instrName is NULL, the
 * instrument number is used.
 * Mode is a sum of the following values:
 * 0,1,2: kill all instances (1), oldest only (1), or newest (2)
 * 4: only turnoff notes with exactly matching (fractional) instr number
 * 8: only turnoff notes with indefinite duration (p3 < 0 or MIDI)
 * allow_release, if non-zero, the killed instances are allowed to release.
 */
PUBLIC int csoundKillInstance(CSOUND *csound, MYFLT instr, char *instrName,
                              int mode, int allow_release);

/**
 * Sets the value of a slot in a function table.
 * The table number and index are assumed to be valid.
 */
PUBLIC void csoundTableSet(CSOUND *, int table, int index, MYFLT value);

/**
 * Copy the contents of a function table into a supplied array *dest
 * The table number is assumed to be valid, and the destination needs to
 * have sufficient space to receive all the function table contents.
 */
PUBLIC void csoundTableCopyOut(CSOUND *csound, int table, MYFLT *dest);

/**
 * Asynchronous version of csoundTableCopyOut()
 */
PUBLIC void csoundTableCopyOutAsync(CSOUND *csound, int table, MYFLT *dest);
/**
 * Copy the contents of an array *src into a given function table
 * The table number is assumed to be valid, and the table needs to
 * have sufficient space to receive all the array contents.
 */
PUBLIC void csoundTableCopyIn(CSOUND *csound, int table, MYFLT *src);

/**
 * Asynchronous version of csoundTableCopyIn()
 */
PUBLIC void csoundTableCopyInAsync(CSOUND *csound, int table, MYFLT *src);

/**
 * Asynchronous version of csoundCompileTree()
 */
PUBLIC int csoundCompileTreeAsync(CSOUND *csound, TREE *root);

#ifdef __cplusplus
}
#endif
#pragma once

#ifdef __cplusplus
extern "C" {
#endif /*  __cplusplus */

#include "csoundCore.h"

PUBLIC int insert_score_event(CSOUND *, EVTBLK *, double);

PUBLIC int insert_score_event_at_sample(CSOUND *, EVTBLK *, int64_t);

/**
 * Prints information about the end of a performance, and closes audio
 * and MIDI devices.
 * Note: after calling csoundCleanup(), the operation of the perform
 * functions is undefined.
 */
PUBLIC int csoundCleanup(CSOUND *);

/**
 * Register a function to be called once in every control period
 * by sensevents(). Any number of functions may be registered,
 * and will be called in the order of registration.
 * The callback function takes two arguments: the Csound instance
 * pointer, and the userData pointer as passed to this function.
 * This facility can be used to ensure a function is called synchronously
 * before every csound control buffer processing. It is important
 * to make sure no blocking operations are performed in the callback.
 * The callbacks are cleared on csoundCleanup().
 * Returns zero on success.
 */
PUBLIC int csoundRegisterSenseEventCallback(CSOUND *,
                                            void (*func)(CSOUND *, void *),
                                            void *userData);

#ifdef __cplusplus
}
#endif /* __cplusplus */
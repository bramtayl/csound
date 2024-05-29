#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  Read arguments, parse and compile an orchestra, read, process and
 *  load a score.
 */
PUBLIC int csoundCompileArgs(CSOUND *, int argc, const char **argv);

/**
 * Prepares Csound for performance. Normally called after compiling
 * a csd file or an orc file, in which case score preprocessing is
 * performed and performance terminates when the score terminates.
 *
 * However, if called before compiling a csd file or an orc file,
 * score preprocessing is not performed and "i" statements are dispatched
 * as real-time events, the <CsOptions> tag is ignored, and performance
 * continues indefinitely or until ended using the API.
 */
PUBLIC int csoundStart(CSOUND *csound);

/**
 * Compiles Csound input files (such as an orchestra and score, or CSD)
 * as directed by the supplied command-line arguments,
 * but does not perform them. Returns a non-zero error code on failure.
 * This function cannot be called during performance, and before a
 * repeated call, csoundReset() needs to be called.
 * In this (host-driven) mode, the sequence of calls should be as follows:
 * /code
 *       csoundCompile(csound, argc, argv);
 *       while (!csoundPerformBuffer(csound));
 *       csoundCleanup(csound);
 *       csoundReset(csound);
 * /endcode
 *  Calls csoundStart() internally.
 *  Can only be called again after reset (see csoundReset())
 */
PUBLIC int csoundCompile(CSOUND *, int argc, const char **argv);

/**
 * Compiles a Csound input file (CSD, .csd file), but does not perform it.
 * Returns a non-zero error code on failure.
 *
 * If csoundStart is called before csoundCompileCsd, the <CsOptions>
 * element is ignored (but csoundSetOption can be called any number of
 * times), the <CsScore> element is not pre-processed, but dispatched as
 * real-time events; and performance continues indefinitely, or until
 * ended by calling csoundStop or some other logic. In this "real-time"
 * mode, the sequence of calls should be:
 *
 * \code
 *
 * csoundSetOption("-an_option");
 * csoundSetOption("-another_option");
 * csoundStart(csound);
 * csoundCompileCsd(csound, csd_filename);
 * while (1) {
 *    csoundPerformBuffer(csound);
 *    // Something to break out of the loop
 *    // when finished here...
 * }
 * csoundCleanup(csound);
 * csoundReset(csound);
 *
 * \endcode
 *
 * NB: this function can be called repeatedly during performance to
 * replace or add new instruments and events.
 *
 * But if csoundCompileCsd is called before csoundStart, the <CsOptions>
 * element is used, the <CsScore> section is pre-processed and dispatched
 * normally, and performance terminates when the score terminates, or
 * csoundStop is called. In this "non-real-time" mode (which can still
 * output real-time audio and handle real-time events), the sequence of
 * calls should be:
 *
 * \code
 *
 * csoundCompileCsd(csound, csd_filename);
 * csoundStart(csound);
 * while (1) {
 *    int finished = csoundPerformBuffer(csound);
 *    if (finished) break;
 * }
 * csoundCleanup(csound);
 * csoundReset(csound);
 *
 * \endcode
 *
 */
PUBLIC int csoundCompileCsd(CSOUND *csound, const char *csd_filename);

/**
 * Behaves the same way as csoundCompileCsd, except that the content
 * of the CSD is read from the csd_text string rather than from a file.
 * This is convenient when it is desirable to package the csd as part of
 * an application or a multi-language piece.
 */
PUBLIC int csoundCompileCsdText(CSOUND *csound, const char *csd_text);

#ifdef __cplusplus
}
#endif
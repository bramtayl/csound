#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Stores a pointer to the specified channel of the bus in *p,
* creating the channel first if it does not exist yet.
* 'type' must be the bitwise OR of exactly one of the following values,
*   CSOUND_CONTROL_CHANNEL
*     control data (one MYFLT value)
*   CSOUND_AUDIO_CHANNEL
*     audio data (csoundGetKsmps(csound) MYFLT values)
*   CSOUND_STRING_CHANNEL
*     string data (MYFLT values with enough space to store
*     csoundGetChannelDatasize() characters, including the
*     NULL character at the end of the string)
* and at least one of these:
*   CSOUND_INPUT_CHANNEL
*   CSOUND_OUTPUT_CHANNEL
* If the channel already exists, it must match the data type
* (control, audio, or string), however, the input/output bits are
* OR'd with the new value. Note that audio and string channels
* can only be created after calling csoundCompile(), because the
* storage size is not known until then.

* Return value is zero on success, or a negative error code,
*   CSOUND_MEMORY  there is not enough memory for allocating the channel
*   CSOUND_ERROR   the specified name or type is invalid
* or, if a channel with the same name but incompatible type
* already exists, the type of the existing channel. In the case
* of any non-zero return value, *p is set to NULL.
* Note: to find out the type of a channel without actually
* creating or changing it, set 'type' to zero, so that the return
* value will be either the type of the channel, or CSOUND_ERROR
* if it does not exist.
*
* Operations on **p are not thread-safe by default. The host is required
* to take care of threadsafety by
* 1) with control channels use __atomic_load() or
*    __atomic_store() gcc atomic builtins to get or set a channel,
*    if available.
* 2) For string and audio channels (and controls if option 1 is not
*    available), retrieve the channel lock with csoundGetChannelLock()
*    and use csoundSpinLock() and csoundSpinUnLock() to protect access
*    to **p.
* See Top/threadsafe.c in the Csound library sources for
* examples.  Optionally, use the channel get/set functions
* provided below, which are threadsafe by default.
*/
PUBLIC int csoundGetChannelPtr(CSOUND *, MYFLT **p, const char *name, int type);

/**
 * Returns a list of allocated channels in *lst. A controlChannelInfo_t
 * structure contains the channel characteristics.
 * The return value is the number of channels, which may be zero if there
 * are none, or CSOUND_MEMORY if there is not enough memory for allocating
 * the list. In the case of no channels or an error, *lst is set to NULL.
 * Notes: the caller is responsible for freeing the list returned in *lst
 * with csoundDeleteChannelList(). The name pointers may become invalid
 * after calling csoundReset().
 */
PUBLIC int csoundListChannels(CSOUND *, controlChannelInfo_t **lst);

/**
 * Releases a channel list previously returned by csoundListChannels().
 */
PUBLIC void csoundDeleteChannelList(CSOUND *, controlChannelInfo_t *lst);

/**
 * Set parameters hints for a control channel. These hints have no internal
 * function but can be used by front ends to construct GUIs or to constrain
 * values. See the controlChannelHints_t structure for details.
 * Returns zero on success, or a non-zero error code on failure:
 *   CSOUND_ERROR:  the channel does not exist, is not a control channel,
 *                  or the specified parameters are invalid
 *   CSOUND_MEMORY: could not allocate memory
 */
PUBLIC int csoundSetControlChannelHints(CSOUND *, const char *name,
                                        controlChannelHints_t hints);

/**
 * Returns special parameters (assuming there are any) of a control channel,
 * previously set with csoundSetControlChannelHints() or the chnparams
 * opcode.
 * If the channel exists, is a control channel, the channel hints
 * are stored in the preallocated controlChannelHints_t structure. The
 * attributes member of the structure will be allocated inside this function
 * so it is necessary to free it explicitly in the host.
 *
 * The return value is zero if the channel exists and is a control
 * channel, otherwise, an error code is returned.
 */
PUBLIC int csoundGetControlChannelHints(CSOUND *, const char *name,
                                        controlChannelHints_t *hints);

/**
 * Recovers a pointer to a lock for the specified channel called 'name'.
 * The returned lock can be locked/unlocked  with the csoundSpinLock()
 * and csoundSpinUnLock() functions.
 * @returns the address of the lock or NULL if the channel does not exist
 */
PUBLIC int *csoundGetChannelLock(CSOUND *, const char *name);

/**
 * returns the size of data stored in a channel; for string channels
 * this might change if the channel space gets reallocated
 * Since string variables use dynamic memory allocation in Csound6,
 * this function can be called to get the space required for
 * csoundGetStringChannel()
 */
PUBLIC int csoundGetChannelDatasize(CSOUND *csound, const char *name);

#ifdef __cplusplus
}
#endif
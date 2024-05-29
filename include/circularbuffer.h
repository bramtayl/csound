#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "csound.h"

/**
 * Create circular buffer with numelem number of elements. The
 * element's size is set from elemsize. It should be used like:
 *@code
 * void *rb = csoundCreateCircularBuffer(csound, 1024, sizeof(MYFLT));
 *@endcode
 */
PUBLIC void *csoundCreateCircularBuffer(CSOUND *csound, int numelem,
                                        int elemsize);

/**
 * Read from circular buffer
 * @param csound This value is currently ignored.
 * @param circular_buffer pointer to an existing circular buffer
 * @param out preallocated buffer with at least items number of elements, where
 *              buffer contents will be read into
 * @param items number of samples to be read
 * @returns the actual number of items read (0 <= n <= items)
 */
PUBLIC int csoundReadCircularBuffer(CSOUND *csound, void *circular_buffer,
                                    void *out, int items);

/**
 * Read from circular buffer without removing them from the buffer.
 * @param circular_buffer pointer to an existing circular buffer
 * @param out preallocated buffer with at least items number of elements, where
 *              buffer contents will be read into
 * @param items number of samples to be read
 * @returns the actual number of items read (0 <= n <= items)
 */
PUBLIC int csoundPeekCircularBuffer(CSOUND *csound, void *circular_buffer,
                                    void *out, int items);

/**
 * Write to circular buffer
 * @param csound This value is currently ignored.
 * @param p pointer to an existing circular buffer
 * @param inp buffer with at least items number of elements to be written into
 *              circular buffer
 * @param items number of samples to write
 * @returns the actual number of items written (0 <= n <= items)
 */
PUBLIC int csoundWriteCircularBuffer(CSOUND *csound, void *p, const void *inp,
                                     int items);
/**
 * Empty circular buffer of any remaining data. This function should only be
 * used if there is no reader actively getting data from the buffer.
 * @param csound This value is currently ignored.
 * @param p pointer to an existing circular buffer
 */
PUBLIC void csoundFlushCircularBuffer(CSOUND *csound, void *p);

/**
 * Free circular buffer
 */
PUBLIC void csoundDestroyCircularBuffer(CSOUND *csound, void *circularbuffer);

#ifdef __cplusplus
}
#endif
#pragma once

#include "csound.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates and starts a new thread of execution.
 * Returns an opaque pointer that represents the thread on success,
 * or NULL for failure.
 * The userdata pointer is passed to the thread routine.
 */
PUBLIC void *csoundCreateThread(uintptr_t (*threadRoutine)(void *),
                                void *userdata);

/**
 * Creates and starts a new thread of execution
 * with a user-defined stack size.
 * Returns an opaque pointer that represents the thread on success,
 * or NULL for failure.
 * The userdata pointer is passed to the thread routine.
 */
PUBLIC void *csoundCreateThread2(uintptr_t (*threadRoutine)(void *),
                                 unsigned int stack, void *userdata);

/**
 * Returns the ID of the currently executing thread,
 * or NULL for failure.
 *
 * NOTE: The return value can be used as a pointer
 * to a thread object, but it should not be compared
 * as a pointer. The pointed to values should be compared,
 * and the user must free the pointer after use.
 */
PUBLIC void *csoundGetCurrentThreadId(void);

/**
 * Waits until the indicated thread's routine has finished.
 * Returns the value returned by the thread routine.
 */
PUBLIC uintptr_t csoundJoinThread(void *thread);
/**
 * Creates and returns a monitor object, or NULL if not successful.
 * The object is initially in signaled (notified) state.
 */
PUBLIC void *csoundCreateThreadLock(void);

/**
 * Waits on the indicated monitor object for the indicated period.
 * The function returns either when the monitor object is notified,
 * or when the period has elapsed, whichever is sooner; in the first case,
 * zero is returned.
 * If 'milliseconds' is zero and the object is not notified, the function
 * will return immediately with a non-zero status.
 */
PUBLIC int csoundWaitThreadLock(void *lock, size_t milliseconds);

/**
 * Waits on the indicated monitor object until it is notified.
 * This function is similar to csoundWaitThreadLock() with an infinite
 * wait time, but may be more efficient.
 */
PUBLIC void csoundWaitThreadLockNoTimeout(void *lock);

/**
 * Notifies the indicated monitor object.
 */
PUBLIC void csoundNotifyThreadLock(void *lock);

/**
 * Destroys the indicated monitor object.
 */
PUBLIC void csoundDestroyThreadLock(void *lock);

/**
 * Creates and returns a mutex object, or NULL if not successful.
 * Mutexes can be faster than the more general purpose monitor objects
 * returned by csoundCreateThreadLock() on some platforms, and can also
 * be recursive, but the result of unlocking a mutex that is owned by
 * another thread or is not locked is undefined.
 * If 'isRecursive' is non-zero, the mutex can be re-locked multiple
 * times by the same thread, requiring an equal number of unlock calls;
 * otherwise, attempting to re-lock the mutex results in undefined
 * behavior.
 * Note: the handles returned by csoundCreateThreadLock() and
 * csoundCreateMutex() are not compatible.
 */
PUBLIC void *csoundCreateMutex(int isRecursive);

/**
 * Acquires the indicated mutex object; if it is already in use by
 * another thread, the function waits until the mutex is released by
 * the other thread.
 */
PUBLIC void csoundLockMutex(void *mutex_);

/**
 * Acquires the indicated mutex object and returns zero, unless it is
 * already in use by another thread, in which case a non-zero value is
 * returned immediately, rather than waiting until the mutex becomes
 * available.
 * Note: this function may be unimplemented on Windows.
 */
PUBLIC int csoundLockMutexNoWait(void *mutex_);

/**
 * Releases the indicated mutex object, which should be owned by
 * the current thread, otherwise the operation of this function is
 * undefined. A recursive mutex needs to be unlocked as many times
 * as it was locked previously.
 */
PUBLIC void csoundUnlockMutex(void *mutex_);

/**
 * Destroys the indicated mutex object. Destroying a mutex that
 * is currently owned by a thread results in undefined behavior.
 */
PUBLIC void csoundDestroyMutex(void *mutex_);

/**
 * Create a Thread Barrier. Max value parameter should be equal to
 * number of child threads using the barrier plus one for the
 * master thread */

PUBLIC void *csoundCreateBarrier(unsigned int max);

/**
 * Destroy a Thread Barrier.
 */
PUBLIC int csoundDestroyBarrier(void *barrier);

/**
 * Wait on the thread barrier.
 */
PUBLIC int csoundWaitBarrier(void *barrier);

/** Creates a conditional variable */
PUBLIC void *csoundCreateCondVar();

/** Waits up on a conditional variable and mutex */
PUBLIC void csoundCondWait(void *condVar, void *mutex);

/** Signals a conditional variable */
PUBLIC void csoundCondSignal(void *condVar);

/** Destroys a conditional variable */
PUBLIC void csoundDestroyCondVar(void *condVar);

/**
 * Waits for at least the specified number of milliseconds,
 * yielding the CPU to other threads.
 */
PUBLIC void csoundSleep(size_t milliseconds);

/**
 * If the spinlock is not locked, lock it and return;
 * if is is locked, wait until it is unlocked, then lock it and return.
 * Uses atomic compare and swap operations that are safe across processors
 * and safe for out of order operations,
 * and which are more efficient than operating system locks.
 * Use spinlocks to protect access to shared data, especially in functions
 * that do little more than read or write such data, for example:
 *
 * @code
 * static spin_lock_t lock = SPINLOCK_INIT;
 * csoundSpinLockInit(&lock);
 * void write(size_t frames, int* signal)
 * {
 *   csoundSpinLock(&lock);
 *   for (size_t frame = 0; i < frames; frame++) {
 *     global_buffer[frame] += signal[frame];
 *   }
 *   csoundSpinUnlock(&lock);
 * }
 * @endcode
 */
PUBLIC int csoundSpinLockInit(spin_lock_t *spinlock);

/**
 * Locks the spinlock
 */
PUBLIC void csoundSpinLock(spin_lock_t *spinlock);

/**
* Tries the lock, returns CSOUND_SUCCESS if lock could be acquired,
    CSOUND_ERROR, otherwise.
*/
PUBLIC int csoundSpinTryLock(spin_lock_t *spinlock);

/**
 * Unlocks the spinlock
 */
PUBLIC void csoundSpinUnLock(spin_lock_t *spinlock);

/** @}*/
/** @defgroup MISCELLANEOUS Miscellaneous functions
 *
 *  @{ */

/**
 * Runs an external command with the arguments specified in 'argv'.
 * argv[0] is the name of the program to execute (if not a full path
 * file name, it is searched in the directories defined by the PATH
 * environment variable). The list of arguments should be terminated
 * by a NULL pointer.
 * If 'noWait' is zero, the function waits until the external program
 * finishes, otherwise it returns immediately. In the first case, a
 * non-negative return value is the exit status of the command (0 to
 * 255), otherwise it is the PID of the newly created process.
 * On error, a negative value is returned.
 */
PUBLIC long csoundRunCommand(const char *const *argv, int noWait);

#ifdef __cplusplus
}
#endif
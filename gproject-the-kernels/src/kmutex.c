/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Mutexes
 */

#include <spede/string.h>
#include "kernel.h"
#include "kmutex.h"
#include "queue.h"
#include "scheduler.h"
#include "kproc.h"
#include "spinlock.h"

static spinlock_t kmutex_splock;    // global spinlock for ID pool

// Table of all mutexes
mutex_t mutexes[MUTEX_MAX];

// Mutex ids to be allocated
queue_t    mutex_queue;

/**
 * Initializes kernel mutex data structures
 * @return -1 on error, 0 on success
 */
int kmutexes_init(void) {
    kernel_log_info("Initializing kernel mutexes");

    init_splock(&kmutex_splock);
    queue_init(&mutex_queue);

    for (int i = 0; i < MUTEX_MAX; i++) {
        mutexes[i].allocated = 0;
        mutexes[i].locks     = 0;
        mutexes[i].owner     = NULL;
        queue_init(&mutexes[i].wait_queue);

        // protect global queue of free IDs
        acquire_splock(&kmutex_splock);
        queue_in(&mutex_queue, i);
        release_splock(&kmutex_splock);
    }
    return 0;
}

/**
 * Allocates a mutex
 * @return -1 on error, otherwise the mutex id that was allocated
 */
int kmutex_init(void) {
    int id, err;
    acquire_splock(&kmutex_splock);
      err = queue_out(&mutex_queue, &id);
    release_splock(&kmutex_splock);

    if (err != 0) {
        kernel_log_error("No available mutexes");
        return -1;
    }

    mutex_t *mtx = &mutexes[id];
    acquire_splock(&kmutex_splock);
      mtx->allocated = 1;
      mtx->locks     = 0;
      mtx->owner     = NULL;
      queue_init(&mtx->wait_queue);
    release_splock(&kmutex_splock);

    return id;
}

/**
 * Frees the specified mutex
 * @param id - the mutex id
 * @return 0 on success, -1 on error
 */
int kmutex_destroy(int id) {
    if (id < 0 || id >= MUTEX_MAX) return -1;
    mutex_t *mtx = &mutexes[id];

    acquire_splock(&kmutex_splock);
      if (!mtx->allocated || mtx->locks > 0) {
        release_splock(&kmutex_splock);
        return -1;
      }
      mtx->allocated = 0;
    release_splock(&kmutex_splock);

    // return ID to global pool
    acquire_splock(&kmutex_splock);
      queue_in(&mutex_queue, id);
    release_splock(&kmutex_splock);

    return 0;
}

/**
 * Locks the specified mutex
 * @param id - the mutex id
 * @return -1 on error, otherwise the current lock count
 */
int kmutex_lock(int id) {
    if (id < 0 || id >= MUTEX_MAX) return -1;
    mutex_t *mtx = &mutexes[id];

    acquire_splock(&kmutex_splock);
      proc_t *cur = active_proc;
      if (!mtx->allocated || !cur) {
        release_splock(&kmutex_splock);
        return -1;
      }

      if (mtx->locks > 0 && mtx->owner != cur) {
        // block and enqueue
        kernel_log_info("info: blocking pid %d on mutex %d", cur->pid, id);
        cur->state = WAITING;
        queue_in(&mtx->wait_queue, cur->pid);
        release_splock(&kmutex_splock);

        scheduler_remove(cur);
        scheduler_run();
        return 0;
      }

      // acquire lock
      mtx->owner = cur;
      mtx->locks++;
    release_splock(&kmutex_splock);

    return mtx->locks;
}

/**
 * Unlocks the specified mutex
 * @param id - the mutex id
 * @return -1 on error, otherwise the current lock count
 */
int kmutex_unlock(int id) {
    if (id < 0 || id >= MUTEX_MAX) return -1;
    mutex_t *mtx = &mutexes[id];

    acquire_splock(&kmutex_splock);
      if (!mtx->allocated || mtx->locks == 0 || mtx->owner != active_proc) {
        kernel_log_error("error: invalid unlock by pid %d on mutex %d",
                         active_proc ? active_proc->pid : -1, id);
        release_splock(&kmutex_splock);
        return -1;
      }

      mtx->locks--;
      kernel_log_info("info: release lock %d, %d", mtx->locks, id);

      if (mtx->locks == 0) {
        mtx->owner = NULL;
        int next_pid;
        if (queue_out(&mtx->wait_queue, &next_pid) == 0) {
            proc_t *next = pid_to_proc(next_pid);
            if (next) {
                scheduler_add(next);
                mtx->owner = next;
                mtx->locks = 1;
                kernel_log_info("info: passed lock to pid %d", next_pid);
            }
        }
      }
    release_splock(&kmutex_splock);

    return mtx->locks;
}


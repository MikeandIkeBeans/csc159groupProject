/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Semaphores
 */

#include <spede/string.h>
#include "kernel.h"
#include "ksem.h"
#include "queue.h"
#include "scheduler.h"
#include "kproc.h"
#include "spinlock.h"            // <-- new

static spinlock_t ksem_splock;    // protects sem_queue
static spinlock_t sem_splocks[SEM_MAX];  // per‐semaphore locks

sem_t semaphores[SEM_MAX];
queue_t sem_queue;

/**
 * Initializes kernel semaphore data structures
 */
int ksemaphores_init(void) {
    kernel_log_info("Initializing kernel semaphores");
    init_splock(&ksem_splock);
    queue_init(&sem_queue);

    for (int i = 0; i < SEM_MAX; i++) {
        init_splock(&sem_splocks[i]);         // init per‐sem lock
        semaphores[i].allocated = 0;
        semaphores[i].count     = 0;
        queue_init(&semaphores[i].wait_queue);

        // protect global queue
        acquire_splock(&ksem_splock);
          queue_in(&sem_queue, i);
        release_splock(&ksem_splock);
    }
    return 0;
}

/**
 * Allocates a semaphore
 */
int ksem_init(int value) {
    int id, err;

    acquire_splock(&ksem_splock);
      err = queue_out(&sem_queue, &id);
    release_splock(&ksem_splock);

    if (err != 0) {
        kernel_log_error("No available semaphores");
        return -1;
    }

    // initialize the new semaphore
    acquire_splock(&sem_splocks[id]);
      semaphores[id].allocated = 1;
      semaphores[id].count     = value;
      queue_init(&semaphores[id].wait_queue);
    release_splock(&sem_splocks[id]);

    return id;
}

/**
 * Frees the specified semaphore
 */
int ksem_destroy(int id) {
    if (id < 0 || id >= SEM_MAX) return -1;

    acquire_splock(&sem_splocks[id]);
      if (!semaphores[id].allocated 
          || !queue_is_empty(&semaphores[id].wait_queue)) {
        release_splock(&sem_splocks[id]);
        return -1;
      }
      semaphores[id].allocated = 0;
      semaphores[id].count     = 0;
      queue_init(&semaphores[id].wait_queue);
    release_splock(&sem_splocks[id]);

    // return ID to global pool
    acquire_splock(&ksem_splock);
      queue_in(&sem_queue, id);
    release_splock(&ksem_splock);

    return 0;
}

/**
 * Waits on the specified semaphore if it is held
 */
int ksem_wait(int id) {
    if (id < 0 || id >= SEM_MAX) return -1;

    acquire_splock(&sem_splocks[id]);
      if (!semaphores[id].allocated) {
        release_splock(&sem_splocks[id]);
        return -1;
      }

      kernel_log_info("info: Enter ksem_wait, semaphore id %d", id);

      if (semaphores[id].count == 0) {
        kernel_log_info("info: Waiting semaphore %d", id);
        active_proc->state = WAITING;
        queue_in(&semaphores[id].wait_queue, active_proc->pid);
        release_splock(&sem_splocks[id]);

        scheduler_run();  // yield CPU
        return semaphores[id].count;
      }

      // consume one count
      semaphores[id].count--;
    release_splock(&sem_splocks[id]);

    return semaphores[id].count;
}

/**
 * Posts the specified semaphore
 */
int ksem_post(int id) {
    if (id < 0 || id >= SEM_MAX) return -1;

    acquire_splock(&sem_splocks[id]);
      if (!semaphores[id].allocated) {
        release_splock(&sem_splocks[id]);
        return -1;
      }

      semaphores[id].count++;
      if (!queue_is_empty(&semaphores[id].wait_queue)) {
        int pid;
        queue_out(&semaphores[id].wait_queue, &pid);
        proc_t *p = pid_to_proc(pid);
        if (p) {
          scheduler_add(p);
          semaphores[id].count--;
        }
      }
      int c = semaphores[id].count;
    release_splock(&sem_splocks[id]);

    return c;
}

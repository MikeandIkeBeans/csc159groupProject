/**
 * CPE/CSC 159 - Operating System Pragmatics
 * California State University, Sacramento
 *
 * Kernel Spinlock 
 */

#include <spede/string.h>
#include "spinlock.h"
#include "kernel.h"
#include "interrupts.h"

#define FL_IF           0x00000200      // Interrupt Enable

cpu_t cpu;

static inline int xchg(volatile int *addr, int newval){
    size_t result;
    asm volatile(
            "lock; xchgl %0, %1" :
            "+m" (*addr), "=a" (result) :
            "1" (newval):
            "cc");
    return result;
}

static inline void
cli(void)
{
  asm volatile("cli");
}

static inline void
sti(void)
{
  asm volatile("sti");
}

static inline unsigned int 
readeflags(void)
{
  unsigned int eflags;
  asm volatile("pushf; pop %0" : "=r" (eflags));
  return eflags;
}

void popcli();
void pushcli();
/**
 * Initializes kernel spinlock data structures
 * @return -1 on error, 0 on success
 */
void init_splock(spinlock_t *lk){
    lk->locked = 0;
}

/**
 * acquire the lock
 */
void acquire_splock(spinlock_t *lk) {
    pushcli();  // disable interrupts to avoid deadlock
    while(xchg(&lk->locked, 1) != 0);
}

/**
 * release the lock
 */
void release_splock(spinlock_t *lk) {
    xchg(&lk->locked, 0);

    popcli(); //interrupts_enable();
}

void
pushcli(void)
{
    int eflags;
    eflags = readeflags();
    cli();
    if (cpu.ncli == 0)
        cpu.intena = eflags & FL_IF;
    cpu.ncli++;
}

void
popcli(void)
{
    if (readeflags() & FL_IF){
        kernel_log_error("popcli - interruptible");
        return;
    }
    cpu.ncli--;
    if (cpu.ncli < 0){
        kernel_log_error("popcli");
        return; 
    }

    if (cpu.ncli == 0 && cpu.intena)
        sti();
}

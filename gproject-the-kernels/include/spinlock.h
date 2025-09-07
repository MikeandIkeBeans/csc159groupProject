#ifndef KSPINLOCK_H
#define KSPINLOCK_H

typedef struct spinlock_t {
    volatile int locked;  // Is the lock held?
}spinlock_t;

typedef struct cpu_t {                                                                                                   
	int ncli;       // count of cli                                                                                                             
     	int intena;     // interrupt status                                                                                                         
}cpu_t;

void init_splock(spinlock_t *lk);
void acquire_splock(spinlock_t *lk);
void release_splock(spinlock_t *lk);

#endif

#ifndef _DEVLIB_LOCK_H
#define _DEVLIB_LOCK_H
#include <pthread.h>

#ifdef __USE_XOPEN2K 
#define dlock_t pthread_spinlock_t
#define dlock_lock pthread_mutex_lock
#define dlock_trylock pthread_mutex_trylock
#define dlock_unlock pthread_mutex_unlock
#define dlock_destroy pthread_mutex_destroy
#else
#define dlock_t pthread_mutex_t
#define dlock_lock pthread_spin_lock
#define dlock_trylock pthread_spin_trylock
#define dlock_unlock pthread_spin_unlock
#define dlock_destroy pthread_spin_destroy
#endif

int dlock_init(dlock_t* lock);
#endif

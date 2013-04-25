#include "dlock.h"

int dlock_init(dlock_t* lock)
{
#ifdef __USE_XOPEN2K
    return pthread_spin_init(lock, 0);
#else
    return pthread_mutex_init(lock, NULL);
#endif
}

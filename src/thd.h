
#ifndef THD_H_INCLUDED
#define THD_H_INCLUDED

#ifdef USE_TREADS
#include <pthread.h>
#define THREAD_LOCK(x) pthread_mutex_lock(x)
#define THREAD_UNLOCK(x) pthread_mutex_unlock(x)
#define THREAD_INIT(x,y) pthread_mutex_init(x,y)
#define THREAD_LOCK_DESTROY(x) pthread_mutex_destroy(x)
#else
#define THREAD_LOCK(x) 
#define THREAD_UNLOCK(x) 
#define THREAD_INIT(x,y)
#define THREAD_LOCK_DESTROY(x)
#endif

#endif

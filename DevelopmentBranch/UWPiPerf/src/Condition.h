/*---------------------------------------------------------------
 * An abstract class for waiting on a condition variable. If
 * threads are not available, this does nothing.
 * ------------------------------------------------------------------- */

#ifndef CONDITION_H
#define CONDITION_H

#include "headers.h"
#include "Mutex.h"
#include "util.h"

#define Condition_Lock( Cond ) Mutex_Lock( &Cond.mMutex )
#define Condition_Unlock( Cond ) Mutex_Unlock( &Cond.mMutex )

#if   defined( HAVE_POSIX_THREAD )

typedef struct Condition {
    pthread_cond_t mCondition;
    pthread_mutex_t mMutex;
} Condition;

#    define Condition_Initialize( Cond ) do {             \
    Mutex_Initialize( &(Cond)->mMutex );              \
    pthread_cond_init( &(Cond)->mCondition, NULL );   \
} while ( 0 )
#    define Condition_Destroy( Cond ) do {            \
    pthread_cond_destroy( &(Cond)->mCondition );  \
    Mutex_Destroy( &(Cond)->mMutex );             \
} while ( 0 )
#    define Condition_Wait( Cond ) pthread_cond_wait( &(Cond)->mCondition, &(Cond)->mMutex )
#    define Condition_Wait_Event( Cond ) do { \
    Mutex_Lock( &(Cond)->mMutex ); \
    pthread_cond_wait( &(Cond)->mCondition, &(Cond)->mMutex ); \
    Mutex_Unlock( &(Cond)->mMutex ); \
} while( 0 )
// sleep this thread, waiting for condition signal,
// but bound sleep time by the relative time inSeconds.
#    define Condition_TimedWait( Cond, inSeconds ) do {                         \
    struct timespec absTimeout;                                             \
    absTimeout.tv_sec  = time( NULL ) + inSeconds;                          \
    absTimeout.tv_nsec = 0;                                                 \
   pthread_cond_timedwait( &(Cond)->mCondition, &(Cond)->mMutex, &absTimeout ); \
} while ( 0 )
#    define Condition_Signal( Cond ) pthread_cond_signal( &(Cond)->mCondition )
#    define Condition_Broadcast( Cond ) pthread_cond_broadcast( &(Cond)->mCondition )

#elif defined( HAVE_WIN32_THREAD )

typedef struct Condition {
    HANDLE mCondition;
    HANDLE mMutex;
} Condition;

// set all conditions to be broadcast
// unfortunately in Win32 you have to know at creation
// whether the signal is broadcast or not.
#    define Condition_Initialize( Cond ) do {                         \
    Mutex_Initialize( &(Cond)->mMutex );                          \
    (Cond)->mCondition = CreateEvent( NULL, true, false, NULL );  \
} while ( 0 )
#    define Condition_Destroy( Cond ) do {            \
    CloseHandle( (Cond)->mCondition );            \
    Mutex_Destroy( &(Cond)->mMutex );             \
} while ( 0 )
// atomically release mutex and wait on condition,
// then re-acquire the mutex
/* Change SignalOjectAdnWait to (Mutex_Unlock and WaitForSingleObject) temp because build error. */
#    define Condition_Wait( Cond ) do {                                         \
    Mutex_Unlock(&(Cond)->mMutex);                                                    \
    WaitForSingleObject((Cond)->mCondition, INFINITE);                           \
    /*SignalObjectAndWait( (Cond)->mMutex, (Cond)->mCondition, INFINITE, false );*/ \
    Mutex_Lock( &(Cond)->mMutex );                          \
} while ( 0 )

/* Change SignalOjectAdnWait to (Mutex_Unlock and WaitForSingleObject) temp because build error. */
#    define Condition_Wait_Event( Cond ) do { \
    Mutex_Lock( &(Cond)->mMutex );                          \
    Mutex_Unlock(&(Cond)->mMutex);                                                    \
    WaitForSingleObject((Cond)->mCondition, INFINITE);                           \
    /*SignalObjectAndWait( (Cond)->mMutex, (Cond)->mCondition, INFINITE, false );*/ \
} while ( 0 )
// atomically release mutex and wait on condition,
// then re-acquire the mutex
#    define Condition_TimedWait( Cond, inSeconds ) do {                         \
    SignalObjectAndWait( (Cond)->mMutex, (Cond)->mCondition, inSeconds*1000, false ); \
    Mutex_Lock( &(Cond)->mMutex );                          \
} while ( 0 )
// send a condition signal to wake one thread waiting on condition
// in Win32, this actually wakes up all threads, same as Broadcast
// use PulseEvent to auto-reset the signal after waking all threads
/* Change PulseEvent to SetEvent temp because build error. At last, should use condition variables */
#    define Condition_Signal( Cond ) SetEvent( (Cond)->mCondition )
#    define Condition_Broadcast( Cond ) SetEvent( (Cond)->mCondition )

#else

typedef struct Condition {
    int mCondition;
    int mMutex;
} Condition;

#    define Condition_Initialize( Cond )
#    define Condition_Destroy( Cond )
#    define Condition_Wait( Cond )
#    define Condition_TimedWait( Cond, inSeconds )
#    define Condition_Signal( Cond )
#    define Condition_Broadcast( Cond )

#endif


#endif // CONDITION_H

/*---------------------------------------------------------------
 * An abstract class for locking a mutex (mutual exclusion). If
 * threads are not available, this does nothing.
 * ------------------------------------------------------------------- */

#ifndef MUTEX_H
#define MUTEX_H

#include "headers.h"

#if   defined( HAVE_POSIX_THREAD )

typedef pthread_mutex_t Mutex;
#    define Mutex_Initialize( MutexPtr ) pthread_mutex_init( MutexPtr, NULL )
#    define Mutex_Lock( MutexPtr ) pthread_mutex_lock( MutexPtr )
#    define Mutex_Unlock( MutexPtr ) pthread_mutex_unlock( MutexPtr )
#    define Mutex_Destroy( MutexPtr )  do {         \
    int rc = pthread_mutex_destroy( MutexPtr ); \
    if ( rc == EBUSY ) {                        \
        Mutex_Unlock( MutexPtr );               \
        pthread_mutex_destroy( MutexPtr );      \
    }                                           \
} while ( 0 )

#elif defined( HAVE_WIN32_THREAD )

typedef HANDLE Mutex;
#    define Mutex_Initialize( MutexPtr ) *MutexPtr = CreateMutex( NULL, false, NULL )
#    define Mutex_Lock( MutexPtr ) WaitForSingleObject( *MutexPtr, INFINITE )
#    define Mutex_Unlock( MutexPtr ) ReleaseMutex( *MutexPtr )
#    define Mutex_Destroy( MutexPtr ) CloseHandle( *MutexPtr )

#else

typedef int Mutex;
#    define Mutex_Initialize( MutexPtr )
#    define Mutex_Lock( MutexPtr )
#    define Mutex_Unlock( MutexPtr )
#    define Mutex_Destroy( MutexPtr )

#endif

#endif // MUTEX_H

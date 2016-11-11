/*---------------------------------------------------------------
 * The thread subsystem is responsible for all thread functions. It
 * provides a thread implementation agnostic interface to Iperf. If
 * threads are not available (HAVE_THREAD is undefined), thread_start
 * does not start a new thread but just launches the specified object
 * in the current thread. Everything that defines a thread of
 * execution in Iperf is contained in an thread_Settings structure. To
 * start a thread simply pass one such structure into thread_start.
 * ------------------------------------------------------------------- */

#ifndef THREAD_H
#define THREAD_H

#if   defined( HAVE_POSIX_THREAD )

/* Definitions for Posix Threads (pthreads) */
    #include <pthread.h>

typedef pthread_t nthread_t;

    #define HAVE_THREAD 1

#elif defined( HAVE_WIN32_THREAD )

/* Definitions for Win32 NT Threads */
typedef DWORD nthread_t;

    #define HAVE_THREAD 1

#else

/* Definitions for no threads */
typedef int nthread_t;

    #undef HAVE_THREAD

#endif

    // Forward declaration
    struct thread_Settings;

#include "Condition.h"
#include "Settings.h"

    // initialize or destroy the thread subsystem
    void thread_init( );
    void thread_destroy( );

    // start or stop a thread executing
    void thread_start( struct thread_Settings* thread );
    void thread_stop( struct thread_Settings* thread );

    /* wait for this or all threads to complete */
    void thread_joinall( void );

    int thread_numuserthreads( void );

    // set a thread to be ignorable, so joinall won't wait on it
    void thread_setignore( void );
    void thread_unsetignore( void );

    // Used for threads that may never terminate (ie Listener Thread)
    void thread_register_nonterm( void );
    void thread_unregister_nonterm( void );
    int thread_release_nonterm( int force );

    /* -------------------------------------------------------------------
     * Return the current thread's ID.
     * ------------------------------------------------------------------- */
    #if   defined( HAVE_POSIX_THREAD )
        #define thread_getid() pthread_self()
    #elif defined( HAVE_WIN32_THREAD )
        #define thread_getid() GetCurrentThreadId()
    #else
        #define thread_getid() 0
    #endif

    int thread_equalid( nthread_t inLeft, nthread_t inRight );

    nthread_t thread_zeroid( void );

#if   defined( HAVE_WIN32_THREAD )
    DWORD WINAPI thread_run_wrapper( void* paramPtr );
#else
    void*        thread_run_wrapper( void* paramPtr );
#endif

    void thread_rest ( void );

    // defined in launch.cpp
    void server_spawn( struct thread_Settings* thread );
    void client_spawn( struct thread_Settings* thread );
    void client_init( struct thread_Settings* clients );
    void listener_spawn( struct thread_Settings* thread );

    // defined in reporter.c
    void reporter_spawn( struct thread_Settings* thread );

#endif // THREAD_H

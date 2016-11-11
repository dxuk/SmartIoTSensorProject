/*---------------------------------------------------------------
 * standard signal installer
 * ------------------------------------------------------------------- */

#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "util.h"

#ifdef WIN32

/* list of signal handlers. _NSIG is number of signals defined. */

static SigfuncPtr handlers[ _NSIG ] = { 0};

/* -------------------------------------------------------------------
 * sig_dispatcher
 *
 * dispatches the signal to appropriate signal handler. This emulates
 * the signal handling of Unix.
 *
 * ------------------------------------------------------------------- */

BOOL WINAPI sig_dispatcher( DWORD type ) {
    SigfuncPtr h = NULL;
    int signo;

    switch ( type ) {
        case CTRL_C_EVENT:
            signo = SIGINT;
            h = handlers[ SIGINT ];
            break;

        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            signo = SIGTERM;
            h = handlers[ SIGTERM ];
            break;

        default:
            break;
    }

    if ( h != NULL ) {
        // call the signal handler
        h( signo );
        return true;
    } else {
        return false;
    }
}

/* -------------------------------------------------------------------
 * my_signal
 *
 * installs a  signal handler. I emulate Unix signals by storing the
 * function pointers and dispatching events myself, using the
 * sig_dispatcher above.
 * ------------------------------------------------------------------- */

SigfuncPtr my_signal( int inSigno, SigfuncPtr inFunc ) {
    SigfuncPtr old = NULL;

    if ( inSigno >= 0  &&  inSigno < _NSIG ) {
        old = handlers[ inSigno ];
        handlers[ inSigno ] = inFunc;
    }

    return old;
} /* end my_signal */

#else /* not WIN32 */

/* -------------------------------------------------------------------
 * my_signal
 *
 * installs a signal handler, and returns the old handler.
 * This emulates the semi-standard signal() function in a
 * standard way using the Posix sigaction function.
 *
 * from Stevens, 1998, section 5.8
 * ------------------------------------------------------------------- */

SigfuncPtr my_signal( int inSigno, SigfuncPtr inFunc ) {
    struct sigaction theNewAction, theOldAction;

    assert( inFunc != NULL );

    theNewAction.sa_handler = inFunc;
    sigemptyset( &theNewAction.sa_mask );
    theNewAction.sa_flags = 0;

    if ( inSigno == SIGALRM ) {
#ifdef SA_INTERRUPT
        theNewAction.sa_flags |= SA_INTERRUPT;  /* SunOS 4.x */
#endif
    } else {
#ifdef SA_RESTART
        theNewAction.sa_flags |= SA_RESTART;    /* SVR4, 4.4BSD */
#endif
    }

    if ( sigaction( inSigno, &theNewAction, &theOldAction ) < 0 ) {
        return SIG_ERR;
    } else {
        return theOldAction.sa_handler;
    }
} /* end my_signal */

#endif /* not WIN32 */

/* -------------------------------------------------------------------
 * sig_exit
 *
 * Quietly exits. This protects some against being called multiple
 * times. (TODO: should use a mutex to ensure (num++ == 0) is atomic.)
 * ------------------------------------------------------------------- */

void sig_exit( int inSigno ) {
    static int num = 0;
    if ( num++ == 0 ) {
        fflush( 0 );
        exit( 0 );
    }
} /* end sig_exit */


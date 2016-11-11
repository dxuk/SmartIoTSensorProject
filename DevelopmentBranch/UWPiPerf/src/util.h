/*---------------------------------------------------------------
 * various C utility functions.
 * ------------------------------------------------------------------- */

#ifndef UTIL_H
#define UTIL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include "config.h"
#endif

/* -------------------------------------------------------------------
 * set/getsockopt wrappers for SO_RCVBUF and SO_SNDBUF; TCP_MAXSEG
 * socket.c
 * ------------------------------------------------------------------- */
int setsock_tcp_windowsize( SOCKET inSock, int inTCPWin, int inSend );
int getsock_tcp_windowsize( SOCKET inSock, int inSend );

void setsock_tcp_mss( SOCKET inSock, int inTCPWin );
int  getsock_tcp_mss( SOCKET inSock );

/* -------------------------------------------------------------------
 * signal handlers
 * signal.c
 * ------------------------------------------------------------------- */
typedef void Sigfunc(int);
void sig_exit( int inSigno );

typedef Sigfunc *SigfuncPtr;

SigfuncPtr my_signal( int inSigno, SigfuncPtr inFunc );

#ifdef WIN32

/* under windows, emulate unix signals */
enum {
    SIGINT,
    SIGTERM,
    SIGPIPE,
    _NSIG
};

BOOL WINAPI sig_dispatcher( DWORD type );

#endif

/* -------------------------------------------------------------------
 * error handlers
 * error.c
 * ------------------------------------------------------------------- */
void warn      ( const char *inMessage, const char *inFile, int inLine );
void warn_errno( const char *inMessage, const char *inFile, int inLine );

#if defined( HAVE_POSIX_THREAD ) || defined( HAVE_WIN32_THREAD)
#define FAIL( cond, msg, settings )             \
  do {                                          \
    if ( cond ) {                               \
      warn( msg, __FILE__, __LINE__ );          \
      thread_stop(settings);                    \
    }                                           \
  } while( 0 )
#else
#define FAIL( cond, msg, settings )             \
  do {                                          \
    if ( cond ) {                               \
      warn( msg, __FILE__, __LINE__ );          \
      exit( 1 );                                \
    }                                           \
  } while( 0 )
#endif

#define WARN( cond, msg )                       \
  do {                                          \
    if ( cond ) {                               \
      warn( msg, __FILE__, __LINE__ );          \
    }                                           \
  } while( 0 )

#if defined( HAVE_POSIX_THREAD ) || defined( HAVE_WIN32_THREAD)
#define FAIL_errno( cond, msg, settings )       \
  do {                                          \
    if ( cond ) {                               \
      warn_errno( msg, __FILE__, __LINE__ );    \
      thread_stop(settings);                    \
    }                                           \
  } while( 0 )
#else
#define FAIL_errno( cond, msg, settings )       \
  do {                                          \
    if ( cond ) {                               \
      warn_errno( msg, __FILE__, __LINE__ );    \
      exit( 1 );                                \
    }                                           \
  } while( 0 )
#endif

#define WARN_errno( cond, msg )                 \
  do {                                          \
    if ( cond ) {                               \
      warn_errno( msg, __FILE__, __LINE__ );    \
    }                                           \
  } while( 0 )

/* -------------------------------------------------------------------
 * initialize buffer to a pattern
 * ------------------------------------------------------------------- */
void pattern( char *outBuf, int inBytes );

/* -------------------------------------------------------------------
 * input and output numbers, converting with kilo, mega, giga
 * stdio.c
 * ------------------------------------------------------------------- */
double byte_atof( const char *inString );
max_size_t byte_atoi( const char  *inString );
void byte_snprintf( char* outString, int inLen, double inNum, char inFormat );

/* -------------------------------------------------------------------
 * redirect the stdout to a specified file
 * stdio.c
 * ------------------------------------------------------------------- */
void redirect(const char *inOutputFileName);

/* -------------------------------------------------------------------
 * delete macro
 * ------------------------------------------------------------------- */
#define DELETE_PTR( ptr )                       \
  do {                                          \
    if ( ptr != NULL ) {                        \
      delete ptr;                               \
      ptr = NULL;                               \
    }                                           \
  } while( false )

#define DELETE_ARRAY( ptr )                     \
  do {                                          \
    if ( ptr != NULL ) {                        \
      delete [] ptr;                            \
      ptr = NULL;                               \
    }                                           \
  } while( false )

#endif /* UTIL_H */


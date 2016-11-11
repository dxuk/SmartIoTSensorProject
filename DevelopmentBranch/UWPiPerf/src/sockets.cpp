/*---------------------------------------------------------------
 * set/getsockopt and read/write wrappers
 * ------------------------------------------------------------------- */

#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "util.h"

/* -------------------------------------------------------------------
 * If inMSS > 0, set the TCP maximum segment size  for inSock.
 * Otherwise leave it as the system default.
 * ------------------------------------------------------------------- */

const char warn_mss_fail[] = "\
WARNING: attempt to set TCP maxmimum segment size to %d failed.\n\
Setting the MSS may not be implemented on this OS.\n";

const char warn_mss_notset[] =
"WARNING: attempt to set TCP maximum segment size to %d, but got %d\n";

void setsock_tcp_mss( SOCKET inSock, int inMSS ) {
#ifdef TCP_MAXSEG
    int rc;
    int newMSS;
    Socklen_t len;

    assert( inSock != INVALID_SOCKET );

    if ( inMSS > 0 ) {
        /* set */
        newMSS = inMSS;
        len = sizeof( newMSS );
        rc = setsockopt( inSock, IPPROTO_TCP, TCP_MAXSEG, (char*) &newMSS,  len );
        if ( rc == SOCKET_ERROR ) {
            fprintf( stderr, warn_mss_fail, newMSS );
            return;
        }

        /* verify results */
        rc = getsockopt( inSock, IPPROTO_TCP, TCP_MAXSEG, (char*) &newMSS, &len );
        WARN_errno( rc == SOCKET_ERROR, "getsockopt TCP_MAXSEG" );
        if ( newMSS != inMSS ) {
            fprintf( stderr, warn_mss_notset, inMSS, newMSS );
        }
    }
#endif
} /* end setsock_tcp_mss */

/* -------------------------------------------------------------------
 * returns the TCP maximum segment size
 * ------------------------------------------------------------------- */

int getsock_tcp_mss( SOCKET inSock ) {
    int theMSS = 0;

#ifdef TCP_MAXSEG
    int rc;
    Socklen_t len;
    assert( inSock >= 0 );

    /* query for MSS */
    len = sizeof( theMSS );
    rc = getsockopt( inSock, IPPROTO_TCP, TCP_MAXSEG, (char*) &theMSS, &len );
    WARN_errno( rc == SOCKET_ERROR, "getsockopt TCP_MAXSEG" );
#endif

    return theMSS;
} /* end getsock_tcp_mss */

/* -------------------------------------------------------------------
 * Attempts to reads n bytes from a socket.
 * Returns number actually read, or -1 on error.
 * If number read < inLen then we reached EOF.
 *
 * from Stevens, 1998, section 3.9
 * ------------------------------------------------------------------- */

ssize_t readn( SOCKET inSock, void *outBuf, size_t inLen ) {
    size_t  nleft;
    int     nread;
    char *ptr;

    assert( inSock >= 0 );
    assert( outBuf != NULL );
    assert( inLen > 0 );

    ptr   = (char*) outBuf;
    nleft = inLen;

    while ( nleft > 0 ) {
        nread = (int) read( inSock, ptr, (int) nleft );
        if ( nread < 0 ) {
            if ( errno == EINTR )
                nread = 0;  /* interupted, call read again */
            else
                return -1;  /* error */
        } else if ( nread == 0 )
            break;        /* EOF */

        nleft -= nread;
        ptr   += nread;
    }

    return (int) (inLen - nleft);
} /* end readn */

/* -------------------------------------------------------------------
 * Attempts to write  n bytes to a socket.
 * returns number actually written, or -1 on error.
 * number written is always inLen if there is not an error.
 *
 * from Stevens, 1998, section 3.9
 * ------------------------------------------------------------------- */

ssize_t writen( SOCKET inSock, const void *inBuf, size_t inLen ) {
    size_t  nleft;
    int     nwritten;
    char    *ptr; /* FIX: const char * write() at compiler wran */

    assert( inSock >= 0 );
    assert( inBuf != NULL );
    assert( inLen > 0 );

    ptr   = (char*) inBuf;
    nleft = inLen;

    while ( nleft > 0 ) {
        nwritten = (int) write( inSock, ptr, (int) nleft );
        if ( nwritten <= 0 ) {
            if ( errno == EINTR )
                nwritten = 0; /* interupted, call write again */
            else
                return -1;    /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }

    return (int) inLen;
} /* end writen */


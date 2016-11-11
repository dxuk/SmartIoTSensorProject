#ifndef SOCKET_ADDR_H
#define SOCKET_ADDR_H

#include "headers.h"
#include "Settings.h"

/* ------------------------------------------------------------------- */
    void SockAddr_localAddr( thread_Settings *inSettings );
    void SockAddr_remoteAddr( thread_Settings *inSettings );

    void SockAddr_setHostname( const char* inHostname,
                               iperf_sockaddr *inSockAddr,
                               int isIPv6 );          // DNS lookup
    void SockAddr_getHostname( iperf_sockaddr *inSockAddr,
                               char* outHostname,
                               size_t len );   // reverse DNS lookup
    void SockAddr_getHostAddress( iperf_sockaddr *inSockAddr,
                                  char* outAddress,
                                  size_t len ); // dotted decimal

    void SockAddr_setPort( iperf_sockaddr *inSockAddr, unsigned short inPort );
    void SockAddr_setPortAny( iperf_sockaddr *inSockAddr );
    unsigned short SockAddr_getPort( iperf_sockaddr *inSockAddr );

    void SockAddr_setAddressAny( iperf_sockaddr *inSockAddr );

    // return pointer to the struct in_addr
    struct in_addr* SockAddr_get_in_addr( iperf_sockaddr *inSockAddr );
#ifdef HAVE_IPV6
    // return pointer to the struct in_addr
    struct in6_addr* SockAddr_get_in6_addr( iperf_sockaddr *inSockAddr );
#endif
    // return the sizeof the addess structure (struct sockaddr_in)
    Socklen_t SockAddr_get_sizeof_sockaddr( iperf_sockaddr *inSockAddr );

    int SockAddr_isMulticast( iperf_sockaddr *inSockAddr );

    int SockAddr_isIPv6( iperf_sockaddr *inSockAddr );

    int SockAddr_are_Equal( struct sockaddr *first, struct sockaddr *second );
    int SockAddr_Hostare_Equal( struct sockaddr *first, struct sockaddr *second );

    void SockAddr_zeroAddress( iperf_sockaddr *inSockAddr );

#endif /* SOCKET_ADDR_H */

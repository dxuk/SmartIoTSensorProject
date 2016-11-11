/*---------------------------------------------------------------
 * Has routines the Client and Server classes use in common for
 * performance testing the network.
 * ------------------------------------------------------------------- */

#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "PerfSocket.h"
#include "SocketAddr.h"
#include "util.h"

/* -------------------------------------------------------------------
 * Set socket options before the listen() or connect() calls.
 * These are optional performance tuning factors.
 * ------------------------------------------------------------------- */
#if 0 /* template */

    if ( isSCTP( inSettings ) ) {
        /* SCTP */
    } else if(! isUDP( inSettings )) {
        /* TCP */
    } else {
        /* UDP */
    }

#endif /* template */

void SetSocketOptions( thread_Settings *inSettings )
{
    int rc = 0;
    int MSS;
    int tos;
    Socklen_t len;
    int val;
#ifdef SO_PRIORITY
    int  priority;
#endif
#ifdef HAVE_MULTICAST
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *int_addr;
#endif
#ifdef HAVE_IPV6_MULTICAST
    int fd6;
    struct ifreq ifr6;
    struct sockaddr_in6 *int_addr6;
#endif

#ifdef WIN32
    DWORD dwBytesSent;
#endif /* WIN32 */

/* ------------------> TCP/UDP window size ------------------------> */
    /* set the TCP window size (socket buffer sizes)
     * also the UDP buffer size
     * must occur before call to accept() for large window sizes
     */
    setsock_tcp_windowsize( inSettings->mSock, inSettings->mTCPWin,
                            (inSettings->mThreadMode == kMode_Client ? 1 : 0) );
/* <------------------ TCP window size <------------------------ */

/* ------------------> TCP_CONGESTION ------------------------> */
    if ( isCongestionControl( inSettings ) ) {
        if ( isSCTP( inSettings ) ) {
            /* SCTP */
        } else if(! isUDP( inSettings )) {
            /* TCP */
#ifdef TCP_CONGESTION
            len = strlen( inSettings->mCongestion ) + 1;
            rc = setsockopt( inSettings->mSock, IPPROTO_TCP, TCP_CONGESTION,
                         inSettings->mCongestion, len);
            if (rc == SOCKET_ERROR ) {
                fprintf(stderr, "Attempt to set '%s' congestion control failed: %s\n",
                    inSettings->mCongestion, strerror(errno));
                exit(1);
            }
#else
            fprintf( stderr, "The -Z option is not available on this operating system\n");
#endif
        } else {
            /* UDP */
        }
    
    }
/* <------------------ TCP_CONGESTION <------------------------ */

/* ------------------> TTL ------------------------> */
    // check if we're sending multicast, and set TTL
    if ( isMulticast( inSettings ) && ( inSettings->mTTL > 0 ) ) {
        val = inSettings->mTTL;
#ifdef HAVE_MULTICAST
        if ( !SockAddr_isIPv6( &inSettings->local ) ) {
            rc = setsockopt( inSettings->mSock, IPPROTO_IP, IP_MULTICAST_TTL,
                (const char*) &val, (Socklen_t) sizeof(val));

            WARN_errno( rc == SOCKET_ERROR, "multicast ttl" );
            if(isCustInterface ( inSettings ) ) {
                fd = socket(AF_INET, SOCK_DGRAM, 0);
                ifr.ifr_addr.sa_family = AF_INET;
                strncpy(ifr.ifr_name, inSettings->mCustInterface, IFNAMSIZ-1);
                if(ioctl(fd, SIOCGIFADDR, &ifr) != -1)
                {
                    int_addr = (struct sockaddr_in *)&ifr.ifr_addr;
                    setsockopt(inSettings->mSock, IPPROTO_IP, IP_MULTICAST_IF, &int_addr->sin_addr, sizeof(int_addr->sin_addr));
                }
                close(fd);
            }
        }
#ifdef HAVE_IPV6_MULTICAST
        else {
            rc = setsockopt( inSettings->mSock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                (const void*) &val, (Socklen_t) sizeof(val));
            WARN_errno( rc == SOCKET_ERROR, "multicast ttl" );

            if(isCustInterface ( inSettings ) ) {
                fd6 = socket(AF_INET6, SOCK_DGRAM, 0);
                ifr6.ifr_addr.sa_family = AF_INET6;
                strncpy(ifr6.ifr_name, inSettings->mCustInterface, IFNAMSIZ-1);
                if(ioctl(fd6, SIOCGIFADDR, &ifr6) == 0)
                {
                    int_addr6 = (struct sockaddr_in6 *)&ifr6.ifr_addr;
                    setsockopt(inSettings->mSock, IPPROTO_IPV6, IPV6_MULTICAST_IF, &int_addr6->sin6_addr, sizeof(int_addr6->sin6_addr));
                }
                close(fd6);
            }
        }
#endif /* HAVE_IPV6_MULTICAST */
#endif /* HAVE_MULTICAST */
    }
/* <------------------ TTL <------------------------ */

/* ------------------> IP_TOS ------------------------> */
    // set IP TOS (type-of-service) field
    if ( inSettings->mTOS > 0 ) {
        tos = inSettings->mTOS;
        len = sizeof(tos);

#ifdef WIN32
//now testing...
//= "0x60 SERVICETYPE_CONTROLLEDLOAD  Controlled Load" 2
//= "0xA0 SERVICETYPE_GUARANTEED      Guaranteed"      3

		/* Delete mQOS temp because build error. */
#if 0
        inSettings->mQOS.SendingFlowspec.TokenRate = 1;                             /* In Bytes/sec */
        inSettings->mQOS.SendingFlowspec.TokenBucketSize = 1;                       /* In Bytes */
        inSettings->mQOS.SendingFlowspec.PeakBandwidth = 1;                         /* In Bytes/sec */
        inSettings->mQOS.SendingFlowspec.Latency = QOS_NOT_SPECIFIED;               /* In microseconds */
        inSettings->mQOS.SendingFlowspec.DelayVariation = QOS_NOT_SPECIFIED;        /* In microseconds */
        inSettings->mQOS.SendingFlowspec.ServiceType = inSettings->mTOS;
        inSettings->mQOS.SendingFlowspec.MaxSduSize = QOS_NOT_SPECIFIED;            /* In Bytes */
        inSettings->mQOS.SendingFlowspec.MinimumPolicedSize = QOS_NOT_SPECIFIED;    /* In Bytes */

        inSettings->mQOS.ReceivingFlowspec.TokenRate = 1;                           /* In Bytes/sec */
        inSettings->mQOS.ReceivingFlowspec.TokenBucketSize = 1;                     /* In Bytes */
        inSettings->mQOS.ReceivingFlowspec.PeakBandwidth = 1;                       /* In Bytes/sec */
        inSettings->mQOS.ReceivingFlowspec.Latency = QOS_NOT_SPECIFIED;             /* In microseconds */
        inSettings->mQOS.ReceivingFlowspec.DelayVariation = QOS_NOT_SPECIFIED;      /* In microseconds */
        inSettings->mQOS.ReceivingFlowspec.ServiceType = inSettings->mTOS;
        inSettings->mQOS.ReceivingFlowspec.MaxSduSize = QOS_NOT_SPECIFIED;          /* In Bytes */
        inSettings->mQOS.ReceivingFlowspec.MinimumPolicedSize = QOS_NOT_SPECIFIED;  /* In Bytes */

        inSettings->mQOS.ProviderSpecific.buf = NULL;
        inSettings->mQOS.ProviderSpecific.len = 0;
#endif

		/* Delete mQOS temp because build error. */
		WSAIoctl(inSettings->mSock, SIO_SET_QOS, NULL/*&inSettings->mQOS*/, 0 /*sizeof(QOS)*/ , NULL, 0, &dwBytesSent, NULL, NULL);

#else /* WIN32 */
#    ifdef IP_TOS
       if ( !isIPV6( inSettings ) )
       {
          // for IPv4
          rc = setsockopt( inSettings->mSock, IPPROTO_IP, IP_TOS,
                               (char*) &tos, len );
          WARN_errno( rc == SOCKET_ERROR, "setsockopt IP_TOS" );
       }
       else
       {
#    ifndef IPV6_TCLASS
#        define IPV6_TCLASS 67
#    endif
          // for IPv6 (traffic class)
          rc = setsockopt( inSettings->mSock, IPPROTO_IPV6, IPV6_TCLASS,
                               (char*) &tos, len );
          WARN_errno( rc == SOCKET_ERROR, "setsockopt IPV6_TCLASS" );
       }
#    endif /* IP_TOS */
#endif /* WIN32 */
    }
/* <------------------ IP_TOS <------------------------ */

/* ------------------> SO_PRIORITY ------------------------> */
#ifdef SO_PRIORITY
    /* From socket(7): "Set the protocol-defined priority for all
     * packets to be sent on this socket. Linux uses this value to order
     * the networking queues: packets with a higher priority may be
     * processed first depending on the selected device queueing
     * discipline. For ip(7), this also sets the IP type-of-service (TOS)
     * field for outgoing packets. Setting a priority outside the range 0
     * to 6 requires the CAP_NET_ADMIN capability." */
    if ( inSettings->mPriority > 0 ) {
        priority = inSettings->mPriority;
        Socklen_t len = sizeof(priority);
        rc = setsockopt( inSettings->mSock, SOL_SOCKET, SO_PRIORITY,
                             (char*) &priority, len );
        WARN_errno( rc == SOCKET_ERROR, "setsockopt SO_PRIORITY" );
    }
#endif
/* <------------------ SO_PRIORITY <------------------------ */

/* ------------------> TCP_MAXSEG ------------------------> */ 
    if ( inSettings->mMSS > 0 ) {
        if ( isSCTP( inSettings ) ) {
            /* set the SCTP maximum segment size */
            MSS = inSettings->mMSS;
            Socklen_t len = sizeof(MSS);
            rc = setsockopt( inSettings->mSock, SOL_SCTP, SCTP_MAXSEG,
                     (char *) &MSS, len );
            WARN_errno( rc == SOCKET_ERROR, "setsockopt SCTP_MAXSEG" );
        } else if(! isUDP( inSettings )) {
            /* set the TCP maximum segment size */
            setsock_tcp_mss( inSettings->mSock, inSettings->mMSS );
            WARN_errno( rc == SOCKET_ERROR, "setsockopt TCP_MAXSEG" );
        } else {
            /* UDP */
        }       
    }
/* <------------------ TCP_MAXSEG <------------------------ */

/* ------------------> TCP_NODELAY ------------------------> */
    if ( isSCTP( inSettings ) ) {
#ifdef SCTP_NODELAY
        /* set SCTP nodelay option */
        if ( isNoDelay( inSettings ) ) {
            int nodelay = 1;
            Socklen_t len = sizeof(nodelay);
            int rc = setsockopt( inSettings->mSock, SOL_SCTP, SCTP_NODELAY,
                     (char *) &nodelay, len );
            WARN_errno( rc == SOCKET_ERROR, "setsockopt SCTP_NODELAY" );
        }
#endif
    } else if ( !isUDP( inSettings ) ) {
#ifdef TCP_NODELAY
        /* set TCP nodelay option */
        if ( isNoDelay( inSettings ) ) {
            int nodelay = 1;
            Socklen_t len = sizeof(nodelay);
            int rc = setsockopt( inSettings->mSock, IPPROTO_TCP, TCP_NODELAY,
                                 (char*) &nodelay, len );
            WARN_errno( rc == SOCKET_ERROR, "setsockopt TCP_NODELAY" );
        }
#endif
    }
/* <------------------ TCP_NODELAY <------------------------ */
}
// end SetSocketOptions

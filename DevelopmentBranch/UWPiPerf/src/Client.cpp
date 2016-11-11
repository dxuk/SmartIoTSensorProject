/*---------------------------------------------------------------
 * A client thread initiates a connect to the server and handles
 * sending and receiving data, then closes the socket.
 * ------------------------------------------------------------------- */

#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "ui_communication.h"

#include "Client.h"
#include "Thread.h"
#include "SocketAddr.h"
#include "PerfSocket.h"
#include "Extractor.h"
#include "delay.h"
#include "util.h"
#include "Locale.h"

/* -------------------------------------------------------------------
 * Store server hostname, optionally local hostname, and socket info.
 * ------------------------------------------------------------------- */

Client::Client( thread_Settings *inSettings ) {
    mSettings = inSettings;
    mBuf = NULL;

    // initialize buffer
    mBuf = new char[ mSettings->mBufLen ];
    pattern( mBuf, mSettings->mBufLen );
    if ( isFileInput( mSettings ) ) {
        if ( !isSTDIN( mSettings ) )
            Extractor_Initialize( mSettings->mFileName, mSettings->mBufLen, mSettings );
        else
            Extractor_InitializeFile( stdin, mSettings->mBufLen, mSettings );

        if ( !Extractor_canRead( mSettings ) ) {
            unsetFileInput( mSettings );
        }
    }

    // connect
    Connect( );

    if ( isReport( inSettings ) ) {
        ReportSettings( inSettings );
        if ( mSettings->multihdr && isMultipleReport( inSettings ) ) {
            mSettings->multihdr->report->connection.peer = mSettings->peer;
            mSettings->multihdr->report->connection.size_peer = mSettings->size_peer;
            mSettings->multihdr->report->connection.local = mSettings->local;
            SockAddr_setPortAny( &mSettings->multihdr->report->connection.local );
            mSettings->multihdr->report->connection.size_local = mSettings->size_local;
        }
    }

} // end Client

/* -------------------------------------------------------------------
 * Delete memory (hostname strings).
 * ------------------------------------------------------------------- */

Client::~Client() {
    if ( mSettings->mSock != INVALID_SOCKET ) {
        int rc = close( mSettings->mSock );
        WARN_errno( rc == SOCKET_ERROR, "close" );
        mSettings->mSock = INVALID_SOCKET;
    }
    DELETE_ARRAY( mBuf );
} // end ~Client

const double kSecs_to_usecs = 1e6;
const int    kBytes_to_Bits = 8;

void Client::RunTCP( void ) {
    long currLen;
    max_size_t totLen = 0;

#ifdef WIN32
    time_t t0;
    double d0;
#else
    struct itimerval it;
    int err;
#endif

    char* readAt = mBuf;

    // Indicates if the stream is readable
    bool canRead = true, mMode_Time = isModeTime( mSettings );

    ReportStruct *reportstruct = NULL;

    // InitReport handles Barrier for multiple Streams
    mSettings->reporthdr = InitReport( mSettings );
    reportstruct = new ReportStruct;
    reportstruct->packetID = 0;

    lastPacketTime.setnow();

    //-t0 : infinity
    if ( mMode_Time && mSettings->mAmount != 0 ) {
#ifdef WIN32
        t0 = time(NULL);
#else
        memset (&it, 0, sizeof (it));
        it.it_value.tv_sec = (int) ((double)mSettings->mAmount / 100.0);
        it.it_value.tv_usec = (int) 10000 * ((double)mSettings->mAmount -
            (double)it.it_value.tv_sec * 100.0);
        err = setitimer( ITIMER_REAL, &it, NULL );
        if ( err != 0 ) {
            perror("setitimer");
            exit(1);
        }
#endif
    }
    do {
        // Read the next data block from
        // the file if it's file input
        if ( isFileInput( mSettings ) ) {
            Extractor_getNextDataBlock( readAt, mSettings );
            canRead = Extractor_canRead( mSettings ) != 0;
        } else
            canRead = true;

        // perform write
        currLen = write( mSettings->mSock, mBuf, mSettings->mBufLen );
        if ( currLen < 0 ) {
            WARN_errno( currLen < 0, "write2" );
            break;
        }
        totLen += currLen;

        if(mSettings->mInterval > 0) {
            gettimeofday( &(reportstruct->packetTime), NULL );
            reportstruct->packetLen = currLen;
            ReportPacket( mSettings->reporthdr, reportstruct );
        }

        if ( !mMode_Time ) {
            /* mAmount may be unsigned, so don't let it underflow! */
            if( mSettings->mAmount >= (unsigned long) currLen ) {
                mSettings->mAmount -= (unsigned long) currLen;
            } else {
                mSettings->mAmount = 0;
            }
        }
#ifdef WIN32
        else if ( mMode_Time && mSettings->mAmount != 0) {
            d0 = difftime(time(NULL), t0);
            if( d0 > mSettings->mAmount / 100.0 || d0 < 0) {
                break;
            }
        }
#endif
    } while ( ! (sInterupted  ||
        (!mMode_Time  &&  0 >= mSettings->mAmount)) && canRead );

    // stop timing
    gettimeofday( &(reportstruct->packetTime), NULL );

    // if we're not doing interval reporting, report the entire transfer as one big packet
    if(0.0 == mSettings->mInterval) {
        reportstruct->packetLen = totLen;
        ReportPacket( mSettings->reporthdr, reportstruct );
    }
    CloseReport( mSettings->reporthdr, reportstruct );

    DELETE_PTR( reportstruct );
    EndReport( mSettings->reporthdr );
}

/* -------------------------------------------------------------------
 * Send data using the connected UDP/TCP socket,
 * until a termination flag is reached.
 * Does not close the socket.
 * ------------------------------------------------------------------- */

void Client::Run( void )
{
    int wc;
    struct UDP_datagram* mBuf_UDP = (struct UDP_datagram*) mBuf;
    signed long currLen = 0;
    unsigned int burstcount = 0;

    int delay_target = 0;
    int delay = 0;
    int adjust = 0;

    char* readAt = mBuf;

#if HAVE_THREAD
    if ( !isUDP( mSettings ) ) {
        RunTCP();
        return;
    }
#endif

    // Indicates if the stream is readable
    bool canRead = true, mMode_Time = isModeTime( mSettings );

    // setup termination variables
    if ( mMode_Time && mSettings->mAmount ) {
        mEndTime.setnow();
        mEndTime.add( mSettings->mAmount / 100.0 );
    }

    if ( isUDP( mSettings ) ) {
        // Due to the UDP timestamps etc, included
        // reduce the read size by an amount
        // equal to the header size

        // compute delay for bandwidth restriction, constrained to [0,1] seconds
        delay_target = (int) ( mSettings->mBufLen * ((kSecs_to_usecs * kBytes_to_Bits)
                                                     / mSettings->mUDPRate) );
        if ( delay_target < 0  ||
             delay_target > (int) 1 * kSecs_to_usecs ) {
            fprintf( stderr, warn_delay_large, delay_target / kSecs_to_usecs );
            delay_target = (int) kSecs_to_usecs * 1;
        }
        if ( isFileInput( mSettings ) ) {
            if ( isCompat( mSettings ) ) {
                Extractor_reduceReadSize( sizeof(struct UDP_datagram), mSettings );
                readAt += sizeof(struct UDP_datagram);
            } else {
                Extractor_reduceReadSize( sizeof(struct UDP_datagram) +
                                          sizeof(struct client_hdr), mSettings );
                readAt += sizeof(struct UDP_datagram) +
                          sizeof(struct client_hdr);
            }
        }
    }

    ReportStruct *reportstruct = NULL;

    // InitReport handles Barrier for multiple Streams
    mSettings->reporthdr = InitReport( mSettings );
    reportstruct = new ReportStruct;
    reportstruct->packetID = 0;

    lastPacketTime.setnow();

    if (isPoisson(mSettings)) srand(lastPacketTime.getUsecs());

    do {

        // Test case: drop 17 packets and send 2 out-of-order:
        // sequence 51, 52, 70, 53, 54, 71, 72
        /*
        switch( reportstruct->packetID ) {
            case 53: reportstruct->packetID = 70; break;
            case 71: reportstruct->packetID = 53; break;
            case 54: reportstruct->packetID = 71; break;
            default: break;
        }
        */
        gettimeofday( &(reportstruct->packetTime), NULL );

        if ( isUDP( mSettings ) ) {
            // store datagram ID into buffer
            mBuf_UDP->id      = htonl( (reportstruct->packetID)++ );
            mBuf_UDP->tv_sec  = htonl( reportstruct->packetTime.tv_sec );
            mBuf_UDP->tv_usec = htonl( reportstruct->packetTime.tv_usec );

            // delay between writes
            // make an adjustment for how long the last loop iteration took
            // TODO this doesn't work well in certain cases, like 2 parallel streams
            adjust = delay_target + lastPacketTime.subUsec( reportstruct->packetTime );
            if (isPoisson(mSettings)) {
                adjust = delay + lastPacketTime.subUsec( reportstruct->packetTime );
            }
            lastPacketTime.set( reportstruct->packetTime.tv_sec,
                                reportstruct->packetTime.tv_usec );

            if ( adjust > 0  ||  delay > 0 ) {
                delay += adjust;
            }
        }

        // Read the next data block from
        // the file if it's file input
        if ( isFileInput( mSettings ) ) {
            Extractor_getNextDataBlock( readAt, mSettings );
            canRead = Extractor_canRead( mSettings ) != 0;
        } else
            canRead = true;

        // perform write
        currLen = write( mSettings->mSock, mBuf, mSettings->mBufLen );
        if ( currLen < 0 && currLen != -ENOBUFS ) {
            WARN_errno( currLen < 0, "write2" );
            break;
        }

        // report packets
        reportstruct->packetLen = currLen;
        ReportPacket( mSettings->reporthdr, reportstruct );

        if (isPoisson(mSettings)) {
            delay = (int)(-1.0*(double)delay_target*log((double)1.0-1.0*rand()/((double)RAND_MAX))) + adjust;
        }

        if (mSettings->mBurstRate) {
            if ((++burstcount >= mSettings->mBurstRate) && (delay > 0))
            {
                max_size_t loop = mSettings->mBurstRate;
                if(loop)
                {
                    while(--loop)
                    {
                        delay_loop(delay);
                    }
                }
            }
            burstcount %= mSettings->mBurstRate;
        }
        else if ( delay > 0 ) {
            delay_loop( delay );
        }
        if ( !mMode_Time ) {
            /* mAmount may be unsigned, so don't let it underflow! */
            if( mSettings->mAmount >= (unsigned long) currLen ) {
                mSettings->mAmount -= (unsigned long) currLen;
            } else {
                mSettings->mAmount = 0;
            }
        }

    } while ( ! (sInterupted  ||
                 (mMode_Time   &&  mEndTime.before( reportstruct->packetTime ))  ||
                 (!mMode_Time  &&  0 >= mSettings->mAmount)) && canRead );

    // stop timing
    gettimeofday( &(reportstruct->packetTime), NULL );
    CloseReport( mSettings->reporthdr, reportstruct );

    if ( isUDP( mSettings ) ) {
        // send a final terminating datagram
        // Don't count in the mTotalLen. The server counts this one,
        // but didn't count our first datagram, so we're even now.
        // The negative datagram ID signifies termination to the server.

        // store datagram ID into buffer
        mBuf_UDP->id      = htonl( -(reportstruct->packetID)  );
        mBuf_UDP->tv_sec  = htonl( reportstruct->packetTime.tv_sec );
        mBuf_UDP->tv_usec = htonl( reportstruct->packetTime.tv_usec );

        if ( isMulticast( mSettings ) ) {
            wc = write( mSettings->mSock, mBuf, mSettings->mBufLen );
            WARN_errno( wc < 0, "write Run" );
        } else {
            write_UDP_FIN( );
        }
    }
    DELETE_PTR( reportstruct );
    EndReport( mSettings->reporthdr );
}
// end Run

void Client::InitiateServer() {
    if ( !isCompat( mSettings ) ) {
        int currLen;
        client_hdr* temp_hdr;
        if ( isUDP( mSettings ) ) {
            UDP_datagram *UDPhdr = (UDP_datagram *)mBuf;
            temp_hdr = (client_hdr*)(UDPhdr + 1);
        } else {
            temp_hdr = (client_hdr*)mBuf;
        }
        Settings_GenerateClientHdr( mSettings, temp_hdr );
        if ( !isUDP( mSettings ) ) {
            currLen = send( mSettings->mSock, mBuf, sizeof(client_hdr), 0 );
            if ( currLen < 0 ) {
                WARN_errno( currLen < 0, "write1" );
            }
        }
    }
}

/* -------------------------------------------------------------------
 * Setup a socket connected to a server.
 * If inLocalhost is not null, bind to that address, specifying
 * which outgoing interface to use.
 * ------------------------------------------------------------------- */
#if 0 /* template */

    if ( isSCTP( mSettings ) ) {
        /* SCTP */
    } else if(! isUDP( mSettings )) {
        /* TCP */
    } else {
        /* UDP */
    }

#endif /* template */

void Client::Connect( )
{
    int rc;
    int type = 0;
    int protocol = 0;
    int domain;

    SockAddr_remoteAddr( mSettings );

    assert( mSettings->inHostname != NULL );

    // create an internet socket

    if ( isSCTP( mSettings ) ) {
        /* SCTP */
        type = SOCK_STREAM;
        protocol = IPPROTO_SCTP;
    } else if(! isUDP( mSettings )) {
        /* TCP */
        type = SOCK_STREAM;
        protocol = IPPROTO_TCP;
    } else {
        /* UDP */
        type = SOCK_DGRAM;
        protocol = IPPROTO_UDP;
    }
    /* overwrite */
    if ( isSeqpacket( mSettings ) ) {
        type = SOCK_SEQPACKET;
    }

    domain = (SockAddr_isIPv6( &mSettings->peer ) ?
#ifdef HAVE_IPV6
                  AF_INET6
#else
                  AF_INET
#endif
                  : AF_INET);

#ifdef WIN32
    // TODO: SCTP
	// RSVP プロバイダを探す。

    mSettings->mSock = WIN32Socket(mSettings, domain, type, protocol, 0);
#else
    mSettings->mSock = socket( domain, type, protocol );
#endif
    WARN_errno( mSettings->mSock == INVALID_SOCKET, "socket" );

    SetSocketOptions( mSettings );


    SockAddr_localAddr( mSettings );
    if ( mSettings->mLocalhost != NULL ) {
        // bind socket to local address
        rc = bind( mSettings->mSock, (sockaddr*) &mSettings->local,
                   SockAddr_get_sizeof_sockaddr( &mSettings->local ) );
        WARN_errno( rc == SOCKET_ERROR, "bind" );
    }

    // connect socket
#ifdef WIN32
	/* Delete mQOS temp because build error. */
    rc = WSAConnect( mSettings->mSock, (sockaddr*) &mSettings->peer,
                     SockAddr_get_sizeof_sockaddr( &mSettings->peer ),
                     NULL, NULL, NULL/*&mSettings->mQOS*/, NULL);
#else /* WIN32 */
    rc = connect( mSettings->mSock, (sockaddr*) &mSettings->peer,
                  SockAddr_get_sizeof_sockaddr( &mSettings->peer ));
#endif /* WIN32 */
    FAIL_errno( rc == SOCKET_ERROR, "connect", mSettings );

    getsockname( mSettings->mSock, (sockaddr*) &mSettings->local,
                 &mSettings->size_local );
    getpeername( mSettings->mSock, (sockaddr*) &mSettings->peer,
                 &mSettings->size_peer );
} // end Connect

/* -------------------------------------------------------------------
 * Send a datagram on the socket. The datagram's contents should signify
 * a FIN to the application. Keep re-transmitting until an
 * acknowledgement datagram is received.
 * ------------------------------------------------------------------- */
void Client::write_UDP_FIN( ) {
    int rc, wc;
    fd_set readSet;
    struct timeval timeout;

    SOCKET lost_ssock = 0, lost_sock;
    int lost_port, buflen=1024, br;
#ifdef WIN32
    FILE *log_handler = NULL;
#else
    int log_handler = 0;
#endif /* WIN32 */
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    server_hdr *shdr;
    void *buf = NULL; /* NULL indicates no need for a buffer */

    shdr = (server_hdr*) ((UDP_datagram*)mBuf + 1);

    if (mSettings->lossPacketsFileName)
    {
#ifdef WIN32
        if ((log_handler = fopen(mSettings->lossPacketsFileName, "w" )) < 0)
#else
        if ((log_handler = open(mSettings->lossPacketsFileName,
                (O_CREAT | O_WRONLY | O_TRUNC),
                (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) < 0)
#endif /* WIN32 */
        {
            fprintf(stderr, "fopen error %s\n", strerror(errno));
            goto not_interested_in_packet_loss;
        }
        /*
         * Open up a TCP socket to receive the lost packet data on
         */
        memset(&local_addr, 0, sizeof(local_addr));
        if (getsockname(mSettings->mSock, (struct sockaddr *) &local_addr,
                    &addr_len) < 0)
        {
            fprintf(stderr, "getsockname error %s\n", strerror(errno));
            exit(1);
        }
        local_addr.sin_port = 0; /* allow binding to any local port */
        if ((lost_ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
            fprintf(stderr, "socket error %s\n", strerror(errno));
            exit(1);
        }
        if (bind(lost_ssock, (struct sockaddr *) &local_addr,
             sizeof(local_addr)) < 0)
        {
            fprintf(stderr, "bind error %s\n", strerror(errno));
            exit(1);
        }
        memset(&local_addr, 0, sizeof(local_addr));
        addr_len = sizeof(local_addr);
        if (getsockname(lost_ssock, (struct sockaddr *) &local_addr,
                    &addr_len) < 0)
        {
            fprintf(stderr, "getsockname error %s\n", strerror(errno));
            exit(1);
        }
        listen(lost_ssock, 1);
        lost_port = ntohs(local_addr.sin_port);
        printf("TCP socket bound on port %d\n", lost_port);

        shdr->lost_port = local_addr.sin_port; /* network format */

        if (!(buf = malloc(buflen)))
        {
            fprintf(stderr, "Out of memory %s\n", strerror(errno));
            exit(1);
        }

    }
    else
    {
not_interested_in_packet_loss:
        shdr->lost_port = 0; /* this indicates no interest in logging */
    }

    int count = 0;
    while ( count < 10 ) {
        count++;

        // write data
        wc = write( mSettings->mSock, mBuf, mSettings->mBufLen );
        WARN_errno( wc < 0, "write_UDP_FIN" );

        // wait until the socket is readable, or our timeout expires
        FD_ZERO( &readSet );
        FD_SET( mSettings->mSock, &readSet );
        timeout.tv_sec  = 0;
        timeout.tv_usec = 250000; // quarter second, 250 ms

        rc = select( (int) (mSettings->mSock+1), &readSet, NULL, NULL, &timeout );
        FAIL_errno( rc == SOCKET_ERROR, "select", mSettings );

        if ( rc == 0 ) {
            // select timed out
            continue;
        } else {
            // socket ready to read
            rc = read( mSettings->mSock, mBuf, mSettings->mBufLen );
            WARN_errno( rc < 0, "read" );
            if ( rc < 0 ) {
                break;
            } else if ( rc >= (int) (sizeof(UDP_datagram) + sizeof(server_hdr)) ) {
                ReportServerUDP( mSettings, (server_hdr*) ((UDP_datagram*)mBuf + 1) );
            }

            goto out_noerr;
        }
    }

    fprintf( stderr, warn_no_ack, mSettings->mSock, count );
    goto out;

out_noerr:
    if (!mSettings->lossPacketsFileName)
        goto out;
    /* accept and start reading from the TCP socket, dump to stderr */
    memset(&local_addr, 0, sizeof(struct sockaddr_in));
    addr_len = sizeof(local_addr);
    if ((lost_sock = accept(lost_ssock, (struct sockaddr *) &local_addr,
                            &addr_len)) < 0)
    {
        fprintf(stderr, "accept error %s\n", strerror(errno));
        exit(1);
    }
    for(;;)
    {
        br = read(lost_sock, buf, buflen);
        if (br == 0)
        {
            shutdown(lost_sock, SHUT_RDWR);
            break;
        }
        else if (br < 0)
        {
            fprintf(stderr, "recv error %s\n", strerror(errno));
            break;
        }
        else /* proper bytes were received */
        {
            /* the server will send properly formated strings */
            if (log_handler > 0) {
#ifdef WIN32
                wc = (int) fwrite(buf, sizeof(const char),  br,log_handler);
#else
                wc = write(log_handler, buf, br);
#endif
                WARN_errno( wc < 0, "write_UDP_FIN out_noerr" );
            }
        }
    }
#ifdef WIN32
    if (log_handler > 0)
        fclose(log_handler);
#else
    if (log_handler > 0)
        close(log_handler);
#endif
    close(lost_sock);

out:
    if (lost_ssock > 0)
        close(lost_ssock);
    if (buf)
        free(buf); /* not really necessary, since exiting */
}
// end write_UDP_FIN

/*---------------------------------------------------------------
 * Server.cpp
 * by Mark Gates <mgates@nlanr.net>
 *     Ajay Tirumala (tirumala@ncsa.uiuc.edu>.
 * -------------------------------------------------------------------
 * A server thread is initiated for each connection accept() returns.
 * Handles sending and receiving data, and then closes socket.
 * Changes to this version : The server can be run as a daemon
 * ------------------------------------------------------------------- */

#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "Server.h"
#include "List.h"
#include "Extractor.h"
#include "Reporter.h"
#include "Locale.h"

#include "queue.h"

/* -------------------------------------------------------------------
 * Stores connected socket and socket info.
 * ------------------------------------------------------------------- */

Server::Server( thread_Settings *inSettings ) {
    mSettings = inSettings;
    mBuf = NULL;

    // initialize buffer
    mBuf = new char[ mSettings->mBufLen ];
    FAIL_errno( mBuf == NULL, "No memory for buffer\n", mSettings );
}

/* -------------------------------------------------------------------
 * Destructor close socket.
 * ------------------------------------------------------------------- */

Server::~Server() {
    if ( mSettings->mSock != INVALID_SOCKET ) {
        int rc = close( mSettings->mSock );
        WARN_errno( rc == SOCKET_ERROR, "close" );
        mSettings->mSock = INVALID_SOCKET;
    }
    DELETE_ARRAY( mBuf );
}

void Server::Sig_Int( int inSigno ) {
}

/* -------------------------------------------------------------------
 * Receive data from the (connected) socket.
 * Sends termination flag several times at the end.
 * Does not close the socket.
 * ------------------------------------------------------------------- */
void Server::Run( void ) {
    long currLen;
    max_size_t totLen = 0;
    struct UDP_datagram* mBuf_UDP  = (struct UDP_datagram*) mBuf;

    ReportStruct *reportstruct = NULL;

    reportstruct = new ReportStruct;
    if ( reportstruct != NULL ) {
        reportstruct->packetID = 0;
        mSettings->reporthdr = InitReport( mSettings );
        do {
            // perform read
            currLen = recv( mSettings->mSock, mBuf, mSettings->mBufLen, 0 );

            if ( isUDP( mSettings ) ) {
                // read the datagram ID and sentTime out of the buffer
                reportstruct->packetID = ntohl( mBuf_UDP->id );
                reportstruct->sentTime.tv_sec = ntohl( mBuf_UDP->tv_sec  );
                reportstruct->sentTime.tv_usec = ntohl( mBuf_UDP->tv_usec );
                reportstruct->packetLen = currLen;
                gettimeofday( &(reportstruct->packetTime), NULL );
            } else {
                totLen += currLen;
            }

            // terminate when datagram begins with negative index
            // the datagram ID should be correct, just negated
            if ( reportstruct->packetID < 0 ) {
                reportstruct->packetID = -reportstruct->packetID;
                currLen = -1;
            }
            if ( isUDP (mSettings))
                ReportPacket( mSettings->reporthdr, reportstruct );
            else if ( !isUDP (mSettings) && mSettings->mInterval > 0) {
                reportstruct->packetLen = currLen;
                gettimeofday( &(reportstruct->packetTime), NULL );
                ReportPacket( mSettings->reporthdr, reportstruct );
            }
        } while ( currLen > 0 );

        // stop timing
        gettimeofday( &(reportstruct->packetTime), NULL );
        if ( !isUDP (mSettings)) {
            reportstruct->packetLen = totLen;
        }
        CloseReport( mSettings->reporthdr, reportstruct );

        // send a acknowledgement back only if we're NOT receiving multicast
        if ( isUDP( mSettings ) && !isMulticast( mSettings ) ) {
            // send back an acknowledgement of the terminating datagram
            write_UDP_AckFIN( );
        }
    } else {
        FAIL(1, "Out of memory! Closing server thread\n", mSettings);
    }
    Mutex_Lock( &clients_mutex );
    Iperf_delete( &(mSettings->peer), &clients );
    Mutex_Unlock( &clients_mutex );

    DELETE_PTR( reportstruct );
    EndReport( mSettings->reporthdr );
}
// end Recv

/* -------------------------------------------------------------------
 * Send an AckFIN (a datagram acknowledging a FIN) on the socket,
 * then select on the socket for some time. If additional datagrams
 * come in, probably our AckFIN was lost and they are re-transmitted
 * termination datagrams, so re-transmit our AckFIN.
 * ------------------------------------------------------------------- */
void Server::write_UDP_AckFIN( ) {

    int rc, wc;
    struct out_of_order_packet *oop, *otmp;
    struct lost_packet_interval *lpi, *ltmp;
    struct sockaddr_in local_addr, client_addr;
    int bw;
    SOCKET lost_sock = 0;
    int lost_port = 0, buflen=1024, datagrams = 0;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    void *buf = NULL;
    fd_set readSet;
    FD_ZERO( &readSet );

    struct timeval timeout;

    /* Report the list of lost packets, minus out of order packets */
    LIST_FOREACH_SAFE(lpi, &mSettings->reporthdr->report.lost_packets, list, ltmp)
    {
        LIST_FOREACH_SAFE(oop, &mSettings->reporthdr->report.out_of_order_packets, list, otmp)
        {
            if (oop->packetID >= lpi->from && oop->packetID <= lpi->to)
            {
                if (oop->packetID == lpi->from)
                {
                    if (lpi->from == lpi->to) /* garbage collect item */
                    {
                        LIST_REMOVE(lpi, list);
                        free(lpi);
                    }
                    else /* adjust the bounds */
                    lpi->from = (oop->packetID + 1);
                }
                else if (oop->packetID == lpi->to)
                {
                    if (lpi->from == lpi->to) /* garbage collect item */
                    {
                        LIST_REMOVE(lpi, list);
                        free(lpi);
                    }
                    else /* adjust the bounds */
                    lpi->to = (oop->packetID - 1);
                }
                else
                {
                    /* perform a split, by adding one more list item */
                    if (!(ltmp = (struct lost_packet_interval *)
                                    malloc(sizeof(struct lost_packet_interval))))
                    {
                        fprintf(stderr, "out of memory");
                        exit(1);
                    }
                    ltmp->from = oop->packetID + 1;
                    ltmp->to = lpi->to;
                    lpi->to = oop->packetID - 1;
                    /* list_add inserts the element AFTER the head (second param) */
                    LIST_INSERT_AFTER(lpi, ltmp, list);
                }
                LIST_REMOVE(oop, list);
                free(oop);
            }
        }
    }

    /* do not leak memory, in case out of order packets are received but no loss occured */
    LIST_FOREACH_SAFE(oop, &mSettings->reporthdr->report.out_of_order_packets, list, otmp)
    {
        LIST_REMOVE(oop, list);
        free(oop);
    }

    int count = 0;
    while ( count < 10 ) {
        count++;

        UDP_datagram *UDP_Hdr;
        server_hdr *hdr;

        UDP_Hdr = (UDP_datagram*) mBuf;

        if ( mSettings->mBufLen > (int) ( sizeof( UDP_datagram )
                + sizeof( server_hdr ) ) ) {
            Transfer_Info *stats = GetReport( mSettings->reporthdr );

            hdr = (server_hdr*) (UDP_Hdr+1);
            lost_port = ntohs(hdr->lost_port);

            hdr->flags        = htonl( HEADER_VERSION1 );
            hdr->total_len1   = htonl( (long) (stats->TotalLen >> 32) );
            hdr->total_len2   = htonl( (long) (stats->TotalLen & 0xFFFFFFFF) );
            hdr->stop_sec     = htonl( (long) stats->endTime );
            hdr->stop_usec    = htonl( (long)((stats->endTime - (long)stats->endTime)
                    * rMillion));
            hdr->error_cnt    = htonl( stats->cntError );
            hdr->outorder_cnt = htonl( stats->cntOutofOrder );
            hdr->datagrams    = htonl( stats->cntDatagrams );
            hdr->jitter1      = htonl( (long) stats->jitter );
            hdr->jitter2      = htonl( (long) ((stats->jitter - (long)stats->jitter)
                    * rMillion) );

            datagrams = ntohl(hdr->datagrams);
        }

        // write data
        wc = write( mSettings->mSock, mBuf, mSettings->mBufLen );
        WARN_errno( wc < 0, "write_UDP_AckFIN" );

        // wait until the socket is readable, or our timeout expires
        FD_SET( mSettings->mSock, &readSet );
        timeout.tv_sec  = 1;
        timeout.tv_usec = 0;

        rc = (int) select( (int)mSettings->mSock+1, &readSet, NULL, NULL, &timeout );
        FAIL_errno( rc == SOCKET_ERROR, "select", mSettings );

        if ( rc == 0 ) {
            // select timed out
            goto out_noerr;
        } else {
            // socket ready to read
            rc = read( mSettings->mSock, mBuf, mSettings->mBufLen );
            WARN_errno( rc < 0, "read" );
            if ( rc <= 0 ) {
                // Connection closed or errored
                // Stop using it.
                goto out_noerr;
            }
        }
    }

    fprintf( stderr, warn_ack_failed, mSettings->mSock, count );
    goto out;

out_noerr:
    if (lost_port == 0)
        goto out;
    memset(&local_addr, 0, sizeof(local_addr));
    if (getsockname(mSettings->mSock, (struct sockaddr *) &local_addr,
                    &addr_len) < 0)
    {
        fprintf(stderr, "getsockname error %s\n", strerror(errno));
        goto out;
    }
    memset(&client_addr, 0, sizeof(client_addr));
    addr_len = sizeof(client_addr);
    if (getpeername(mSettings->mSock, (struct sockaddr *) &client_addr,
                    &addr_len) < 0)
    {
        fprintf(stderr, "getpeername error %s\n", strerror(errno));
        goto out;
    }

    local_addr.sin_port = 0; /* allow binding to any local port */
    if ((lost_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        fprintf(stderr, "socket error %s\n", strerror(errno));
        goto out;
    }
    if (bind(lost_sock, (struct sockaddr *) &local_addr,
             sizeof(local_addr)) < 0)
    {
        fprintf(stderr, "bind error %s\n", strerror(errno));
        goto out;
    }
    if (!(buf = malloc(buflen)))
    {
        fprintf(stderr, "Out of memory %s\n", strerror(errno));
        goto out;
    }

    client_addr.sin_port = htons(lost_port);
    if (connect(lost_sock, (struct sockaddr *) &client_addr,
                sizeof(client_addr)) < 0)
    {
        fprintf(stderr, "connect error, %s\n", strerror(errno));
        goto out;
    }
    snprintf((char *) buf, buflen,  "LOST_GAPS DATAGRAMS_TOTAL %d\n", datagrams);
    bw = write(lost_sock, (char *) buf, strlen((char *)buf));
    LIST_FOREACH_SAFE(lpi, &mSettings->reporthdr->report.lost_packets, list, ltmp)
    {
        LIST_REMOVE(lpi, list);
        snprintf((char *)buf, buflen, "LOST_GAPS %d %d\n", lpi->from, lpi->to);
        bw = write(lost_sock, (char *) buf, strlen((char *)buf));
        if (bw < 0)
            fprintf(stderr, "send error %s\n", strerror(errno));
        /* printf("Sent %s", (char *) buf, strlen((char *) buf)); */
        free(lpi);
    }
    shutdown(lost_sock, SHUT_WR);

out:
    if (lost_sock > 0)
        close(lost_sock);
    /* do not leak memory */
    LIST_FOREACH_SAFE(lpi, &mSettings->reporthdr->report.lost_packets, list, ltmp)
    {
        LIST_REMOVE(lpi, list);
        free(lpi);
    }
    if (buf)
        free(buf);

}
 // end write_UDP_AckFIN


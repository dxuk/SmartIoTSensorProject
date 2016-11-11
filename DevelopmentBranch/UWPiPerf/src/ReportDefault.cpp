#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "ui_communication.h"

#include "Settings.h"
#include "util.h"
#include "Reporter.h"
#include "report_default.h"
#include "Thread.h"
#include "Locale.h"
#include "PerfSocket.h"
#include "SocketAddr.h"

/*
 * Prints transfer reports in default style
 */
void reporter_printstats( Transfer_Info *stats )
{
    static char header_printed = 0;

    byte_snprintf( buffer, sizeof(buffer)/2, (double) stats->TotalLen,
                   toupper( stats->mFormat));
    byte_snprintf( &buffer[sizeof(buffer)/2], sizeof(buffer)/2,
                   stats->TotalLen / (stats->endTime - stats->startTime),
                   stats->mFormat);

    if ( stats->mUDP != (char)kMode_Server ) {
        // TCP Reporting
        if( !header_printed ) {
            printf("%s", report_bw_header);
            header_printed = 1;
        }
        printf( report_bw_format, stats->transferID,
                stats->startTime, stats->endTime,
                buffer, &buffer[sizeof(buffer)/2] );
    } else {
        double average_delay;
        // UDP Reporting
        if( !header_printed ) {
            printf("%s", report_bw_jitter_loss_header);
            header_printed = 1;
        }
        if (stats->startTime!=0) {
            average_delay=1.0*stats->delay/(stats->cntDatagrams-stats->cntError);
        } else {
            average_delay=1.0*stats->delay_total/(stats->cntDatagrams-stats->cntError);
        }
        printf( report_bw_delay_jitter_loss_format, stats->transferID,
            stats->startTime, stats->endTime,
            buffer, &buffer[sizeof(buffer)/2],
            average_delay,
                stats->jitter*1000.0, stats->cntError, stats->cntDatagrams,
                (100.0 * stats->cntError) / stats->cntDatagrams );
        stats->delay=0;
        if ( stats->cntOutofOrder > 0 ) {
            printf( report_outoforder,
                    stats->transferID, stats->startTime,
                    stats->endTime, stats->cntOutofOrder );
        }
    }
    if ( stats->free == 1 && stats->mUDP == (char)kMode_Client ) {
        printf( report_datagrams, stats->transferID, stats->cntDatagrams );
    }
}


/*
 * Prints multiple transfer reports in default style
 */
void reporter_multistats( Transfer_Info *stats ) {

    byte_snprintf( buffer, sizeof(buffer)/2, (double) stats->TotalLen,
                   toupper( stats->mFormat));
    byte_snprintf( &buffer[sizeof(buffer)/2], sizeof(buffer)/2,
                   stats->TotalLen / (stats->endTime - stats->startTime),
                   stats->mFormat);

    if ( stats->mUDP != (char)kMode_Server ) {
        // TCP Reporting
        printf( report_sum_bw_format,
                stats->startTime, stats->endTime,
                buffer, &buffer[sizeof(buffer)/2] );
    } else {
        // UDP Reporting
        printf( report_sum_bw_jitter_loss_format,
                stats->startTime, stats->endTime,
                buffer, &buffer[sizeof(buffer)/2],
                stats->jitter*1000.0, stats->cntError, stats->cntDatagrams,
                (100.0 * stats->cntError) / stats->cntDatagrams );
        if ( stats->cntOutofOrder > 0 ) {
            printf( report_sum_outoforder,
                    stats->startTime,
                    stats->endTime, stats->cntOutofOrder );
        }
    }
    if ( stats->free == 1 && stats->mUDP == (char)kMode_Client ) {
        printf( report_sum_datagrams, stats->cntDatagrams );
    }
}

/*
 * Prints server transfer reports in default style
 */
void reporter_serverstats( Connection_Info *nused, Transfer_Info *stats ) {
    printf( server_reporting, stats->transferID );
    reporter_printstats( stats );
}

/*
 * Report the client or listener Settings in default style
 */
#if 0 /* template */

    if ( isSCTP( data ) ) {
        /* SCTP */
    } else if(! isUDP( data )) {
        /* TCP */
    } else {
        /* UDP */
    }

#endif /* template */

void reporter_reportsettings( ReporterData *data ) {
    int win, win_requested;

    win = getsock_tcp_windowsize( data->info.transferID,
                  (data->mThreadMode == kMode_Listener ? 0 : 1) );
    win_requested = data->mTCPWin;

    printf("%s", separator_line );
    if ( data->mThreadMode == kMode_Listener ) {
        if ( isSCTP( data ) ) {
            printf( server_port, "SCTP", data->mPort );
        } else if(! isUDP( data )) {
            printf( server_port, "TCP", data->mPort );
        } else {
            printf( server_port, "UDP", data->mPort );
        }
    } else {
        if ( isSCTP( data ) ) {
            printf( client_port, data->mHost, "SCTP", data->mPort );
        } else if(! isUDP( data )) {
            printf( client_port, data->mHost, "TCP", data->mPort );
        } else {
            printf( client_port, data->mHost, "UDP", data->mPort );
        }
    }
    if ( data->mLocalhost != NULL ) {
        printf( bind_address, data->mLocalhost );
        if ( SockAddr_isMulticast( &data->connection.local ) ) {
            printf( join_multicast, data->mLocalhost );
        }
    }

    if ( isUDP( data ) ) {
        printf( (data->mThreadMode == kMode_Listener ?
                                   server_datagram_size : client_datagram_size),
                data->mBufLen );
        if ( SockAddr_isMulticast( &data->connection.peer ) ) {
            printf( multicast_ttl, data->info.mTTL);
        }
    }
    byte_snprintf( buffer, sizeof(buffer), win,
                   toupper( data->info.mFormat));

    if ( isSCTP( data ) ) {
        /* SCTP */
        printf( "%s: %s", sctp_window_size, buffer );
    } else if(! isUDP( data )) {
        /* TCP */
        printf( "%s: %s", tcp_window_size, buffer );
    } else {
        /* UDP */
        printf( "%s: %s", udp_buffer_size, buffer );
    }

    if ( win_requested == 0 ) {
        printf( " %s", window_default );
    } else if ( win != win_requested ) {
        byte_snprintf( buffer, sizeof(buffer), win_requested,
                       toupper( data->info.mFormat));
        printf( warn_window_requested, buffer );
    }
    printf("%s", "\n" );
    printf("%s", separator_line );
}

/*
 * Report a socket's peer IP address in default style
 */
void *reporter_reportpeer( Connection_Info *stats, SOCKET ID ) {
    if ( ID > 0 ) {
        // copy the inet_ntop into temp buffers, to avoid overwriting
        char local_addr[ REPORT_ADDRLEN ];
        char remote_addr[ REPORT_ADDRLEN ];
        struct sockaddr *local = ((struct sockaddr*)&stats->local);
        struct sockaddr *peer = ((struct sockaddr*)&stats->peer);

        if ( local->sa_family == AF_INET ) {
            inet_ntop( AF_INET, &((struct sockaddr_in*)local)->sin_addr,
                       local_addr, REPORT_ADDRLEN);
        }
#ifdef HAVE_IPV6
          else {
            inet_ntop( AF_INET6, &((struct sockaddr_in6*)local)->sin6_addr,
                       local_addr, REPORT_ADDRLEN);
        }
#endif

        if ( peer->sa_family == AF_INET ) {
            inet_ntop( AF_INET, &((struct sockaddr_in*)peer)->sin_addr,
                       remote_addr, REPORT_ADDRLEN);
        }
#ifdef HAVE_IPV6
          else {
            inet_ntop( AF_INET6, &((struct sockaddr_in6*)peer)->sin6_addr,
                       remote_addr, REPORT_ADDRLEN);
        }
#endif

        printf( report_peer,
                ID,
                local_addr,  ( local->sa_family == AF_INET ?
                              ntohs(((struct sockaddr_in*)local)->sin_port) :
#ifdef HAVE_IPV6
                              ntohs(((struct sockaddr_in6*)local)->sin6_port)),
#else
                              0),
#endif
                remote_addr, ( peer->sa_family == AF_INET ?
                              ntohs(((struct sockaddr_in*)peer)->sin_port) :
#ifdef HAVE_IPV6
                              ntohs(((struct sockaddr_in6*)peer)->sin6_port)));
#else
                              0));
#endif
    }
    return NULL;
}
// end ReportPeer

/* -------------------------------------------------------------------
 * Report the MSS and MTU, given the MSS (or a guess thereof)
 * ------------------------------------------------------------------- */

// compare the MSS against the (MTU - 40) to (MTU - 80) bytes.
// 40 byte IP header and somewhat arbitrarily, 40 more bytes of IP options.

#define checkMSS_MTU( inMSS, inMTU ) (inMTU-40) >= inMSS  &&  inMSS >= (inMTU-80)

void reporter_reportMSS( int inMSS, thread_Settings *inSettings ) {
    if ( inMSS <= 0 ) {
        printf( report_mss_unsupported, inSettings->mSock );
    } else {
        const char* net; /* FIX: compiler warn cast to const char* */
        int mtu = 0;

        if ( checkMSS_MTU( inMSS, 1500 ) ) {
            net = "ethernet";
            mtu = 1500;
        } else if ( checkMSS_MTU( inMSS, 4352 ) ) {
            net = "FDDI";
            mtu = 4352;
        } else if ( checkMSS_MTU( inMSS, 9180 ) ) {
            net = "ATM";
            mtu = 9180;
        } else if ( checkMSS_MTU( inMSS, 65280 ) ) {
            net = "HIPPI";
            mtu = 65280;
        } else if ( checkMSS_MTU( inMSS, 576 ) ) {
            net = "minimum";
            mtu = 576;
            printf("%s", warn_no_pathmtu );
        } else {
            mtu = inMSS + 40;
            net = "unknown interface";
        }

        printf( report_mss,
                inSettings->mSock, inMSS, mtu, net );
    }
}
// end ReportMSS


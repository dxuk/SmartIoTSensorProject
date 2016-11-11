#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "ui_communication.h"

#include "Settings.h"
#include "util.h"
#include "Reporter.h"
#include "report_CSV.h"
#include "Locale.h"

void CSV_timestamp( char *timestamp, int length );

void CSV_stats( Transfer_Info *stats ) {
    // $TIMESTAMP,$ID,$INTERVAL,$BYTE,$SPEED,$JITTER,$LOSS,$PACKET,$%LOSS
    max_size_t speed = (max_size_t)(((double)stats->TotalLen * 8.0) / (stats->endTime - stats->startTime));
    char timestamp[16];
    CSV_timestamp( timestamp, sizeof(timestamp) );
    if ( stats->mUDP != (char)kMode_Server ) {
        // TCP Reporting
        printf( reportCSV_bw_format,
                timestamp,
                (stats->reserved_delay == NULL ? ",,," : stats->reserved_delay),
                stats->transferID,
                stats->startTime,
                stats->endTime,
                stats->TotalLen,
                speed);
    } else {
        // UDP Reporting
        printf( reportCSV_bw_jitter_loss_format,
                timestamp,
                (stats->reserved_delay == NULL ? ",,," : stats->reserved_delay),
                stats->transferID,
                stats->startTime,
                stats->endTime,
                stats->TotalLen,
                speed,
                stats->jitter*1000.0,
                stats->cntError,
                stats->cntDatagrams,
                (100.0 * stats->cntError) / stats->cntDatagrams, stats->cntOutofOrder );
    }
    if ( stats->free == 1 && stats->reserved_delay != NULL ) {
        free( stats->reserved_delay );
    }
}

void *CSV_peer( Connection_Info *stats, SOCKET ID ) {

    // copy the inet_ntop into temp buffers, to avoid overwriting
    char local_addr[ REPORT_ADDRLEN ];
    char remote_addr[ REPORT_ADDRLEN ];
    char *buf = (char *) malloc( REPORT_ADDRLEN*2 + 10 );
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

    snprintf(buf, REPORT_ADDRLEN*2+10, reportCSV_peer,
             local_addr, ( local->sa_family == AF_INET ?
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
    return buf;
}

void CSV_serverstats( Connection_Info *conn, Transfer_Info *stats ) {
    stats->reserved_delay = CSV_peer( conn, stats->transferID );
    stats->free = 1;
    CSV_stats( stats );
}

void CSV_timestamp( char *timestamp, int length ) {
    time_t times;
    struct tm *timest;
    times = time( NULL );
    timest = localtime( &times );
    strftime( timestamp, length,"%Y%m%d%H%M%S", timest );
}

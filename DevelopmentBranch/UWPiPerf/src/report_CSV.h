#ifndef REPORT_CSV_H
#define REPORT_CSV_H

void CSV_stats( Transfer_Info *stats );
void *CSV_peer( Connection_Info *stats, SOCKET ID);
void CSV_serverstats( Connection_Info *conn, Transfer_Info *stats );


#endif // REPORT_CSV_H

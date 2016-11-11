#ifndef REPORT_DEFAULT_H
#define REPORT_DEFAULT_H

void reporter_printstats( Transfer_Info *stats );
void reporter_multistats( Transfer_Info *stats );
void reporter_serverstats( Connection_Info *conn, Transfer_Info *stats );
void reporter_reportsettings( ReporterData *stats );
void *reporter_reportpeer( Connection_Info *stats, SOCKET ID);


#endif // REPORT_DEFAULT_H

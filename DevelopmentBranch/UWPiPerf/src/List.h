#ifndef Iperf_LIST_H
#define Iperf_LIST_H

#include "headers.h"
#include "Settings.h"
#include "Reporter.h"
#include "Mutex.h"

/*
 * List handling utilities to replace STD vector
 */

struct Iperf_ListEntry;

/*
 * A List entry that consists of a sockaddr
 * a pointer to the Audience that sockaddr is
 * associated with and a pointer to the next
 * entry
 */
struct Iperf_ListEntry {
    iperf_sockaddr data;
    MultiHeader *holder;
    thread_Settings *server;
    Iperf_ListEntry *next;
};

extern Mutex clients_mutex;
extern Iperf_ListEntry *clients;

/*
 * Functions to modify or search the List
 */
void Iperf_pushback ( Iperf_ListEntry *add, Iperf_ListEntry **root );

void Iperf_delete ( iperf_sockaddr *del, Iperf_ListEntry **root );

void Iperf_destroy ( Iperf_ListEntry **root );

Iperf_ListEntry* Iperf_present ( iperf_sockaddr *find, Iperf_ListEntry *root );

Iperf_ListEntry* Iperf_hostpresent ( iperf_sockaddr *find, Iperf_ListEntry *root );

#endif

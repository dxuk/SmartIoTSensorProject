#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "List.h"
#include "Mutex.h"
#include "SocketAddr.h"

/*
 * Global List and Mutex variables
 */
Iperf_ListEntry *clients = NULL;
Mutex clients_mutex;

/*
 * Add Entry add to the List
 */
void Iperf_pushback ( Iperf_ListEntry *add, Iperf_ListEntry **root ) {
    add->next = *root;
    *root = add;
}

/*
 * Delete Entry del from the List
 */
void Iperf_delete ( iperf_sockaddr *del, Iperf_ListEntry **root ) {
    Iperf_ListEntry *temp = Iperf_present( del, *root );
    if ( temp != NULL ) {
        if ( temp == *root ) {
            *root = (*root)->next;
        } else {
            Iperf_ListEntry *itr = *root;
            while ( itr->next != NULL ) {
                if ( itr->next == temp ) {
                    itr->next = itr->next->next;
                    break;
                }
                itr = itr->next;
            }
        }
        delete temp;
    }
}

/*
 * Destroy the List (cleanup function)
 */
void Iperf_destroy ( Iperf_ListEntry **root ) {
    Iperf_ListEntry *itr1 = *root, *itr2;
    while ( itr1 != NULL ) {
        itr2 = itr1->next;
        delete itr1;
        itr1 = itr2;
    }
    *root = NULL;
}

/*
 * Check if the exact Entry find is present
 */
Iperf_ListEntry* Iperf_present ( iperf_sockaddr *find, Iperf_ListEntry *root ) {
    Iperf_ListEntry *itr = root;
    while ( itr != NULL ) {
        if ( SockAddr_are_Equal( (sockaddr*)itr, (sockaddr*)find ) ) {
            return itr;
        }
        itr = itr->next;
    }
    return NULL;
}

/*
 * Check if a Entry find is in the List or if any
 * Entry exists that has the same host as the
 * Entry find
 */
Iperf_ListEntry* Iperf_hostpresent ( iperf_sockaddr *find, Iperf_ListEntry *root ) {
    Iperf_ListEntry *itr = root;
    while ( itr != NULL ) {
        if ( SockAddr_Hostare_Equal( (sockaddr*)itr, (sockaddr*)find ) ) {
            return itr;
        }
        itr = itr->next;
    }
    return NULL;
}

/*---------------------------------------------------------------
 * Listener sets up a socket listening on the server host. For each
 * connected socket that accept() returns, this creates a Server
 * socket and spawns a thread for it.
 * ------------------------------------------------------------------- */

#ifndef LISTENER_H
#define LISTENER_H

#include "Thread.h"
#include "Settings.h"

class Listener;

class Listener {
public:
    // stores server port and TCP/UDP mode
    Listener( thread_Settings *inSettings );

    // destroy the server object
    ~Listener();

    // accepts connections and starts Servers
    void Run( void );

    // Starts the Servers as a daemon
    void runAsDaemon( const char *, int );

    void Listen( );

    void McastJoin( );

    void McastSetTTL( int val );

    void Accept( thread_Settings *server );

    void UDPSingleServer ();

protected:
    int mClients;
    char* mBuf;
    thread_Settings *mSettings;
    thread_Settings *server;

}; // end class Listener

#endif // LISTENER_H

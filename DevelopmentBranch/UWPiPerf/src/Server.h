/*---------------------------------------------------------------
 * A server thread is initiated for each connection accept() returns.
 * Handles sending and receiving data, and then closes socket.
 * ------------------------------------------------------------------- */

#ifndef SERVER_H
#define SERVER_H


#include "Settings.h"
#include "util.h"
#include "Timestamp.h"

/* ------------------------------------------------------------------- */
class Server {
public:
    // stores server socket, port and TCP/UDP mode
    Server( thread_Settings *inSettings );

    // destroy the server object
    ~Server();

    // accepts connection and receives data
    void Run( void );

    void write_UDP_AckFIN( );

    static void Sig_Int( int inSigno );

private:
    thread_Settings *mSettings;
    char* mBuf;
    Timestamp mEndTime;

}; // end class Server

#endif // SERVER_H

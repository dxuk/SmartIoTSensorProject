/*---------------------------------------------------------------
 * A client thread initiates a connect to the server and handles
 * sending and receiving data, then closes the socket.
 * ------------------------------------------------------------------- */

#ifndef CLIENT_H
#define CLIENT_H

/* for direct file operations */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Settings.h"
#include "Timestamp.h"

/* -------------------------------------------------------------------*/
class Client {
public:
    // stores server hostname, port, UDP/TCP mode, and UDP rate
    Client( thread_Settings *inSettings );

    // destroy the client object
    ~Client();

    // connects and sends data
    void Run( void );

    // TCP specific version of above
    void RunTCP( void );

    void InitiateServer();

    // UDP / TCP
    void Send( void );

    void write_UDP_FIN( );

    // client connect
    void Connect( );

protected:
    thread_Settings *mSettings;
    char* mBuf;
    Timestamp mEndTime;
    Timestamp lastPacketTime;

}; // end class Client

#endif // CLIENT_H

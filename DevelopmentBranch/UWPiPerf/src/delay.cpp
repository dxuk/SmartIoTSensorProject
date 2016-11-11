/*---------------------------------------------------------------
 * accurate microsecond delay
 * ------------------------------------------------------------------- */

#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "ui_communication.h"

#include "Timestamp.h"
#include "delay.h"

#ifndef WIN32
/* -------------------------------------------------------------------
 * determine the timer resolution of nanosleep
 * ------------------------------------------------------------------- */
static long measure_nanosleep_threshold()
{
    timespec req = {0, 1000};
    Timestamp bg, ed;
    bg.setnow();
    for (int i = 0; i < 20; ++i) {
        nanosleep(&req, NULL);
    }
    ed.setnow();
    long diff = ed.subUsec(bg) / 20;
    if (diff <= 20) {
        diff = 10;
    } else if (diff <= 200 ) {
        diff = 10 * ( (diff+5) / 10 ) - 5;
    } else if (diff <= 600 ) {
        diff = 50 * ( (diff+25) / 50 ) - 50;
    } else if (diff <= 2000 ) {
        diff = 100 * ( (diff+50) / 100 ) - 50;
    } else {
        diff = 500 * ( (diff+250) / 500 ) - 50;
    }
    //printf("thr %ld\n", diff);
    return diff;
}
static long nanosleep_threshold = measure_nanosleep_threshold();

/* -------------------------------------------------------------------
 * A micro-second delay function. This uses gettimeofday (underneith
 * the Timestamp) which has a resolution of upto microseconds. I've
 * found it's good to within about 10 usecs.
 * I used to do calibration, but iperf automatically adjusts itself
 * so that isn't necesary, and it causes some problems if the
 * calibration adjustment is larger than your sleep time.
 * ------------------------------------------------------------------- */

void delay_loop( unsigned long usec ) {
    Timestamp end;
    end.add( usec * 1e-6 );

    Timestamp now;
    long diff;
    while ( (diff = end.subUsec(now)) > 0 ) {
        if (diff >= nanosleep_threshold) {
/* convert to seconds; nanosleep requires 0 <= tv_nsec <= 999999999 */
            timespec req = { usec / 1000000UL,
                            (usec % 1000000UL) * 1000UL };
            nanosleep(&req, NULL);
        } else {
            sched_yield();
        }
        now.setnow();
    }
}
#else
void delay_loop( unsigned long usec ) {

    LARGE_INTEGER freq, start, now;

    if (!QueryPerformanceFrequency(&freq))
    {
        printf("qpfreq err\n");//xxx
    }
    QueryPerformanceCounter(&start);
    for(;;) {
        QueryPerformanceCounter((LARGE_INTEGER*) &now);
        if( ((double)(now.QuadPart - start.QuadPart) / (double)freq.QuadPart)  * 1000000 > usec ) break;
    }
}
#endif

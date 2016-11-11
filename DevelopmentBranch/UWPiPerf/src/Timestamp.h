/*---------------------------------------------------------------
 * A generic interface to a timestamp.
 * This implementation uses the unix gettimeofday().
 * ------------------------------------------------------------------- */

#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#define HEADERS()

#include "headers.h"

/* ------------------------------------------------------------------- */
class Timestamp {
public:
    /* -------------------------------------------------------------------
     * Create a timestamp, with the current time in it.
     * ------------------------------------------------------------------- */
    Timestamp( void ) {
        setnow();
    }

    /* -------------------------------------------------------------------
     * Create a timestamp, with the given seconds/microseconds
     * ------------------------------------------------------------------- */
    Timestamp( long sec, long usec ) {
        set( sec, usec );
    }

    /* -------------------------------------------------------------------
     * Create a timestamp, with the given seconds
     * ------------------------------------------------------------------- */
    Timestamp( double sec ) {
        set( sec );
    }

    /* -------------------------------------------------------------------
     * Set timestamp to current time.
     * ------------------------------------------------------------------- */
    void setnow( void ) {
        gettimeofday( &mTime, NULL );
    }

    /* -------------------------------------------------------------------
     * Set timestamp to the given seconds/microseconds
     * ------------------------------------------------------------------- */
    void set( long sec, long usec ) {
        assert( sec  >= 0 );
        assert( usec >= 0  &&  usec < kMillion );

        mTime.tv_sec  = sec;
        mTime.tv_usec = usec;
    }

    /* -------------------------------------------------------------------
     * Set timestamp to the given seconds
     * ------------------------------------------------------------------- */
    void set( double sec ) {
        mTime.tv_sec  = (long) sec;
        mTime.tv_usec = (long) ((sec - mTime.tv_sec) * kMillion);
    }

    /* -------------------------------------------------------------------
     * return seconds portion of timestamp
     * ------------------------------------------------------------------- */
    long getSecs( void ) {
        return mTime.tv_sec;
    }

    /* -------------------------------------------------------------------
     * return microseconds portion of timestamp
     * ------------------------------------------------------------------- */
    long getUsecs( void ) {
        return mTime.tv_usec;
    }

    /* -------------------------------------------------------------------
     * return timestamp as a floating point seconds
     * ------------------------------------------------------------------- */
    double get( void ) {
        return mTime.tv_sec + mTime.tv_usec / ((double) kMillion);
    }

    /* -------------------------------------------------------------------
     * subtract the right timestamp from my timestamp.
     * return the difference in microseconds.
     * ------------------------------------------------------------------- */
    long subUsec( Timestamp right ) {
        return(mTime.tv_sec  - right.mTime.tv_sec) * kMillion +
        (mTime.tv_usec - right.mTime.tv_usec);
    }

    /* -------------------------------------------------------------------
     * subtract the right timestamp from my timestamp.
     * return the difference in microseconds.
     * ------------------------------------------------------------------- */
    long subUsec( timeval right ) {
        return(mTime.tv_sec  - right.tv_sec) * kMillion +
        (mTime.tv_usec - right.tv_usec);
    }

    /* -------------------------------------------------------------------
     * Return the number of microseconds from now to last time of setting.
     * ------------------------------------------------------------------- */
    long delta_usec(void) {
        struct timeval previous = mTime;

        setnow();
        return subUsec(previous);
    }

    /* -------------------------------------------------------------------
     * subtract the right timestamp from my timestamp.
     * return the difference in seconds as a floating point.
     * ------------------------------------------------------------------- */
    double subSec( Timestamp right ) {
        return(mTime.tv_sec  - right.mTime.tv_sec) +
        (mTime.tv_usec - right.mTime.tv_usec) / ((double) kMillion);
    }

    /* -------------------------------------------------------------------
     * add the right timestamp to my timestamp.
     * ------------------------------------------------------------------- */
    void add( Timestamp right ) {
        mTime.tv_sec  += right.mTime.tv_sec;
        mTime.tv_usec += right.mTime.tv_usec;

        // watch for under- and overflow
        if ( mTime.tv_usec < 0 ) {
            mTime.tv_usec += kMillion;
            mTime.tv_sec--;
        }
        if ( mTime.tv_usec >= kMillion ) {
            mTime.tv_usec -= kMillion;
            mTime.tv_sec++;
        }

        assert( mTime.tv_usec >= 0  &&
                mTime.tv_usec <  kMillion );
    }

    /* -------------------------------------------------------------------
     * add the seconds to my timestamp.
     * TODO optimize?
     * ------------------------------------------------------------------- */
    void add( double sec ) {
        mTime.tv_sec  += (long) sec;
        mTime.tv_usec += (long) ((sec - ((long) sec )) * kMillion);

        // watch for overflow
        if ( mTime.tv_usec >= kMillion ) {
            mTime.tv_usec -= kMillion;
            mTime.tv_sec++;
        }

        assert( mTime.tv_usec >= 0  &&
                mTime.tv_usec <  kMillion );
    }

    /* -------------------------------------------------------------------
     * return true if my timestamp is before the right timestamp.
     * ------------------------------------------------------------------- */
    bool before( timeval right ) {
        return mTime.tv_sec < right.tv_sec  ||
        (mTime.tv_sec == right.tv_sec &&
         mTime.tv_usec < right.tv_usec);
    }
    bool before( Timestamp right ) { return before(right.mTime); }

    /* -------------------------------------------------------------------
     * return true if my timestamp is after the right timestamp.
     * ------------------------------------------------------------------- */
    bool after( timeval right ) {
        return mTime.tv_sec > right.tv_sec  ||
        (mTime.tv_sec == right.tv_sec &&
         mTime.tv_usec > right.tv_usec);
    }
    bool after( Timestamp right ) { return after(right.mTime); }

    /**
     * This function returns the fraction of time elapsed after the beginning
     * till the end
     */
    double fraction(Timestamp currentTime, Timestamp endTime) {
        if ( (currentTime.after(*this)) && (endTime.after(currentTime)) ) {
            return(((double)currentTime.subUsec(*this)) /
                   ((double)endTime.subUsec(*this)));
        } else {
            return -1.0;
        }
    }


protected:
    enum {
        kMillion = 1000000
    };

    struct timeval mTime;

}; // end class Timestamp

#endif // TIMESTAMP_H

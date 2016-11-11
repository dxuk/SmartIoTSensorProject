/*---------------------------------------------------------------
 * An implementation of gettimeofday for Windows.
 * ------------------------------------------------------------------- */

#ifndef GETTIMEOFDAY_H
#define GETTIMEOFDAY_H

#ifndef HAVE_GETTIMEOFDAY

int gettimeofday( struct timeval* tv, void* timezone );

#endif /* HAVE_GETTIMEOFDAY */
#endif /* GETTIMEOFDAY_H */

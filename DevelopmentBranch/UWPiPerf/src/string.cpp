/*---------------------------------------------------------------
 * various string utilities
 * ------------------------------------------------------------------- */

#include "pch.h"

#define HEADERS()

#include "headers.h"

#include "util.h"

/* -------------------------------------------------------------------
 * pattern
 *
 * Initialize the buffer with a pattern of (index mod 10).
 * ------------------------------------------------------------------- */

void pattern( char *outBuf, int inBytes ) {
    assert( outBuf != NULL );

    while ( inBytes-- > 0 ) {
        outBuf[ inBytes ] = (inBytes % 10) + '0';
    }
} /* end pattern */


/*---------------------------------------------------------------
 * snprintf.c
 *
 * Throughout the book I use snprintf() because it's safer than sprintf().
 * But as of the time of this writing, not all systems provide this
 * function.  The function below should only be built on those systems
 * that do not provide a real snprintf().
 * The function below just acts like sprintf(); it is not safe, but it
 * tries to detect overflow.
 * ------------------------------------------------------------------- */

#include "pch.h"

#define HEADERS()

#include "headers.h"

#ifndef HAVE_SNPRINTF

    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>
    #include <stdarg.h>

    #include "snprintf.h"

/* Delete snprintf temp because build error. */
#if 0
int snprintf(char *buf, size_t size, const char *fmt, ...) {
    int n;
    va_list ap;

    va_start(ap, fmt);
#ifdef HAVE_VSNPRINTF
    vsnprintf(buf, size, fmt, ap); /* Sigh, some vsprintf's return ptr, not length */
#else
    vsprintf(buf, fmt, ap); /* Sigh, some vsprintf's return ptr, not length */
#endif
    n = (int) strlen(buf);
    va_end(ap);

    if ( n < 0 || n >= (int) size ) {
        fprintf( stderr, "snprintf: overflowed array\n" );
        exit(1);
    }

    return(n);
}
#endif

#endif /* HAVE_SNPRINTF */

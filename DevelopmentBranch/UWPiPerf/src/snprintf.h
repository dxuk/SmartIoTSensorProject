/*---------------------------------------------------------------
 * snprintf.h
 * to use this prototype, make sure HAVE_SNPRINTF is not defined
 * ------------------------------------------------------------------- */

#ifndef SNPRINTF_H
#define SNPRINTF_H
/*
 * Throughout the book I use snprintf() because it's safer than sprintf().
 * But as of the time of this writing, not all systems provide this
 * function.  The function below should only be built on those systems
 * that do not provide a real snprintf().
 * The function below just acts like sprintf(); it is not safe, but it
 * tries to detect overflow.
 */
#ifndef HAVE_SNPRINTF

int snprintf(char *buf, size_t size, const char *fmt, ...);

#endif /* HAVE_SNPRINTF */
#endif /* SNPRINTF_H */

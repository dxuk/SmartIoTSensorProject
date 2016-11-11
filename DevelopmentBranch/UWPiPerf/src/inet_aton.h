/*---------------------------------------------------------------
 * to use this prototype, make sure HAVE_INET_PTON is not defined
 * to use this prototype, make sure HAVE_INET_NTOP is not defined
 * ------------------------------------------------------------------- */

#ifndef INET_ATON_H
#define INET_ATON_H

#include "headers.h"

/*
 * inet_pton is the new, better version of inet_aton.
 * inet_aton is not IP version agnostic.
 * inet_aton is the new, better version of inet_addr.
 * inet_addr is incorrect in that it returns -1 as an error value,
 * while -1 (0xFFFFFFFF) is a valid IP address (255.255.255.255).
 */
#ifndef HAVE_INET_NTOP
int inet_ntop(int af, const void *src, char *dst, size_t size);
int inet_ntop4(const unsigned char *src, char *dst,
                      size_t size);
#ifdef HAVE_IPV6
int inet_ntop6(const unsigned char *src, char *dst,
                      size_t size);
#endif
#endif /* HAVE_INET_NTOP */

#ifndef HAVE_INET_PTON
int inet_pton(int af, const char *src, void *dst);
int inet_pton4(const char *src, unsigned char *dst);
#ifdef HAVE_IPV6
int inet_pton6(const char *src, unsigned char *dst);
#endif
#endif /* HAVE_INET_PTON */

#endif /* INET_ATON_H */


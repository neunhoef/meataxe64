/*
 * $Id: spanmsp.h,v 1.1 2002/09/05 18:24:26 jon Exp $
 *
 * Function to spin from a span under multiple generators until a proper subspace is found
 *
 */

#ifndef included__spanmsp
#define included__spanmsp

extern unsigned int spanmspin(const char *in, const char *out,
                              unsigned int argc, const char * const args[],
                              const char *name);

#endif

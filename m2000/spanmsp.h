/*
 * $Id: spanmsp.h,v 1.2 2005/06/22 21:52:54 jon Exp $
 *
 * Function to spin from a span under multiple generators until a proper subspace is found
 *
 */

#ifndef included__spanmsp
#define included__spanmsp

extern u32 spanmspin(const char *in, const char *out,
                     unsigned int argc, const char * const args[],
                     const char *name);

#endif

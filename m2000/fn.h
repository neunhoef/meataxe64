/*
 * $Id: fn.h,v 1.1 2006/05/09 22:04:10 jon Exp $
 *
 * Function to filter a list based on nullity
 *
 */

#ifndef included__fn
#define included__fn

extern int filter_nullity(const char *in_list, const char *out_list, u32 nullity,
                          unsigned int argc, const char * const args[],
                          const char *name);

#endif

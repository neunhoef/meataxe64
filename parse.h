/*
 * $Id: parse.h,v 1.2 2002/10/12 14:17:06 jon Exp $
 *
 * Function to parse command line flags
 *
 */

#ifndef included__parse
#define included__parse

extern int verbose;

extern unsigned int memory;

/* NULL return means parse failure, already reported */
extern const char *const *parse_line(int argc, const char *const argv[], int *new_argc);

#endif

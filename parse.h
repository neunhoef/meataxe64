/*
 * $Id: parse.h,v 1.1 2002/07/09 09:08:12 jon Exp $
 *
 * Function to parse command line flags
 *
 */

#ifndef included__parse
#define included__parse

extern int verbose;

/* NULL return means parse failure, already reported */
extern const char *const *parse_line(int argc, const char *const argv[], int *new_argc);

#endif

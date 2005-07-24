/*
 * $Id: parse.h,v 1.7 2005/07/24 11:31:35 jon Exp $
 *
 * Function to parse command line flags
 *
 */

#ifndef included__parse
#define included__parse

extern u32 verbose;

extern u32 memory;

extern u32 max_grease;

extern unsigned int maximum_rows;

/* NULL return means parse failure, already reported */
extern const char *const *parse_line(int argc, const char *const argv[], int *new_argc);

/* Return a string giving the stuff we can parse for inclusion in usage */
extern const char *parse_usage(void);

#endif

/*
 * $Id: parse.h,v 1.9 2005/12/21 18:12:22 jon Exp $
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

extern unsigned int print_stats;

/* These values will only be computed if the system is compiled for statistics gathering */
/* They will be printed in stats mode */
extern double adds, incs, scaled_adds, scaled_incs;

/* NULL return means parse failure, already reported */
extern const char *const *parse_line(int argc, const char *const argv[], int *new_argc);

/* Return a string giving the stuff we can parse for inclusion in usage */
extern const char *parse_usage(void);

#endif

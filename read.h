/*
 * $Id: read.h,v 1.4 2001/10/16 22:55:53 jon Exp $
 *
 * Reading a header for meataxe
 *
 */

#ifndef included__read
#define included__read

#include <stdio.h>

extern int read_text_header(FILE *, const header **, const char *);
extern int read_binary_header(FILE *, const header **, const char *);
extern int read_text_header_items(FILE *, unsigned int *nod, unsigned int *prime,
                                  unsigned int *nor, unsigned int *noc, const char *);

#endif

/*
 * $Id: read.h,v 1.6 2001/11/25 12:44:33 jon Exp $
 *
 * Reading a header for meataxe
 *
 */

#ifndef included__read
#define included__read

#include <stdio.h>
#include "header.h"

extern int read_text_header(FILE *, const header **, const char *);
extern int read_binary_header(FILE *, const header **, const char *);
extern int read_text_header_items(FILE *, unsigned int *nod, unsigned int *prime,
                                  unsigned int *nor, unsigned int *noc, const char *);
extern int open_and_read_binary_header(FILE **, const header **, const char *, const char *);

#endif

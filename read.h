/*
 * $Id: read.h,v 1.5 2001/11/07 22:35:27 jon Exp $
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

#endif

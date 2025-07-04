/*
 * $Id: read.h,v 1.8 2005/06/22 21:52:53 jon Exp $
 *
 * Reading a header for meataxe
 *
 */

#ifndef included__read
#define included__read

#include <stdio.h>
#include "header.h"

extern int read_text_header(FILE *, const header **, const char *, const char *);
extern int read_binary_header(FILE *, const header **, const char *, const char *);
extern int read_text_header_items(FILE *, u32 *nod, u32 *prime,
                                  u32 *nor, u32 *noc, const char *, const char *);
extern int open_and_read_binary_header(FILE **, const header **, const char *, const char *);

#endif

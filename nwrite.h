/*
 * $Id: nwrite.h,v 1.1 2012/03/24 13:32:21 jon Exp $
 *
 * Writing a header for meataxe 64
 *
 */

#ifndef included__write
#define included__write

#include <stdio.h>
#include "nheader.h"

extern int write_text_nheader(FILE *, const nheader *, const char *);
extern int write_binary_nheader(FILE *, const nheader *, const char *, const char *);
extern int open_and_write_binary_nheader(FILE **, const nheader *, const char *, const char *);

#endif

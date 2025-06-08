/*
 * $Id: nwrite.h,v 1.2 2012/06/17 10:41:29 jon Exp $
 *
 * Writing a header for meataxe 64
 *
 */

#ifndef included__nwrite
#define included__nwrite

#include <stdio.h>
#include "nheader.h"

extern int nwrite_text_nheader(FILE *, const nheader *, const char *);
extern int nwrite_binary_nheader(FILE *, const nheader *, const char *, const char *);
extern int open_and_write_binary_nheader(FILE **, const nheader *, const char *, const char *);

#endif /* included__nwrite */

/*
 * $Id: write.h,v 1.7 2002/06/27 07:31:58 jon Exp $
 *
 * Writing a header for meataxe
 *
 */

#ifndef included__write
#define included__write

#include <stdio.h>
#include "header.h"

extern int write_text_header(FILE *, const header *, const char *);
extern int write_binary_header(FILE *, const header *, const char *, const char *);
extern int open_and_write_binary_header(FILE **, const header *, const char *, const char *);

#endif

/*
 * $Id: write.h,v 1.5 2001/11/07 22:35:27 jon Exp $
 *
 * Writing a header for meataxe
 *
 */

#ifndef included__write
#define included__write

#include <stdio.h>
#include "header.h"

extern int write_text_header(FILE *, const header *);
extern int write_binary_header(FILE *, const header *, const char *);

#endif

/*
 * $Id: elements.h,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Element manipulation for meataxe
 *
 */

#ifndef included__elements
#define included__elements

#include <stdio.h>

/* Read an element from the text input stream */
/* Return 1 for success, 0 for failure */
extern int get_element(FILE *, unsigned int nob,
                       unsigned int prime, unsigned int *);

#endif

/*
 * $Id: elements.h,v 1.3 2001/09/02 22:16:41 jon Exp $
 *
 * Element manipulation for meataxe
 *
 * Internally, values are held in unsigned ints, with maximal
 * packing and the least offset value at the least significant end
 *
 */

#ifndef included__elements
#define included__elements

#include <stdio.h>

/* Read an element from the text input stream */
/* Return 1 for success, 0 for failure */
extern int get_element_from_text(const FILE *, unsigned int nob,
                                 unsigned int prime, unsigned int *);
/* Extract an element from a row at given position */
extern unsigned int get_element_from_row(unsigned int nob, unsigned int index,
                                         const unsigned int *row);

/* Insert an element into a row at given position */
extern void put_element_to_row(unsigned int nob, unsigned int index,
                               unsigned int *row, unsigned int elt);

#endif

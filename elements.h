/*
 * $Id: elements.h,v 1.10 2001/11/06 22:25:40 jon Exp $
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
extern int get_element_from_text(FILE *, unsigned int nod,
                                 unsigned int prime, unsigned int *);
/* Extract an element from a row at given position */
extern unsigned int get_element_from_row(unsigned int nob, unsigned int index,
                                         const unsigned int *row);

/* Initialise for element access */
extern void element_access_init(unsigned int nob, unsigned int from, unsigned int size,
                                unsigned int *word_offset, unsigned int *bit_offset,
                                unsigned int *mask);

/* Extract some elements from a row at given position */
extern unsigned int get_elements_from_row(const unsigned int *row, unsigned int width,
                                          unsigned int bit_offset, unsigned int mask);

/* Insert an element into a row at given position */
extern void put_element_to_row(unsigned int nob, unsigned int index,
                               unsigned int *row, unsigned int elt);

extern unsigned int elements_contract(unsigned int elts, unsigned int prime, unsigned int nob);

extern unsigned int count_word(unsigned int word, unsigned int nob);

extern unsigned int negate_elements(unsigned int elts, unsigned int nob, unsigned int prime);

extern unsigned int invert_elements(unsigned int elts, unsigned int nob, unsigned int prime);

#endif

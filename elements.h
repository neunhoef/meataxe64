/*
 * $Id: elements.h,v 1.16 2002/09/27 19:36:59 jon Exp $
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

/* Extract an element from a character row at given position */
extern unsigned int get_element_from_char_row(unsigned int eperb, unsigned int prime,
                                              unsigned int index, const char *row);

/* Initialise for element access */
extern void element_access_init(unsigned int nob, unsigned int from, unsigned int size,
                                unsigned int *word_offset, unsigned int *bit_offset,
                                unsigned int *mask);

/* Just get the mask */
extern unsigned int get_mask_and_elts(unsigned int nob, unsigned int *elts_per_word);

/* Extract an element from a row at given position, with fixed mask */
extern unsigned int get_element_from_row_with_params(unsigned int nob, unsigned int index, unsigned int mask,
                                                     unsigned int elts_per_word, const unsigned int *row);

/* Extract some elements from a row at given position */
extern unsigned int get_elements_from_row(const unsigned int *row, unsigned int width, unsigned int nob,
                                          unsigned int bit_offset, unsigned int mask);

/* Insert an element into a row at given position */
extern void put_element_to_row(unsigned int nob, unsigned int index,
                               unsigned int *row, unsigned int elt);

/* Insert an element into a row at given position */
extern void put_element_to_row_with_params(unsigned int nob, unsigned int index, unsigned int mask,
                                           unsigned int elts_per_word, unsigned int *row, unsigned int elt);

/* Insert an element into a character row at given position */
extern void put_element_to_char_row(unsigned int eperb, unsigned int prime,
                                    unsigned int index, char *row, unsigned int elt);

extern unsigned int elements_contract(unsigned int elts, unsigned int prime, unsigned int nob);

extern unsigned int count_word(unsigned int word, unsigned int nob);

extern unsigned int negate_elements(unsigned int elts, unsigned int nob, unsigned int prime);

extern unsigned int invert_elements(unsigned int elts, unsigned int nob, unsigned int prime);

extern unsigned int first_non_zero(unsigned int *row, unsigned int nob,
                                   unsigned int len, unsigned int *pos);

#endif

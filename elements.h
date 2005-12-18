/*
 * $Id: elements.h,v 1.21 2005/12/18 11:22:07 jon Exp $
 *
 * Element manipulation for meataxe
 *
 * Internally, values are held in words, with maximal
 * packing and the least offset value at the least significant end
 *
 */

#ifndef included__elements
#define included__elements

#include <stdio.h>

/* Read an element from the text input stream */
/* Return 1 for success, 0 for failure */
extern int get_element_from_text(FILE *, u32 nod,
                                 u32 prime, word *);
/* Extract an element from a row at given position */
extern word get_element_from_row(u32 nob, u32 index,
                                 const word *row);

/* Extract an element from a character row at given position */
extern word get_element_from_char_row(u32 eperb, u32 prime,
                                      u32 index, const unsigned char *row);

/* Initialise for element access */
extern void element_access_init(u32 nob, u32 from, u32 size,
                                u32 *word_offset, u32 *bit_offset,
                                word *mask);

/* Just get the mask */
extern word get_mask_and_elts(u32 nob, u32 *elts_per_word);

/* Extract an element from a row at given position, with fixed mask */
extern word get_element_from_row_with_params(u32 nob, u32 index, word mask,
                                             u32 elts_per_word, const word *row);

/* Extract some elements from a row at given position, into an array of words */
extern void get_elements_from_row_with_params_into_row(u32 nob, u32 index, word mask,
                                                       u32 elts_per_word, const word *row,
                                                       u32 count, word *out);

/* Extract some elements from a row at given position, from within one word only */
extern word get_elements_in_word_from_row(const word *row,
                                          u32 bit_offset, word mask);

/* Extract some elements from a row at given position, across words */
extern word get_elements_out_word_from_row(const word *row,
                                           u32 shift,
                                           u32 bit_offset, word mask);

/* Extract some elements from a row at given position */
extern word get_elements_from_row(const word *row, u32 width, u32 nob,
                                  u32 bit_offset, word mask);

/* Insert an element into a row at given position */
extern void put_element_to_row(u32 nob, u32 index,
                               word *row, word elt);

/* Insert an element into a row at given position */
extern void put_element_to_row_with_params(u32 nob, u32 index, word mask,
                                           u32 elts_per_word, word *row, word elt);

/* Insert an element into a row at given position */
extern void put_element_to_clean_row_with_params(u32 nob, u32 index,
                                                 u32 elts_per_word, word *row, word elt);

/* Insert an element into a character row at given position */
extern void put_element_to_char_row(u32 eperb, u32 prime,
                                    u32 index, unsigned char *row, word elt);

extern word elements_contract(word elts, u32 prime, u32 nob);

extern word count_word(word word, u32 nob);

extern word negate_elements(word elts, u32 nob, u32 prime);

extern word invert_elements(word elts, u32 nob, u32 prime);

extern u32 first_non_zero(word *row, u32 nob,
                          u32 len, u32 *pos);

/* Extra functions for conversion between 32 and 64 bit systems */

extern u32 get_element_from_u32_row(u32 nob, u32 index,
                                     const u32 *row);

extern u64 get_element_from_u64_row(u32 nob, u32 index,
                                     const u64 *row);

extern void put_element_to_u32_row(u32 nob, u32 index,
                                   u32 *row, u32 elt);

extern void put_element_to_u64_row(u32 nob, u32 index,
                                   u64 *row, u64 elt);

#endif

/*
 * Function to generate elements of the span of a set of rows
 *
 */

#ifndef included__span
#define included__span

/* Produce the next vector in the span of rows */
/* vector should be set to all zeroes to initialise */
/* vector has one field element per word */
/* function does not detect run out of span, and will simply wrap */
/* out_num indicates a vector guaranteed to be involved in the sum */
/* and which could therefore be replaced by the sum in a basis */
extern void span(u32 nor, word *vector, u32 prime, u32 *out_num);

#endif

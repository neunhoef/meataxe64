/*
 * $Id: span.h,v 1.1 2002/10/27 11:54:26 jon Exp $
 *
 * Function to generate elements of the span of a set of rows
 *
 */

#ifndef included__span
#define included__span

/* Produce the next vector in the span of rows */
/* vector should be set to all zeroes to initialise */
/* vector has one field element per unsigned int */
/* function does not detect run out of span, and will simply wrap */
/* out_num indicates a vector guaranteed to be involved in the sum */
/* and which could therefore be replaced by the sum in a basis */
extern void span(unsigned int nor, unsigned int *vector, unsigned int prime, unsigned int *out_num);

#endif

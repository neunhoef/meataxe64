/*
 * $Id: clean_vectors.h,v 1.1 2002/10/12 14:17:06 jon Exp $
 *
 * Clean one file of vectors with another
 *
 */

#ifndef included__clean_vectors
#define included__clean_vectors

/* Clean the dirty vectors with the clean vectors, and output the result */
extern int clean_vectors(const char *echelised, const char *vectors, const char *output, const char *name);

#endif

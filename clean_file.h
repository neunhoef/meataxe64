/*
 * $Id: clean_file.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Cleaning and echilisation, when the already clean vectors
 * are in a file which is to be updated
 *
 */

#ifndef included__clean_file
#define included__clean_file

#include <stdio.h>

/* Clean input rows with clean_vectors, and extend clean_vectors */
/* Return the new number of clean vectors */
/* Record the map extension */
/* nor is total rows in file, nor1 is number in rows1 */
/* nor2 is space available in rows2 (scratch) */
/* Also, if asked, return the map for input rows (needed for standard base) */
extern int clean_file(FILE *clean_vectors, unsigned int *nor,
                      unsigned int **rows1, unsigned int nor1,
                      unsigned int **rows2, unsigned int nor2,
                      int *map, int *new_map, int record,
                      unsigned int grease_level, unsigned int prime,
                      unsigned int len, unsigned int nob,
                      unsigned int start, const char *name);

#endif

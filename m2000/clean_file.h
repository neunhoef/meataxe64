/*
 * $Id: clean_file.h,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Cleaning and echilisation, when the already clean vectors
 * are in a file which is to be updated
 *
 */

#ifndef included__clean_file
#define included__clean_file

#include <stdio.h>
#include "rows.h"

/* Clean input rows with clean_vectors, and extend clean_vectors */
/* Return the new number of clean vectors */
/* Record the map extension */
/* nor is total rows in file, nor1 is number in rows1 */
/* nor2 is space available in rows2 (scratch) */
/* Also, if asked, return the map for input rows (needed for standard base) */
extern int clean_file(row_ops *row_operations,
                      FILE *clean_vectors, u32 *nor,
                      word **rows1, u32 nor1,
                      word **rows2, u32 nor2,
                      int *map, int *new_map, int record,
                      u32 grease_level, u32 prime,
                      u32 len, u32 nob,
                      u32 start, const char *name);

#endif

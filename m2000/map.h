/*
 * $Id: map.h,v 1.1 2001/10/03 00:01:42 jon Exp $
 *
 * Handle maps for exploded matrices
 *
 */

#ifndef included__map
#define included__map

extern void input_map(const char *name, const char *dir, unsigned int *cols,
                      unsigned int *rows, const char ***names);

extern void output_map(const char *name, const char *dir, unsigned int cols,
                       unsigned int rows, const char ***names);

#endif

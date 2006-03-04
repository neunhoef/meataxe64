/*
 * $Id: gen.h,v 1.1 2006/03/04 09:02:06 jon Exp $
 *
 * Types and functions to handle generator sets
 *
 */

#ifndef included__gen
#define included__gen

#include <stdio.h>

typedef struct gen_struct *gen;

struct gen_struct
{
  FILE *f;       /* File containing the generator */
  const char *m; /* Name of the generator */
  u32 nor;       /* Rows from input already multiplied by this generator */
  int is_map;    /* 1 if a map, 0 otherwise */
  gen next;      /* The next generator in the set */
};

typedef struct genf_struct *genf;

struct genf_struct
{
  FILE *f;       /* File containing the generator */
  const char *m; /* Name of the generator */
  u32 nor;       /* Rows from input already multiplied by this generator */
  int is_map;    /* 1 if a map, 0 otherwise */
  s64 base_ptr;  /* Pointer to row nor + 1 in output basis file */
  genf next;     /* The next generator in the set */
};

#endif

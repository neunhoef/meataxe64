/*
 * $Id: gen.h,v 1.2 2006/05/01 09:08:45 jon Exp $
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

typedef struct gen2_struct *gen2;

struct gen2_struct
{
  FILE *f1, *f2;        /* File containing the generator */
  const char *m1, *m2;  /* Name of the generator */
  u32 nor;              /* Rows from input already multiplied by this generator */
  int is_map1, is_map2; /* 1 if a map, 0 otherwise */
  word **rows_1;        /* Rows of left tensor */
  word **rows_2;        /* Rows of right tensor */
  gen2 next;            /* The next generator in the set */
};

typedef struct gen2f_struct *gen2f;

struct gen2f_struct
{
  FILE *f1, *f2;       /* File containing the generator */
  const char *m1, *m2; /* Name of the generator */
  u32 nor;             /* Rows from input already multiplied by this generator */
  int is_map1, is_map2; /* 1 if a map, 0 otherwise */
  word **rows_1;       /* Rows of left tensor */
  word **rows_2;       /* Rows of right tensor */
  s64 base_ptr;        /* Pointer to row nor + 1 in output basis file */
  gen2f next;          /* The next generator in the set */
};

extern void cleanup_files(FILE **files, u32 count);

extern int unfinished(gen gens, unsigned int argc, u32 nor);

extern int unfinishedf(genf gens, unsigned int argc, u32 nor);

#endif

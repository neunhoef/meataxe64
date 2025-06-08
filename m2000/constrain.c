/*
 * $Id: constrain.c,v 1.2 2005/07/24 09:32:45 jon Exp $
 *
 * Constraint based solution of decompositions
 * Constraints are inequalities (<=) or equalities
 * All solutions are assumed positive, so the equality constrinats can be used as inequalities
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"

static unsigned int num_equalities;
static unsigned int num_inequalities;
static unsigned int num_vars;
static unsigned int **equalities;
static unsigned int **inequalities;
static unsigned int *indexes;

typedef int (*test) (unsigned int, unsigned int);

static int less_than_test(unsigned int a, unsigned int b)
{
  return (a <= b);
}

static int equality_test(unsigned int a, unsigned int b)
{
  return (a == b);
}

static int constraint_test(test t, unsigned int **constraints, unsigned int *vector,
                           unsigned int vec_len, unsigned int rows)
{
  unsigned int i, j;
  assert(NULL != vector);
  assert(NULL != constraints);
  for (i = 0; i < rows; i++) {
    unsigned int tot = 0;
    assert(NULL != constraints[i]);
    for (j = 0; j < vec_len; j++) {
      tot += constraints[i][j] * vector[j];
    }
    if (0 == (*t)(tot, constraints[i][num_vars])) {
      return 0;
    }
  }
  return 1;
}

static int check_inequality_constraints(unsigned int **constraints, unsigned int *vector,
                                        unsigned int vec_len, unsigned int rows)
{
  return constraint_test(&less_than_test, constraints, vector, vec_len, rows);
}

static int check_equality_constraints(unsigned int **constraints, unsigned int *vector,
                                      unsigned int vec_len, unsigned int rows)
{
  return constraint_test(&equality_test, constraints, vector, vec_len, rows);
}

static unsigned int solve(unsigned int *vector, unsigned int depth, unsigned int max_depth)
{
  unsigned int total = 0, new_depth = depth + 1;
  assert(NULL != vector);
  assert(depth < max_depth);
  vector[depth] = 0;
  while (check_inequality_constraints(inequalities, vector, new_depth, num_inequalities) &&
         check_inequality_constraints(equalities, vector, new_depth, num_equalities)) {
    if (new_depth < max_depth) {
      unsigned int new_sols = solve(vector, new_depth, max_depth);
      total += new_sols;
    } else {
      /* Reached the last variable */
      if (check_equality_constraints(equalities, vector, max_depth, num_equalities)) {
        unsigned int i;
        printf("solution: %u", vector[0]);
        for (i = 1; i < max_depth; i++) {
          printf(", %u", vector[i]);
        }
        printf("\n");
        total++;
      }
    }
    vector[depth]++;
  }
  return total;
}

static const char *name = "cons";

static void usage(const char *name)
{
  fprintf(stderr, "%s: usage: %s <number of inequalities> <number of equalities> <number of variables> <file of inequalities> <file of equalities>\n", name, name);
}

static int compar(const void *e1, const void *e2)
{
  unsigned int *v1 = (unsigned int *)e1;
  unsigned int *v2 = (unsigned int *)e2;
  unsigned int i1 = *v1;
  unsigned int i2 = *v2;
  if (equalities[0][i1] != equalities[0][i2]) {
    return (equalities[0][i1] > equalities[0][i2]) ? -1 : 1;
  } else {
    return 0;
  }
}

int main(int argc, const char *const argv[])
{
  unsigned int *vector;
  unsigned int total, i;
  FILE *equalities_file, *inequalities_file;
  if (6 != argc) {
    usage(name);
    exit(1);
  }
  num_inequalities = strtoul(argv[1], NULL, 0);
  num_equalities = strtoul(argv[2], NULL, 0);
  num_vars = strtoul(argv[3], NULL, 0);
  if (0 == num_inequalities || 0 == num_equalities || 0 == num_vars) {
    usage(name);
    exit(1);
  }
  /* Alloc vectors  and constraints */
  vector = my_malloc(num_vars * sizeof(unsigned int));
  indexes = my_malloc(num_vars * sizeof(unsigned int));
  equalities = my_malloc(num_equalities * sizeof(unsigned int *));
  inequalities = my_malloc(num_inequalities * sizeof(unsigned int *));
  for (i = 0; i < num_equalities; i++) {
    equalities[i] = my_malloc((num_vars + 1) * sizeof(unsigned int));
  }
  for (i = 0; i < num_inequalities; i++) {
    inequalities[i] = my_malloc((num_vars + 1) * sizeof(unsigned int));
  }
  for (i = 0; i < num_vars; i++) {
    indexes[i] = i;
  }
  inequalities_file = fopen(argv[4], "r");
  if (NULL == inequalities_file) {
    fprintf(stderr, "%s: failed to open '%s', terminating\n", name, argv[4]);
    exit(1);
  }
  for (i = 0; i < num_inequalities; i++) {
    if (0 == read_numbers(inequalities_file, num_vars + 1, inequalities[i])) {
      fprintf(stderr, "%s: failed to read '%s', terminating\n", name, argv[4]);
      exit(1);
    }
  }
  fclose(inequalities_file);
  equalities_file = fopen(argv[5], "r");
  if (NULL == equalities_file) {
    fprintf(stderr, "%s: failed to open '%s', terminating\n", name, argv[5]);
    exit(1);
  }
  for (i = 0; i < num_equalities; i++) {
    if (0 == read_numbers(equalities_file, num_vars + 1, equalities[i])) {
      fprintf(stderr, "%s: failed to read '%s', terminating\n", name, argv[5]);
      exit(1);
    }
  }
  fclose(equalities_file);
  qsort(indexes, num_vars, sizeof(unsigned int), &compar);
  /*
  for (i = 0; i < num_vars; i++) { printf("%u, ", indexes[i]); };
  printf("\n");
  */
  total = solve(vector, 0, num_vars);
  if (0 == total) {
    printf("No solutions found\n");
  } else {
    printf("%u solutions found\n", total);
  }
  return 0;
}

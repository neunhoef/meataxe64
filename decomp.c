/*
 * $Id: decomp.c,v 1.1 2002/09/01 12:33:40 jon Exp $
 *
 * Compute possible decompositions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "utils.h"

static unsigned int num_sub_irreds;
static unsigned int num_irreds;
static unsigned int degree;
static unsigned int full_degree;
static unsigned int *sub_degrees;
static unsigned int **irreds;
static unsigned int *irred_degrees;

static void try_nth_component(unsigned int n, unsigned int limit, unsigned int remaining_degree,
                              unsigned int *remaining_sub_multiplicities,
                              unsigned int *multiplicities,
                              unsigned int num_sub_irreds)
{
  unsigned int *my_remains = malloc(num_sub_irreds * sizeof(unsigned int));
  unsigned int this_degree = 0;
  unsigned int this_multiplicity = multiplicities[n];
  unsigned int max_multiplicity;
  unsigned int i, j;
  assert(n <= limit);
  memcpy(my_remains, remaining_sub_multiplicities, num_sub_irreds * sizeof(unsigned int));
  for (i = 0; i < num_sub_irreds; i++) {
    this_degree += sub_degrees[i] * irreds[n][i];
  }
  max_multiplicity = remaining_degree / this_degree;
  for (i = 0; i < num_sub_irreds; i++) {
    if (max_multiplicity * irreds[n][i] > remaining_sub_multiplicities[i]) {
      max_multiplicity = remaining_sub_multiplicities[i] / irreds[n][i];
    }
  }
  for (i = 0; i <= max_multiplicity; i++) {
    if (0 == remaining_degree) {
      /* This one works */
      printf("Found a solution: %d = %d +\n", full_degree, degree);
      for (j = 0; j < limit; j++) {
        printf("%d: %d\n", irred_degrees[j], multiplicities[j]);
      }
      break;
    } else {
      if (n + 1 < limit) {
        try_nth_component(n + 1, limit, remaining_degree, my_remains, multiplicities, num_sub_irreds);
      }
    }
    remaining_degree -= this_degree;
    for (j = 0; j < num_sub_irreds; j++) {
      my_remains[j] -= irreds[n][j];
    }
    multiplicities[n] +=1;
  }
  multiplicities[n] = this_multiplicity;
  free(my_remains);
}

static const char *name = "decomp";

static void usage(const char *name)
{
  fprintf(stderr, "%s: usage: %s <number of irreds> <number of subgroup irreds> <irreducible character degree> <subgroup degrees> <ordinary character decomposition> <modular irreducible decompositions>\n", name, name);
}

int main(int argc, const char *const argv[])
{
  unsigned int *character;
  unsigned int *multiplicities;
  FILE *sub_degrees_file, *character_file, *irreds_file;
  unsigned int i, j;
  if (7 != argc) {
    usage(name);
    exit(1);
  }
  num_irreds = strtoul(argv[1], NULL, 0);
  num_sub_irreds = strtoul(argv[2], NULL, 0);
  degree = strtoul(argv[3], NULL, 0);
  if (0 == num_irreds || 0 == num_sub_irreds) {
    usage(name);
    exit(1);
  }
  character = malloc(num_sub_irreds * sizeof(unsigned int));
  multiplicities = malloc(num_irreds * sizeof(unsigned int));
  sub_degrees = malloc(num_sub_irreds * sizeof(unsigned int));
  sub_degrees_file = fopen(argv[4], "r");
  character_file = fopen(argv[5], "r");
  if (NULL == sub_degrees_file) {
    fprintf(stderr, "%s: failed to open '%s', terminating\n", name, argv[4]);
    exit(1);
  }
  if (0 == read_numbers(sub_degrees_file, num_sub_irreds, sub_degrees)) {
    fprintf(stderr, "%s: failed to read '%s', terminating\n", name, argv[4]);
    exit(1);
  }
  fclose(sub_degrees_file);
  character_file = fopen(argv[5], "r");
  if (NULL == character_file) {
    fprintf(stderr, "%s: failed to open '%s', terminating\n", name, argv[5]);
    exit(1);
  }
  if (0 == read_numbers(character_file, num_sub_irreds, character)) {
    fprintf(stderr, "%s: failed to read '%s', terminating\n", name, argv[5]);
    exit(1);
  }
  fclose(character_file);
  irreds = malloc(num_irreds * sizeof(unsigned int *));
  for (i = 0; i < num_irreds; i++) {
    irreds[i] = malloc(num_sub_irreds * sizeof(unsigned int));
  }
  irreds_file = fopen(argv[6], "r");
  if (NULL == irreds_file) {
    fprintf(stderr, "%s: failed to open '%s', terminating\n", name, argv[6]);
    exit(1);
  }
  for (i = 0; i < num_irreds; i++) {
    if (0 == read_numbers(irreds_file, num_sub_irreds, irreds[i])) {
      fprintf(stderr, "%s: failed to read '%s', terminating\n", name, argv[6]);
      exit(1);
    }
  }
  fclose(irreds_file);
  irred_degrees = malloc(num_irreds * sizeof(unsigned int));
  for (i = 0; i < num_irreds; i++) {
    irred_degrees[i] = 0;
    for (j = 0; j < num_sub_irreds; j++) {
      irred_degrees[i] += irreds[i][j] * sub_degrees[j];
    }
  }
  full_degree = 0;
  for (i = 0; i < num_sub_irreds; i++) {
    full_degree += character[i] * sub_degrees[i];
  }
  printf("character has degree %d\n", full_degree);
  memset(multiplicities, 0, num_irreds * sizeof(unsigned int));
  try_nth_component(0, num_irreds, full_degree - degree, character, multiplicities, num_sub_irreds);
  return 0;
}

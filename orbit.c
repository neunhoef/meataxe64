/*
 * $Id: orbit.c,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Functions for handling orbits
 *
 */

#include "orbit.h"
#include "endian.h"
#include "utils.h"
#include <stdio.h>
#include <assert.h>

/* Read binary form of an orbit */
int read_orbits(FILE *inp, unsigned int nor, orbit_set **orbits,
               const char *in, const char *name)
{
  unsigned int size, i, j, total_points = 0;
  orbit_set *os;
  assert(NULL != inp);
  assert(NULL != orbits);
  assert(NULL != in);
  assert(NULL != name);
  /* Read the total number of orbits */
  if (1 != endian_read_int(&size, inp)) {
    fprintf(stderr, "%s: failed to read number of orbits from %s, terminating\n", name, in);
    return 0;
  }
  /* Allocate the orbit_set structure */
  os = my_malloc(sizeof(orbit_set));
  os->size = size;
  /* Allocate the orbit pointers */
  os->orbits = my_malloc(size * sizeof(orbit));
  /* Now read each orbit */
  for (i = 0; i < size; i++) {
    orbit *orb = os->orbits + i;
    unsigned int o_size;
    /* Now read the size of the orbit */
    if (0 == endian_read_int(&o_size, inp)) {
      fprintf(stderr, "%s: failed to read orbit size for orbit %d to orbits file %s, terminating\n", name, i, in);
      return 0;
    }
    total_points += o_size;
    orb->size = o_size;
    orb->values = my_malloc(o_size * sizeof(unsigned int));
    /* Now read the orbit entries */
    for (j = 0; j < o_size; j++) {
      if (0 == endian_read_int(orb->values + j, inp)) {
        fprintf(stderr, "%s: failed to read orbit value at offset %d for orbit %d to orbits file %s, terminating\n", name, j, i, in);
        return 0;
      }
      if (orb->values[j] >= nor) {
        fprintf(stderr, "%s: orbit value %d out of range offset %d for orbit %d in orbits file %s, terminating\n", name, orb->values[j], j, i, in);
        return 0;
      }
    }
  }
  if (nor != total_points) {
    fprintf(stderr, "%s: orbits file %s does not contains all points, terminating\n", name, in);
    return 0;
  }
  *orbits = os;
  return 1;
}

/* Write binary form of an orbit */
int write_orbits(FILE *outp, const orbit_set *orbits,
                const char *out, const char *name)
{
  unsigned int size, i, j;
  assert(NULL != outp);
  assert(NULL != orbits);
  assert(NULL != out);
  assert(NULL != name);
  size = orbits->size;
  /* Write the total number of orbits */
  if (0 == endian_write_int(size, outp)) {
    fprintf(stderr, "%s: failed to write size to orbits file %s, terminating\n", name, out);
    return 0;
  }
  /* Now write each orbit */
  for (i = 0; i < size; i++) {
    orbit *orb = orbits->orbits + i;
    unsigned int o_size = orb->size;
    /* Now write the size of the orbit */
    if (0 == endian_write_int(o_size, outp)) {
      fprintf(stderr, "%s: failed to write orbit size for orbit %d to orbits file %s, terminating\n", name, i, out);
      return 0;
    }
    /* Now write the orbit entries */
    for (j = 0; j < o_size; j++) {
      if (0 == endian_write_int(orb->values[j], outp)) {
        fprintf(stderr, "%s: failed to write orbit value at offset %d for orbit %d to orbits file %s, terminating\n", name, j, i, out);
        return 0;
      }
    }
  }
  return 1;
}

/* Write text form of an orbit */
void write_text_orbits(const orbit_set *orbits)
{
  unsigned int size, i, j;
  assert(NULL != orbits);
  size = orbits->size;
  /* Write the total number of orbits */
  printf("%d\n", size);
  /* Now write each orbit */
  for (i = 0; i < size; i++) {
    orbit *orb = orbits->orbits + i;
    unsigned int o_size = orb->size;
    /* Now write the size of the orbit */
    printf("%d\n", o_size);
    /* Now write the orbit entries */
    for (j = 0; j < o_size; j++) {
      printf("%d ", orb->values[j]);
    }
    printf("\n");
  }
}

void orbit_set_free(orbit_set *orbits)
{
  unsigned int size, i;
  assert(NULL != orbits);
  size = orbits->size;
  for (i = 0; i < size; i++) {
    assert(NULL != orbits->orbits[i].values);
    free(orbits->orbits[i].values);
  }
  free(orbits->orbits);
  free(orbits);
}

void orbit_free(orbit *orb)
{
  assert(NULL != orb);
  assert(NULL != orb->values);
  free(orb->values);
  free(orb);
}

void orbit_chain_free(orbit *orb)
{
  orbit *ptr;
  assert(NULL != orb);
  assert(NULL != orb->values);
  while(NULL != orb) {
    ptr = orb->next;
    free(orb->values);
    free(orb);
    orb = ptr;
  }
}

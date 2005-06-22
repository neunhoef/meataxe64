/*
 * $Id: orbit.c,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Functions for handling orbits
 *
 */

#include "orbit.h"
#include "endian.h"
#include "utils.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>

/* Read binary form of an orbit */
int read_orbits(FILE *inp, u32 nor, orbit_set **orbits,
               const char *in, const char *name)
{
  u32 size, i, j, total_points = 0;
  orbit_set *os;
  assert(NULL != inp);
  assert(NULL != orbits);
  assert(NULL != in);
  assert(NULL != name);
  /* Read the total number of orbits */
  errno = 0;
  if (1 != endian_read_u32(&size, inp)) {
    if ( 0 != errno) {
      perror(name);
    }
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
    u32 o_size;
    /* Now read the size of the orbit */
    errno = 0;
    if (0 == endian_read_u32(&o_size, inp)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read orbit size for orbit %d to orbits file %s, terminating\n", name, i, in);
      return 0;
    }
    total_points += o_size;
    orb->size = o_size;
    orb->values = my_malloc(o_size * sizeof(word));
    /* Now read the orbit entries */
    for (j = 0; j < o_size; j++) {
      errno = 0;
      if (0 == endian_read_word(orb->values + j, inp)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to read orbit value at offset %d for orbit %d to orbits file %s, terminating\n", name, j, i, in);
        return 0;
      }
      if (orb->values[j] >= nor) {
        fprintf(stderr, "%s: orbit value %d out of range offset %d for orbit %d in orbits file %s, terminating\n", name, (unsigned int)orb->values[j], j, i, in);
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
  u32 size, i, j;
  assert(NULL != outp);
  assert(NULL != orbits);
  assert(NULL != out);
  assert(NULL != name);
  size = orbits->size;
  /* Write the total number of orbits */
  errno = 0;
  if (0 == endian_write_u32(size, outp)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to write size to orbits file %s, terminating\n", name, out);
    return 0;
  }
  /* Now write each orbit */
  for (i = 0; i < size; i++) {
    orbit *orb = orbits->orbits + i;
    u32 o_size = orb->size;
    /* Now write the size of the orbit */
    errno = 0;
    if (0 == endian_write_u32(o_size, outp)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write orbit size for orbit %d to orbits file %s, terminating\n", name, i, out);
      return 0;
    }
    /* Now write the orbit entries */
    for (j = 0; j < o_size; j++) {
      errno = 0;
      if (0 == endian_write_word(orb->values[j], outp)) {
        if ( 0 != errno) {
          perror(name);
        }
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
  u32 size, i, j;
  assert(NULL != orbits);
  size = orbits->size;
  /* Write the total number of orbits */
  printf("%d\n", size);
  /* Now write each orbit */
  for (i = 0; i < size; i++) {
    orbit *orb = orbits->orbits + i;
    u32 o_size = orb->size;
    /* Now write the size of the orbit */
    printf("%d\n", o_size);
    /* Now write the orbit entries */
    for (j = 0; j < o_size; j++) {
      printf("%d ", (unsigned int)orb->values[j]);
    }
    printf("\n");
  }
}

void orbit_set_free(orbit_set *orbits)
{
  u32 size, i;
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

orbit_set *orbit_set_alloc(orbit_set *orbits)
{
  orbit_set *os;
  u32 i;
  assert(NULL != orbits);
  /* Allocate the orbit_set structure */
  os = my_malloc(sizeof(orbit_set));
  os->size = orbits->size;
  /* Allocate the orbit pointers */
  os->orbits = my_malloc(orbits->size * sizeof(orbit));
  /* Now allocate each orbit */
  for (i = 0; i < orbits->size; i++) {
    orbit *orb = os->orbits + i;
    u32 o_size = orbits->orbits[i].size;
    orb->size = o_size;
    orb->values = my_malloc(o_size * sizeof(word));
  }
  return os;
}

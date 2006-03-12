/*
 * $Id: pofp.c,v 1.1 2006/03/12 10:23:19 jon Exp $
 *
 * Function to compute fixed points of permutation in orbit
 *
 */

#include "pofp.h"
#include <assert.h>
#include <stdio.h>
#include "gen.h"
#include "maps.h"
#include "orbit.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static void cleanup(u32 count, FILE **files)
{
  if (NULL != files) {
    while (count > 0) {
      if (NULL != *files) {
        fclose(*files);
      }
      files++;
      count--;
    }
  }
}

u32 fixed_points_orbit(const char *in_orbit, unsigned int orbit_num, const char *out,
                       u32 argc, const char *const args[],
                       const char *name)
{
  FILE *outp = NULL, *inp = NULL, **files = NULL;
  unsigned int d, j;
  struct gen_struct *gens;
  const header *h;
  header *h_out;
  u32 nor, noc, nor_o = 0;
  orbit_set *orbits;
  orbit *orb;
  word **maps;
  word *fixed_points;
  assert(NULL != in_orbit);
  assert(NULL != out);
  assert(NULL != name);
  assert(0 != argc);
  assert(NULL != args);
  files = my_malloc(argc * sizeof(FILE *));
  gens = my_malloc(argc * sizeof(struct gen_struct));
  for (d = 1; d < argc; d++) {
    gens[d - 1].next = gens + d;
    files[d] = NULL;
  }
  gens[argc - 1].next = gens;
  files[0] = NULL;
  if (0 == open_and_read_binary_header(&inp, &h, in_orbit, name)) {
    exit(1);
  }
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  if (0 == read_orbits(inp, nor, &orbits, in_orbit, name)) {
    fprintf(stderr, "%s: cannot read orbits, terminating\n", name);
    header_free(h);
    fclose(inp);
    exit(1);
  }
  header_free(h);
  fclose(inp);
  if (orbit_num >= orbits->size) {
    fprintf(stderr, "%s: orbit number %d is out of range, terminating\n", name, orbit_num);
    exit(1);
  }
  for (d = 0; d < argc; d++) {
    const char *gen_name = args[d];
    const header *h;
    if (0 == open_and_read_binary_header(files + d, &h, gen_name, name)) {
      cleanup(argc, files);
      exit(1);
    }
    if (1 != header_get_prime(h)) {
      fprintf(stderr, "%s: non-permutation generator %s, terminating\n", name, gen_name);
      cleanup(argc, files);
      exit(1);
    }
    gens[d].m = gen_name;
    gens[d].f = files[d];
    gens[d].nor = 0;
    if (noc != header_get_noc(h) ||
        noc != header_get_nor(h)) {
      fprintf(stderr, "%s: incompatible parameters for %s, terminating\n",
              name, gen_name);
      cleanup(argc, files);
      exit(1);
    }
    header_free(h);
  }
  /* Read generators */
  maps = my_malloc(sizeof(*maps) * argc);
  for (d = 0; d < argc; d++) {
    maps[d] = malloc_map(nor);
  }
  for (d = 0; d < argc; d++) {
    int res = read_map(gens[d].f, nor, maps[d], name, gens[d].m);
    if (0 == res) {
      unsigned int j;
      for (j = 0; j < argc; j++) {
        map_free(maps[j]);
        free(maps);
      }
      exit(1);
    }
  }
  cleanup(argc, files); /* Close all generators */
  orb = orbits->orbits + orbit_num;
  fixed_points = my_malloc(sizeof(*fixed_points) * nor);
  for (d = 0; d < orb->size; d++) {
    word point = orb->values[d];
    int fixed = 1;
    for (j = 0; j < argc; j++) {
      if (maps[j][point] != point) {
        fixed = 0;
        break;
      }
    }
    if (fixed) {
      fixed_points[nor_o] = point;
      nor_o++;
    }
  }
  for (d = 0; d < argc; d++) {
    map_free(maps[d]);
  }
  free(maps);
  if (0 != nor_o) {
    h_out = header_create(1, 0, 0, noc, nor_o);
    if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
      free(fixed_points);
      exit(1);
    }
    if (0 == write_map(outp, nor, fixed_points, name, out)) {
      fclose(outp);
      free(fixed_points);
      exit(1);
    }
    fclose(outp);
  }
  return nor_o;
}

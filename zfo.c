/*
 * $Id: zfo.c,v 1.4 2004/01/04 21:22:50 jon Exp $
 *
 * Find the orbits under multiple generators
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "endian.h"
#include "header.h"
#include "maps.h"
#include "orbit.h"
#include "parse.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zfo";

static void fo_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <out_file> <in_file> <max_orbit_size> [<in_file>*]\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in, *out;
  FILE *inp, *outp;
  unsigned int prime, nor, noc, nob, max_orb, cur_point = 0, total_orbits = 0, i, j, count, im;
  unsigned int *map;
  const header *h;
  orbit_set *orbits;
  orbit **orb_ptr;
  orbit orb;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (4 > argc) {
    fo_usage();
    exit(1);
  }
  out = argv[1];
  in = argv[2];
  max_orb = strtoul(argv[3], NULL, 0);
  if (1 >= max_orb) {
    fprintf(stderr, "Maximum orbit size too small, terminating\n");
    exit(1);
  }
  if (0 == open_and_read_binary_header(&inp, &h, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  nob = header_get_nob(h);
  if (1 != prime || noc != nor) {
    fprintf(stderr, "%s: %s is not a map, terminating\n", name, in);
    fclose(inp);
    exit(1);
  }
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    fclose(inp);
    exit(1);
  }
  header_free(h);
  map = malloc_map(nor);
  if (0 == read_map(inp, nor, map, name, in)) {
    fclose(inp);
    fclose(outp);
    exit(1);
  }
  fclose(inp);
  /* A set of pointer to the orbits, for each element */
  orb_ptr = my_malloc(nor * sizeof(orbit *));
  /* All pointers NULL to start with */
  memset(orb_ptr, 0, nor * sizeof(orbit *));
  /* An orbit for working in */
  orb.values = my_malloc(max_orb * sizeof(unsigned int));
  while (cur_point < nor) {
    /* Find the next untried point and compute its orbit in orb */
    if (NULL == orb_ptr[cur_point]) {
      /* Found an untried point */
      im = map[cur_point], i;
      orb.size = 1;
      orb.values[0] = cur_point;
      while (im != cur_point) {
        if (im >= nor) {
          fprintf(stderr, "%s: orbit containing %d has out of range value %d, terminating\n", name, cur_point, im);
          fclose(outp);
          exit(1);
        }
        if (orb.size >= max_orb) {
          fprintf(stderr, "%s: orbit containing %d has out of range size, terminating\n", name, cur_point);
          fclose(outp);
          exit(1);
        } else {
          orb.values[orb.size] = im;
          orb.size++;
          im = map[im];
        }
      }
      /* Now we have a full orbit */
      orb_ptr[cur_point] = my_malloc(sizeof(orbit));
      orb_ptr[cur_point]->size = orb.size;
      orb_ptr[cur_point]->values = my_malloc(orb.size * sizeof(unsigned int));
      memcpy(orb_ptr[cur_point]->values, orb.values, orb.size * sizeof(unsigned int));
      /* No fusion yet */
      orb_ptr[cur_point]->next = NULL;
      /* Now make all points in orb_ptr point to this orbit */
      for (i = 1; i < orb.size; i++) {
        orb_ptr[orb.values[i]] = orb_ptr[cur_point];
      }
      total_orbits++;
    }
    cur_point++;
  }
  printf("A total of %d orbits were found under %s\n", total_orbits, in);
  count = 4;
  while (count < (unsigned int)argc) {
    /* Incoming conditions */
    /* Each pointer in orb_ptr is valid */
    in = argv[count];
/*
    printf("Processing generator %s\n", in);
*/
    if (0 == open_and_read_binary_header(&inp, &h, in, name)) {
      fclose(outp);
      exit(1);
    }
    if (1 != header_get_prime(h) ||
        nor != header_get_nor(h) ||
        noc != header_get_noc(h)) {
      fprintf(stderr, "%s: %s is not a map, terminating\n", name, in);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
    if (0 == read_map(inp, nor, map, name, in)) {
      fclose(inp);
      fclose(outp);
      exit(1);
    }
    fclose(inp);
    cur_point = 0;
/*
    printf("Processing orbits\n");
*/
    while (cur_point < nor) {
      orbit *orb = orb_ptr[cur_point];
      orbit *orig = orb;
      if (NULL != orb) {
/*
        printf("Processing orbit %d\n", cur_point);
*/
        while (NULL != orb) {
          for (i = 0; i < orb->size; i++) {
            orb_ptr[orb->values[i]] = NULL;
            /* Make sure we don't follow any pointers into this orbit */
          }
          for (i = 0; i < orb->size; i++) {
            /* Look at images under map, and fuse orbits if necessary */
            im = map[orb->values[i]];
            if (NULL != orb_ptr[im]) {
              unsigned int k, *values;
/*
              printf("Fusing orbit %d\n", im);
*/
              orb_ptr[im]->next = orb->next;
              orb->next = orb_ptr[im];
              k = orb_ptr[im]->size;
              values = orb_ptr[im]->values;
              for (j = 0; j < k; j++) {
                orb_ptr[values[j]] = NULL;
              }
            } else {
              /* No action, this does not cause any fusion */
            }
          }
          /* Process next orbit in the chain */
          orb = orb->next;
        }
        /* Don't lose the chain, which is now a complete orbit */
        orb_ptr[cur_point] = orig;
      }
      cur_point++;
    }
    /* Now fuse the chains */
    cur_point = 0;
/*
    printf("Fusing orbits\n");
*/
    while (cur_point < nor) {
      orbit *orb = orb_ptr[cur_point];
      if (NULL != orb) {
/*
        printf("Fusing orbits %d\n", cur_point);
*/
        if (NULL != orb->next) {
          unsigned int size = orb->size;
          unsigned int *values;
          orbit *ptr = orb->next;
          while (NULL != ptr) {
            size += ptr->size;
            ptr = ptr->next;
          }
          values = my_malloc(size * sizeof(unsigned int));
          i = 0;
          ptr = orb;
          while (NULL != ptr) {
            memcpy(values + i, ptr->values, ptr->size * sizeof(unsigned int));
            i += ptr->size;
            ptr = ptr->next;
          }
          /* Free the first orbit in the chain */
          free(orb->values);
          /* Link in the new orbit and set its size */
          orb->values = values;
          orb->size = size;
          /* Get a pointer to the rest of the chain */
          ptr = orb->next;
          /* And unlink the rest of the chain */
          orb->next = NULL;
          /* Now free the rest of the chain */
          orbit_chain_free(ptr);
        } else {
          /* Not fused, just carry on */
        }
      }
      cur_point++;
    }
    /* Now recreate orb_ptr */
/*
    printf("Recreating pointers\n");
*/
    cur_point = 0;
    total_orbits = 0;
    while (cur_point < nor) {
      orbit *orb = orb_ptr[cur_point];
      if (NULL != orb && orb->values[0] == cur_point) {
        /* Only consider orbits that haven't been dealt with */
        unsigned int size = orb->size;
/*
        printf("Recreating orbit %d\n", cur_point);
*/
        for (i = 1; i < size; i++) {
          /* Reset the pointer to the relevant orbit */
          orb_ptr[orb->values[i]] = orb;
        }
        total_orbits++;
      }
      cur_point++;
    }
    printf("After processing %s, there are %d orbits\n", in, total_orbits);
    count++;
  }
  orbits = my_malloc(sizeof(*orbits));
  orbits->size = total_orbits;
  orbits->orbits = my_malloc(total_orbits * sizeof(orbit));
  cur_point = 0;
  i = 0;
  while (cur_point < nor) {
    if (NULL != orb_ptr[cur_point]) {
      /* Record the orbit in the structure */
      unsigned int k = orb_ptr[cur_point]->size;
      orbits->orbits[i] = *(orb_ptr[cur_point]);
      /* Now set all pointers to this orbit to NULL */
      for (j = 0; j < k; j++) {
        orb_ptr[orbits->orbits[i].values[j]] = NULL;
      }
      i++;
    }
    /* And on to next potential orbit point */
    cur_point++;
  }
  if (0 == write_orbits(outp, orbits, out, name)) {
    fprintf(stderr, "%s: failed to write orbits to %s, terminating\n",
            name, out);
    fclose(outp);
    exit(1);
  }
  orbit_set_free(orbits);
  map_free(map);
  return 0;
}

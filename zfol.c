/*
 * $Id: zfol.c,v 1.1 2005/01/06 21:37:51 jon Exp $
 *
 * Find the orbits under multiple generators for a linear representation
 *
 * This provides the information needed to produce weighted orbit
 * sums when condensing against a non-trivial linear character
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
#include "primes.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zfol";

static void fo_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file> <out_file> <permutation generator> <character of generator> [<permutations and characters>]\n", name, name, parse_usage());
}

static void cleanup_gens(FILE **gens, const header **h_perm, const header **h_lambda, unsigned int total, unsigned int count)
{
  unsigned int i;
  assert(NULL != gens);
  assert(NULL != h_perm);
  assert(NULL != h_lambda);
  for (i = 0; i < count; i++) {
    fclose(gens[i]);
    fclose(gens[total + i]);
    header_free(h_perm[i]);
    header_free(h_lambda[i]);
  }
  free(gens);
  free(h_perm);
  free(h_lambda);
}

static void cleanup_perms_and_lambdas(word **perms, word *lambdas, unsigned int count)
{
  unsigned int i;
  assert(NULL != perms);
  assert(NULL != lambdas);
  for (i = 0; i < count; i++) {
    free(perms[i]);
  }
  free(perms);
  free(lambdas);
}

/* Compute the character of the element leading us to pt */
static word trace(u32 *schreier, u32 *back, word *lambdas, u32 idx_pt, binary_fn mult)
{
  word lambda = 1;
  assert(NULL != schreier);
  assert(NULL != back);
  assert(NULL != lambdas);
  while (0 != idx_pt) {
    lambda = (*mult)(lambdas[schreier[idx_pt] - 1], lambda);
    idx_pt = back[idx_pt];
  }
  return lambda;
}

int main(int argc, const char * const argv[])
{
  const char *in, *out;
  FILE *inp, *outp, **gens;
  u32 prime, gen_prime, nor, noc, nob, i, max_orb = 0;
  u32 *stack;
  u32 *indexes;
  /* TODO: check if Schreier and back are irrelevant */
  u32 *schreier; /* An array indexed by offset in orbit, giving the generator reaching there (1 based) */
  u32 *back; /* An array indexed by offset in orbit, giving the preceding point. i = gen[schreier[i]-1][back[orb->values[i]] */
  unsigned int count;
  int *done;
  word *lambdas;
  word **perms;
  const header *h_in, *h_out; /* The input and output orbits */
  const header **h_perm, **h_lambda; /* The permutation and linear generators */
  orbit_set *orbits_in, *orbits_out;
  prime_ops ops;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (5 > argc || argc % 2 == 0) {
    fo_usage();
    exit(1);
  }
  count = (argc - 3) / 2;
  assert(count >= 1);
  in = argv[1];
  out = argv[2];
  if (0 == open_and_read_binary_header(&inp, &h_in, in, name)) {
    exit(1);
  }
  prime = header_get_prime(h_in);
  nor = header_get_nor(h_in);
  noc = header_get_noc(h_in);
  nob = header_get_nob(h_in);
  if (1 != prime || noc != nor) {
    fprintf(stderr, "%s: %s is not a map, terminating\n", name, in);
    fclose(inp);
    exit(1);
  }
  h_perm = my_malloc(sizeof(*h_perm) * count);
  h_lambda = my_malloc(sizeof(*h_lambda) * count);
  gens = my_malloc(sizeof(*gens) * count * 2);
  lambdas = my_malloc(sizeof(*lambdas) * count);
  perms = my_malloc(sizeof(*perms) * count);
  for (i = 0; i < count; i++) {
    perms[i] = my_malloc(sizeof(*(perms[i])) * nor);
  }
  for (i = 0; i < count; i++) {
    if (0 == open_and_read_binary_header(gens + i, h_perm + i, argv[3 + 2 * i], name)) {
      cleanup_gens(gens, h_perm, h_lambda, count, i);
      cleanup_perms_and_lambdas(perms, lambdas, count);
      fclose(inp);
      exit(1);
    }
    if (0 == open_and_read_binary_header(gens + count + i, h_lambda + i, argv[4 + 2 * i], name)) {
      fclose(gens[i]);
      header_free(h_perm[i]);
      cleanup_gens(gens, h_perm, h_lambda, count, i);
      cleanup_perms_and_lambdas(perms, lambdas, count);
      fclose(inp);
      exit(1);
    }
  }
  gen_prime = header_get_prime(h_lambda[0]);
  for (i = 0; i < count; i++) {
    if (header_get_nor(h_perm[i]) != nor ||
        header_get_noc(h_perm[i]) != noc ||
        header_get_prime(h_perm[i]) != 1 ||
        header_get_nor(h_lambda[i]) != 1 ||
        header_get_noc(h_lambda[i]) != 1 ||
        header_get_prime(h_lambda[i]) != gen_prime ||
        (!is_a_prime_power(gen_prime))) {
      fprintf(stderr, "%s: parameter mismatch in generators, terminating\n", name);
      cleanup_gens(gens, h_perm, h_lambda, count, count);
      cleanup_perms_and_lambdas(perms, lambdas, count);
      fclose(inp);
      exit(1);
    }
  }
  for (i = 0; i < count; i++) {
    if (0 == read_map(gens[i], nor, perms[i], name, argv[3 + 2 * i])) {
      cleanup_gens(gens, h_perm, h_lambda, count, count);
      cleanup_perms_and_lambdas(perms, lambdas, count);
      fclose(inp);
      exit(1);
    }
  }
  for (i = 0; i < count; i++) {
    if (0 == endian_read_row(gens[count + i], lambdas + i, 1)) {
      fprintf(stderr, "%s: failed to read generator %s, terminating\n", name, argv[4 + 2 * i]);
      cleanup_gens(gens, h_perm, h_lambda, count, count);
      cleanup_perms_and_lambdas(perms, lambdas, count);
      fclose(inp);
      exit(1);
    }
  }
  if (0 == read_orbits(inp, nor, &orbits_in, in, name)) {
    fprintf(stderr, "%s: cannot read orbit file %s, terminating\n", name, in);
    cleanup_gens(gens, h_perm, h_lambda, count, count);
    cleanup_perms_and_lambdas(perms, lambdas, count);
    fclose(inp);
    exit(1);
  }
  fclose(inp);
  header_free(h_in);
  /* Now compute max_orb */
  for (i = 0; i < orbits_in->size; i++) {
    if (orbits_in->orbits[i].size > max_orb) {
      max_orb = orbits_in->orbits[i].size;
    }
  }
  orbits_out = orbit_set_alloc(orbits_in);
  h_out = header_create(gen_prime, header_get_nob(h_lambda[0]), header_get_nod(h_lambda[0]), noc, nor);
  cleanup_gens(gens, h_perm, h_lambda, count, count);
  if (0 == open_and_write_binary_header(&outp, h_out, out, name)) {
    cleanup_perms_and_lambdas(perms, lambdas, count);
    header_free(h_out);
    orbit_set_free(orbits_in);
    orbit_set_free(orbits_out);
    exit(1);
  }
  header_free(h_out);
  if (0 == primes_init(gen_prime, &ops)) {
    fprintf(stderr, "%s: failed to initialise for prime power %d, terminating\n", name, gen_prime);
    cleanup_perms_and_lambdas(perms, lambdas, count);
    orbit_set_free(orbits_in);
    orbit_set_free(orbits_out);
    exit(1);
  }
  /* Set up stack of size max orbit length */
  stack = my_malloc(sizeof(*stack) * max_orb * count);
  done = my_malloc(sizeof(*done) * max_orb);
  indexes = my_malloc(sizeof(*indexes) * nor);
  schreier = my_malloc(sizeof(*schreier) * max_orb);
  back = my_malloc(sizeof(*back) * max_orb);
  /* For each orbit */
  for (i = 0; i < orbits_in->size; i++) {
    orbit *orb_in = orbits_in->orbits + i;
    orbit *orb_out = orbits_out->orbits + i;
    u32 l = orb_in->size;
    u32 j;
    u32 ptr = 0;
    int ok;
    /* Initialise done and orb_out->values */
    for (j = 0; j < l; j++) {
      done[j] = 0;
      orb_out->values[j] = 0;
    }
    orb_out->values[0] = 1;
    /* Now compute indexes for this orbit */
    for (j = 0; j < l; j++) {
      indexes[orb_in->values[j]] = j;
    }
    /* Initialise stack */
    stack[ptr++] = orb_in->values[0];
    schreier[0] = 0;
    back[0] = 0;
    while (ptr > 0) {
      word pt = stack[--ptr];
      u32 idx_pt = indexes[pt];
      if (0 == done[idx_pt]) {
        u32 k;
        done[idx_pt] = 1;
        for (k = 0; k < count; k++) {
          word q = perms[k][pt];
          u32 idx_q = indexes[q];
          assert(q < nor);
          if (0 == orb_out->values[idx_q]) {
            orb_out->values[idx_q] = (*ops.mul)(orb_out->values[idx_pt], lambdas[k]);
            stack[ptr++] = q;
            schreier[idx_q] = k + 1; /* Index of generator (+1) */
            back[idx_q] = idx_pt; /* The point we came from via this generator */
          }
        }
      }
    }
    /* Now determine if the point stabiliser is in the kernel of the character */
    /* This uses Schreier's Lemma to give generators for the stabiliser, which we then check */
    ok = 1;
    for (j = 0; j < l; j++) {
      u32 k;
      word pt = orb_in->values[j];
      word lambda = trace(schreier, back, lambdas, j, ops.mul); /* The character of the coset rep taking us to the jth point */
      assert(lambda == orb_out->values[j]);
      for (k = 0; k < count; k++) {
        /* Loop over the generators */
        word im = perms[k][pt];
        u32 idx_im = indexes[im];
        word prod;
        word mu = trace(schreier, back, lambdas, idx_im, ops.mul);
        assert(mu == orb_out->values[idx_im]);
        prod = (*ops.mul)(lambda, lambdas[k]);
        prod = (*ops.mul)(prod, (*ops.invert)(mu));
        if (1 != prod) {
          ok = 0;
          break;
        }
      }
    }
    if (1 != ok) {
      /* Orbit fails, clear to zero */
      u32 k;
      for (k = 0; k < l; k++) {
        orb_out->values[k] = 0;
      }
    }
  }
  free(stack);
  free(done);
  free(indexes);
  free(schreier);
  free(back);
  orbit_set_free(orbits_in);
  cleanup_perms_and_lambdas(perms, lambdas, count);
  if (0 == write_orbits(outp, orbits_out, out, name)) {
    fprintf(stderr, "%s: cannot write orbit file %s, terminating\n", name, out);
    orbit_set_free(orbits_out);
    exit(1);
  }
  orbit_set_free(orbits_out);
  fclose(outp);
  return 0;
}

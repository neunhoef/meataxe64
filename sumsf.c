/*
 * $Id: sumsf.c,v 1.11 2004/08/28 19:58:00 jon Exp $
 *
 * Function to compute linear sums of two matices
 *
 */

#include "sumsf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "add.h"
#include "endian.h"
#include "header.h"
#include "ident.h"
#include "mul.h"
#include "parse.h"
#include "primes.h"
#include "read.h"
#include "rnf.h"
#include "utils.h"

static void cleanup(unsigned int *orders)
{
  assert(NULL != orders);
  free(orders);
}

int sumsf(const char *out, const char *dir, unsigned int n, unsigned int argc, const char *const args[],
          unsigned int sub_order, accept acceptor, int invertible, int keep, const char *name)
{
  char *buf;
  FILE *f;
  unsigned int *orders;
  unsigned int argc2 = argc / 2;
  unsigned int i, j, k, l, r, s, cur_word = 0, cur_power = 1, base_prime;
  unsigned int prime = 1, nod = 0, nor = 0, noc = 0, count;
  unsigned int order_sum = 0;
  const header *h;
  int m;
  int found = 0;
  const char **names;
  const char **words;
  const char **elts;
  const char **elt_names;
  const char *gen_names[] = { "A", "B", "C", "D", "E", "F", "G", "H" };

  assert(NULL != out);
  assert(NULL != name);
  assert(NULL != args);
  assert(NULL != dir);
  assert(2 < argc && 0 == argc % 2);
  if (argc2 > 8) {
    fprintf(stderr, "%s: too many generators (%d), terminating\n", name, argc2);
    exit(1);
  }
  orders = my_malloc(argc2 * sizeof(unsigned int));
  for (i = 0; i < argc2; i++) {
    orders[i] = strtoul(args[1 + 2 * i], NULL, 0);
    order_sum += orders[i];
  }
  if (2 > order_sum || 0 == n) {
    fprintf(stderr, "%s: unexpected zeroes in orders, or zero depth, terminating\n", name);
    cleanup(orders);
    exit(1);
  }
  if (0 != sub_order && 0 == is_a_prime_power(sub_order)) {
    fprintf(stderr, "%s: bad value %d for subfield order, terminating\n", name, sub_order);
    cleanup(orders);
    exit(1);
  }
  for (i = 0; i < argc2; i++) {
    if (0 == open_and_read_binary_header(&f, &h, args[2 * i], name)) {
      cleanup(orders);
      exit(1);
    }
    fclose(f);
    if (1 != header_get_prime(h)) {
      prime = header_get_prime(h);
      nor = header_get_nor(h);
      noc = header_get_noc(h);
      nod = header_get_nod(h); /* For printing element names */
      header_free(h);
      break;
    }
    header_free(h);
  }
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle both have both generators as permutations\n", name);
    cleanup(orders);
    exit(1);
  }
  if (nor != noc) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, args[2 * i]);
    cleanup(orders);
    exit(1);
  }
  if (0 != sub_order) {
    if (0 != prime % sub_order) {
      fprintf(stderr, "%s: %d is not a field order, terminating\n", name, sub_order);
      cleanup(orders);
      exit(1);
    }
    base_prime = prime_divisor(prime);
    if (0 != prime_index(prime, base_prime) % prime_index(sub_order, base_prime)) {
      fprintf(stderr, "%s: %d is not a sub field order for %d, terminating\n", name, sub_order, prime);
      cleanup(orders);
      exit(1);
    }
  }
  i = 1;
  m = -1;
  j = strlen(out);
  k = j + 13;
  n += 1;
  if (0 == int_pow((0 != sub_order) ? sub_order : prime, n, &count)) {
    fprintf(stderr, "%s: too many elements requested (%d ** %d), terminating\n", name, prime, n);
    cleanup(orders);
    exit(1);
  }
  names = my_malloc((n + 1) * sizeof(const char *));
  words = my_malloc((n + 1) * sizeof(const char *));
  elts = my_malloc(count * sizeof(const char *));
  elt_names = my_malloc(count * sizeof(const char *));
  for (l = 0; l < count; l++) {
    buf = my_malloc(2 * k + 3);
    sprintf(buf, "%s.e.%d", out, l);
    elts[l] = buf;
  }
  if (0 == ident(prime, nor, noc, 1, elts[0], name)) {
    fprintf(stderr, "%s: cannot write identity, terminating\n", name);
    cleanup(orders);
    exit(1);
  }
  elt_names[0] = "I";
  words[0] = "";
  names[0] = "I";
  for (j = 0; j < argc2; j++) {
    names[j + 1] = args[2 * j];
  }
  if (0 != sub_order) {
    prime = sub_order; /* Restrict to subfield if requested */
  }
  while (i < n) {
    char *a;
    const char *b;
    const char *c;
    const char *chosen_letter;
    unsigned int word_len;
    char letter;
    if (i > argc2) {
      buf = my_malloc(2 * k);
      sprintf(buf, "%s%d", out, i - 1);
      names[i] = buf;
    }
    while (1) {
      const char *word = words[cur_word];
      m = next_gen(m, argc2, &letter, orders, word);
      if (m >= 0) {
        break;
      } else {
        m = -1;
        /* Try a new word, with the first letter */
        cur_word++;
        assert(cur_word < i);
      }
    } /* while */
    /* Now cur_word is a pointer to a word we can safely append our letter to */
    assert(cur_word < i);
    chosen_letter = gen_names[m];
    word_len = strlen(words[cur_word]);
    a = my_malloc(2 * word_len + 2);
    strcpy(a, words[cur_word]);
    strcat(a, chosen_letter);
    words[i] = a;
    c = names[cur_word];
    b = args[2 * m];
    if (0 != word_len) {
      if (0 == mul(c, b, names[i], "zsums")) {
        cleanup(orders);
        exit(1);
      }
    }
    /* Now compute the sums we require, and their ranks */
    for (l = 0; l < cur_power; l++) {
      /* Make the base element if necessary */
      if (0 == keep) {
        /* We're discarding as we go for space reasons, so we must remake */
        if (0 == make_element(l, prime, cur_power, i, names, elts, elt_names, name)) {
          fprintf(stderr, "%s: failed to make %s, terminating\n", name, elt_names[l]);
          cleanup(orders);
          exit(1);
        }
      }
      for (r = 1; r < prime; r++) {
        unsigned int pos = cur_power * r + l;
        /* Range from cur_power to cur_power * prime - 1 */
        int ignore = 0;
        buf = my_malloc(strlen(elt_names[l]) + 1 + nod + strlen(words[i]) + 1);
        sprintf(buf, "%s+%d%s", elt_names[l], r, words[i]);
        elt_names[pos] = buf;
        /* Compute if this one can be ignored for rank purposes */
        if (0 != l) {
          unsigned int j;
          if (invertible) {
            for (j = 0; j < argc2; j++) {
              if (verbose) {
                printf("%s: Checking %s with %s\n", name, buf, words[j + 1]);
              }
              if (ignore_word(pos, i + 1, words, j, orders[j], prime)) {
                if (verbose) {
                  printf("%s: Ignoring %s after checking with %s\n", name, buf, words[j + 1]);
                }
                ignore = 1;
                break;
              }
            }
          }
        }
        if (keep || (0 != l && 0 == ignore)) {
          /* We only make the element if either we're keeping it, or we want its rank */
          if (verbose) {
            printf("%s: making %s, formula %s by %s + %d * %s\n", name, elts[pos], elt_names[pos], elts[l], r, names[i]);
            fflush(stdout);
          }
          /* scale names[i] and add to elts[l] */
          if (0 == scaled_add(names[i], elts[l], elts[pos], r, name)) {
            fprintf(stderr, "%s: scaled add failed on %s + %d * %s, terminating\n",
                    name, elts[l], r, names[i - 1]);
            cleanup(orders);
            exit(1);
          }
          /* We've made the element, do we want its rank? */
          /* l != 0 mean we have 1 + old element + lambda * new element, ie at least 3 in sum */
          if (0 != l && 0 == ignore) {
            int res;
            s = rankf(elts[pos], dir, name);
            /* Compute rank, using external files */
            if (verbose) {
              printf("%s: checking element %s of rank %d\n", name, elt_names[pos], s);
              fflush(stdout);
            }
            res = (*acceptor)(s, nor, elts[pos], elt_names[pos]);
            if (res & 1) {
              found = 1;
              printf("%s: found element %s of nullity %d, form %s\n",
                     name, elts[pos], nor - s, elt_names[pos]);
            }
            if (res & 2) {
              printf("%s: terminating\n", name);
              /* Discard the base element if necessary */
              if (0 == keep && 0 != l) {
                /* We're discarding as we go for space reasons, so discard this one */
                (void)remove(elts[l]);
              }
              return 0;
            }
          }
          /* Discard the current element if necessary */
          if (0 == keep && 0 != pos) {
            /* We're discarding as we go for space reasons, so discard this one */
            (void)remove(elts[pos]);
          }
        }
      }
      /* Discard the base element if necessary */
      if (0 == keep && 0 != l) {
        /* We're discarding as we go for space reasons, so discard this one */
        (void)remove(elts[l]);
      }
    }
    cur_power *= prime;
    i++;
  }
  /* Failed to find a suitable element */
  /* Failed to terminate on a suitable element */
  if (found) {
    return 0;
  } else {
    return 255;
  }
}

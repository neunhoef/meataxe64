/*
 * $Id: sums.c,v 1.19 2005/06/22 21:52:54 jon Exp $
 *
 * Function to compute linear sums of two matices
 *
 */

#include "sums.h"
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
#include "rn.h"
#include "sums_utils.h"
#include "utils.h"

static void cleanup(u32 *orders)
{
  assert(NULL != orders);
  free(orders);
}

int sums(const char *out, u32 n, unsigned int argc, const char *const args[],
         u32 sub_order, accept acceptor, int invertible, int keep, const char *name)
{
  char *buf;
  FILE *f;
  u32 *orders;
  unsigned int argc2 = argc / 2;
  u32 i, j, k, l, r, s, cur_word = 0, cur_power = 1, base_prime;
  u32 prime = 1, nod = 0, nor = 0, noc = 0, count;
  u32 order_sum = 0;
  const header *h;
  int m;
  int found = 0;
  const char **names;
  const char **words;
  const char *gen_names[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
  char *elt_l, *elt_pos;

  assert(NULL != out);
  assert(NULL != name);
  assert(NULL != args);
  assert(1 < argc && 0 == argc % 2);
  if (argc2 > 8) {
    fprintf(stderr, "%s: too many generators (%d), terminating\n", name, argc2);
    exit(1);
  }
  orders = my_malloc(argc2 * sizeof(u32));
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
  if (2 == argc && n >= orders[0]) {
    n = orders[0];
    /* If only one generator, can't go beyond the point where it wraps */
  } else {
    n += 1;
  }
  if (0 == int_pow((0 != sub_order) ? sub_order : prime, n, &count)) {
    fprintf(stderr, "%s: too many elements requested (%d ** %d), terminating\n", name, prime, n);
    cleanup(orders);
    exit(1);
  }
  names = my_malloc((n + 1) * sizeof(const char *));
  words = my_malloc((n + 1) * sizeof(const char *));
  elt_l = make_elt_name(out, 0);
  if (0 == ident(prime, nor, noc, 1, elt_l, name)) {
    fprintf(stderr, "%s: cannot write identity, terminating\n", name);
    cleanup(orders);
    exit(1);
  }
  free(elt_l);
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
    u32 word_len;
    char letter;
    if (i > argc2) {
      buf = my_malloc(2 * k);
      sprintf(buf, "%s%d", out, i - 1);
      names[i] = buf;
    }
    for (;;) {
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
      elt_l = make_elt_name(out, l);
      if (0 == keep) {
        /* We're discarding as we go for space reasons, so we must remake */
        if (0 == make_element(l, prime, cur_power, i, names, out, name)) {
          fprintf(stderr, "%s: failed to make %s, terminating\n", name, elt_l);
          cleanup(orders);
          exit(1);
        }
      }
      for (r = 1; r < prime; r++) {
        u32 pos = cur_power * r + l;
        /* Range from cur_power to cur_power * prime - 1 */
        int ignore = 0;
        char *elt_script;
        elt_script = make_elt_script(prime, cur_power, r, l, i, nod, words);
        /* Compute if this one can be ignored for rank purposes */
        if (0 != l) {
          u32 j;
          if (invertible) {
            for (j = 0; j < argc2; j++) {
              if (verbose) {
                printf("%s: Checking %s with %s\n", name, elt_script, words[j + 1]);
              }
              if (ignore_word(pos, i + 1, words, j, orders[j], prime)) {
                if (verbose) {
                  printf("%s: Ignoring %s after checking with %s\n", name, elt_script, words[j + 1]);
                }
                ignore = 1;
                break;
              }
            }
          }
        }
        elt_pos = make_elt_name(out, pos);
        if (keep || (0 != l && 0 == ignore)) {
          /* We only make the element if either we're keeping it, or we want its rank */
          if (verbose) {
            printf("%s: making %s, formula %s by %s + %d * %s\n", name, elt_pos, elt_script, elt_l, r, names[i]);
            fflush(stdout);
          }
          /* scale names[i] and add to elt_l */
          if (0 == scaled_add(names[i], elt_l, elt_pos, r, name)) {
            fprintf(stderr, "%s: scaled add failed on %s + %d * %s, terminating\n",
                    name, elt_l, r, names[i]);
            cleanup(orders);
            exit(1);
          }
          /* We've made the element, do we want its rank? */
          /* l != 0 mean we have 1 + old element + lambda * new element, ie at least 3 in sum */
          if (0 != l && 0 == ignore) {
            int res;
            if (0 == rank(elt_pos, &s, name)) {
              cleanup(orders);
              exit(1);
            }
            if (verbose) {
              printf("%s: checking element %s of rank %d\n", name, elt_script, s);
              fflush(stdout);
            }
            res = (*acceptor)(s, nor, elt_pos, elt_script);
            if (res & 1) {
              found = 1;
              printf("%s: found element %s of nullity %d, form %s\n",
                     name, elt_pos, nor - s, elt_script);
            }
            if (res & 2) {
              printf("%s: terminating\n", name);
              /* Discard the base element if necessary */
              if (0 == keep && 0 != l) {
                /* We're discarding as we go for space reasons, so discard this one */
                (void)remove(elt_l);
              }
              free(elt_l);
              free(elt_pos);
              free(elt_script);
              return 0;
            }
          }
          /* Discard the current element if necessary */
          if (0 == keep && 0 != pos) {
            /* We're discarding as we go for space reasons, so discard this one */
            (void)remove(elt_pos);
          }
        }
        free(elt_pos);
        free(elt_script);
      }
      /* Discard the base element if necessary */
      if (0 == keep && 0 != l) {
        /* We're discarding as we go for space reasons, so discard this one */
        (void)remove(elt_l);
      }
      free(elt_l);
    }
    cur_power *= prime;
    i++;
  }
  /* Failed to terminate on a suitable element */
  if (found) {
    return 0;
  } else {
    return 255;
  }
}

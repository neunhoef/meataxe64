/*
 * $Id: sums.c,v 1.10 2002/09/11 10:02:28 jon Exp $
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
#include "utils.h"

int sums(const char *out, unsigned int n, unsigned int argc, const char *const args[],
         unsigned int sub_order, accept acceptor, const char *name)
{
  char *buf;
  const char *in1 = args[0], *in2 = args[2];
  FILE *f;
  unsigned int i, j, k, l, r, s, cur_word = 0, cur_power = 1, base_prime;
  unsigned int prime, nod, nor, noc, count;
  unsigned int o_a = strtoul(args[1], NULL, 0), o_b = strtoul(args[3], NULL, 0);
  const header *h;
  int m;
  const char **names;
  const char **words;
  const char **elts;
  const char **elt_names;

  assert(NULL != in1);
  assert(NULL != in2);
  assert(NULL != out);
  assert(NULL != args);
  assert(0 != argc);
  NOT_USED(argc);
  if (0 == o_a || 0 == o_b || 0 == n) {
    fprintf(stderr, "%s: unexpected zero in order of a, order of b or n, terminating\n", name);
    exit(1);
  }
  if (0 != sub_order && 0 == is_a_prime_power(sub_order)) {
    fprintf(stderr, "%s: bad value %d for subfield order, terminating\n", name, sub_order);
    exit(1);
  }
  if (0 == open_and_read_binary_header(&f, &h, in1, name)) {
    exit(1);
  }
  fclose(f);
  prime = header_get_prime(h);
  if (1 == prime) {
    header_free(h);
    /* Try input 2 */
    if (0 == open_and_read_binary_header(&f, &h, in2, name)) {
      exit(1);
    }
    fclose(f);
    prime = header_get_prime(h);
    if (1 == prime) {
      header_free(h);
      fprintf(stderr, "%s: cannot handle both have both generators as permutations\n", name);
      exit(1);
    }
  }
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  nod = header_get_nod(h); /* For printing element names */
  header_free(h);
  if (nor != noc) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, in1);
    exit(1);
  }
  if (0 != sub_order) {
    if (0 != prime % sub_order) {
      fprintf(stderr, "%s: %d is not a field order, terminating\n", name, sub_order);
      exit(1);
    }
    base_prime = prime_divisor(prime);
    if (0 != prime_index(prime, base_prime) % prime_index(sub_order, base_prime)) {
      fprintf(stderr, "%s: %d is not a sub field order for %d, terminating\n", name, sub_order, prime);
      exit(1);
    }
  }
  i = 1;
  m = 0;
  j = strlen(out);
  k = j + 13;
  n += 1;
  if (0 == int_pow((0 != sub_order) ? sub_order : prime, n, &count)) {
    fprintf(stderr, "%s: too many elements requested (%d ** %d), terminating\n", name, prime, n);
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
    exit(1);
  }
  elt_names[0] = "I";
  words[0] = "";
  names[0] = "I";
  names[1] = in1;
  names[2] = in2;
  if (0 != sub_order) {
    prime = sub_order; /* Restrict to subfield if requested */
  }
  while (i < n) {
    char *a;
    const char *b;
    const char *c;
    const char *chosen_letter;
    unsigned int word_len;
    if (i > 2) {
      buf = my_malloc(2 * k);
      sprintf(buf, "%s%d", out, i - 1);
      names[i] = buf;
    }
/*
    printf("New loop, i = %d\n", i);
*/
    while (1) {
      const char *word = words[cur_word];
      char letter = (0 == m) ? 'A' : 'B';
      unsigned int order = (0 == m) ? o_a : o_b;
      unsigned int len = strlen(word);
      /* Now find maximum number of occurrences of letter at end of word */
      /* and move on if exceeds order */
      if (NULL == strchr(word, letter)) {
        /* No occurrence, all safe */
        break;
      } else {
        unsigned int pos = len;
        /* Count occurrences at end of word */
        while (pos > 0) {
          if (word[pos - 1] == letter) {
            pos--;
          } else {
            break;
          }
        }
        if (len + 1 >= order + pos) {
          /* we've reached the order of this element */
          if (0 == m) {
            m = 1;
            /* Safe to break here, as we can't have a repeat of both letters at the end */
            break;
          } else {
            m = 0;
            /* Try a new word, with first letter */
            /* This may fail if first letter has order 2 */
            cur_word++;
            assert(cur_word < i);
          }
        } else {
          break;
          /* Break anyway, our append was safe */
        }
      }
    }
    /* Now cur_word is a pointer to a word we can safely append our letter to */
    assert(cur_word < i);
    chosen_letter = (0 == m) ? "A" : "B";
    word_len = strlen(words[cur_word]);
    a = my_malloc(2 * word_len + 2);
    strcpy(a, words[cur_word]);
    strcat(a, chosen_letter);
    words[i] = a;
    c = names[cur_word];
    b = (0 == m) ? in1 : in2;
    if (0 != word_len) {
      if (0 == mul(c, b, names[i], "zsums")) {
        exit(1);
      }
/*
      printf("Multiplying %s(%s) * %s(%s) giving %s(%s)\n", c, words[cur_word], b, chosen_letter, names[i], words[i]);
    } else {
      printf("No multiplication required to produce %s\n", names[i]);
*/
    }
    if (0 == m) {
      m = 1;
    } else {
      m = 0;
      cur_word++;
    }
    /* Now compute the sums we require, and their ranks */
    for (l = 0; l < cur_power; l++) {
      for (r = 1; r < prime; r++) {
        unsigned int pos = cur_power * r + l;
        /* Range from cur_power to cur_power * prime - 1 */
        /* scale names[i] and add to elts[l] */
        buf = my_malloc(strlen(elt_names[l]) + 1 + nod + strlen(words[i]) + 1);
        sprintf(buf, "%s+%d%s", elt_names[l], r, words[i]);
        elt_names[pos] = buf;
        if (0 == scaled_add(names[i], elts[l], elts[pos], r, name)) {
          fprintf(stderr, "%s: scaled add failed on %s + %d * %s, terminating\n",
                  name, elts[l], r, names[i - 1]);
/*
          printf("Scaled add %s(%s) + %d.%s(%s) giving %s(%s)\n", elts[l], elt_names[l], r, names[i], words[i], elts[pos], elt_names[pos]);
*/
          exit(1);
        }
        /* l != 0 mean we have 1 + old element + lambda * new element, ie at least 3 in sum */
        if (0 != l) {
          int res;
/*
          printf("Checking rank of %s(%s)\n", elts[pos], elt_names[pos]);
*/
          if (0 == rank(elts[pos], &s, name)) {
            exit(1);
          }
          if (verbose) {
            printf("%s: checking element %s of rank %d\n", name, elt_names[pos], s);
            fflush(stdout);
          }
          res = (*acceptor)(s, nor, elts[pos], elt_names[pos]);
          if (res & 1) {
            printf("%s: found element %s of nullity %d, form %s\n",
                   name, elts[pos], nor - s, elt_names[pos]);
          }
          if (res & 2) {
            printf("%s: terminating\n", name);
            return 0;
          }
        }
      }
    }
    cur_power *= prime;
    i++;
  }
  /* Failed to find a suitable element */
  return 1;
}

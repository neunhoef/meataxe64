/*
 * $Id: sums.c,v 1.4 2002/03/20 18:42:30 jon Exp $
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
#include "read.h"
#include "rn.h"
#include "utils.h"

int sums(const char *in1, const char *in2, const char *out,
         unsigned int o_a, unsigned o_b, unsigned int n,
         const char *name, accept acceptor)
{
  char *buf;
  FILE *f;
  unsigned int i, j, k, l, r, s, cur_word = 0, cur_power = 1;
  unsigned int prime, nod, nor, noc, count;
  const header *h;
  int m;
  const char **names;
  const char **words;
  const char **elts;
  const char **elt_names;

  if (0 == open_and_read_binary_header(&f, &h, in1, name)) {
    exit(1);
  }
  prime = header_get_prime(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  nod = header_get_nod(h); /* For printing element names */
  if (nor != noc) {
    fprintf(stderr, "%s: %s is not square, terminating\n", name, in1);
    exit(1);
  }
  header_free(h);
  fclose(f);
  i = 1;
  m = 0;
  j = strlen(out);
  k = j + 13;
  n += 1;
  if (0 == int_pow(prime, n, &count)) {
    fprintf(stderr, "%s: too many elements requested (%d ** %d), terminating\n", name, prime, n);
    exit(1);
  }
  names = my_malloc(n * sizeof(const char *));
  words = my_malloc(n * sizeof(const char *));
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
  words[0] = "A";
  names[0] = in1;
  for (l = 1; l < prime; l++) {
    /* lambda names[0] + elts[0] */
    if (0 == scaled_add(names[0], elts[0], elts[l], l, name)) {
      fprintf(stderr, "%s: scaled add failed on %s + %d * %s, terminating\n",
              name, elts[l], l, names[i - 1]);
      exit(1);
    }
    buf = my_malloc(2 /*I+*/ + nod /* lambda */ + 1 /* A */ + 1 /* eos */);
    sprintf(buf, "I+%d%s", l, words[0]);
    elt_names[l] = buf;
  }
  while (i < n) {
    char *a;
    const char *b;
    const char *c;
    const char *chosen_letter;
    unsigned int word_len;
    buf = my_malloc(2 * k);
    sprintf(buf, "%s%d", out, i - 1);
    names[i] = buf;
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
          } else {
            m = 0;
            cur_word++;
            assert(cur_word < i);
          }
        }
        break;
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
    if (0 == mul(c, b, names[i], "zsums")) {
      exit(1);
    }
    i++;
    if (0 == m) {
      m = 1;
    } else {
      m = 0;
      cur_word++;
    }
    /* Now compute the sums we require, and their ranks */
    cur_power *= prime;
    for (l = 0; l < cur_power; l++) {
      for (r = 1; r < prime; r++) {
        unsigned int pos = cur_power * r + l;
        /* Range from cur_power to cur_power * prime - 1 */
        /* scale names[i-1] and add to elts[l] */
        if (0 == scaled_add(names[i - 1], elts[l], elts[pos], r, name)) {
          fprintf(stderr, "%s: scaled add failed on %s + %d * %s, terminating\n",
                  name, elts[l], r, names[i - 1]);
          exit(1);
        }
        buf = my_malloc(strlen(elt_names[l]) + 1 + nod + strlen(words[i-1]) + 1);
        sprintf(buf, "%s+%d%s", elt_names[l], r, words[i - 1]);
        elt_names[pos] = buf;
        /* l != 0 mean we have 1 + old element + lambda * new element, ie at least 3 in sum */
        if (0 != l) {
          int res;
          if (0 == rank(elts[pos], &s, name)) {
            exit(1);
          }
          res = (*acceptor)(s, nor);
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
  }
  /* Failed to find a suitable element */
  printf("Failed to find a suitable element\n");
  return 1;
}

/*
 * $Id: sums_utils.c,v 1.1 2003/08/04 20:41:57 jon Exp $
 *
 * Utilities for sums, sumf etc
 *
 */

#include "sums_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/*#include "primes.h"*/
#include "utils.h"

int next_gen(unsigned int cur_gen, unsigned int max_gen, char *gen, const unsigned int *orders, const char *word)
{
  assert(NULL != gen);
  assert(NULL != orders);
  assert(NULL != word);
  while (1) {
    char letter;
    unsigned int len;
    cur_gen++;
    if (cur_gen >= max_gen) {
      return -1;
    }
    if (0 == orders[cur_gen]) {
      continue;
    }
    letter = 'A' + cur_gen;
    len = strlen(word);
    /* Now find maximum number of occurrences of letter at end of word */
    /* and move on if exceeds order */
    if (NULL == strchr(word, letter)) {
      /* No occurrence, all safe */
      *gen = letter;
      return cur_gen;
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
      if (len + 1 >= orders[cur_gen] + pos) {
        /* we've reached the order of this element */
        continue;
      }
      /* Safe to use this generator */
      *gen = letter;
      return cur_gen;
    }
  } /* while */
}

static int find_word(const char *word, const char **words, unsigned int max_gen)
{
  unsigned int i;
  assert(NULL != word);
  assert(NULL != words);
  for (i = 0; i < max_gen; i++) {
    assert(NULL != words[i]);
    if (0 == strcmp(word, words[i])) {
      return (int)i;
    }
  }
  return -1; /* Not found */
}

int ignore_word(unsigned int word, unsigned int max_prod, const char **words, unsigned int gen, unsigned int order, unsigned int prime)
{
  int found_I = 0, meets_current = 0, new_word = 0, ignore;
  unsigned int scalar = 1, i, power = prime, prev_power = 1;
  char letter = 'A' + gen;
  assert(NULL != words);
  /* First look at left multiplication */
  for (i = 1; i < max_prod && 0 == new_word; i++) {
    unsigned int coeff = (word % power) / prev_power;
    if (0 != coeff) {
      unsigned int len, j;
      int reduces = 1;
      char *w;
      assert(NULL != words[i]);
      len = 1 + strlen(words[i]) + 1;
      w = my_malloc(len);
      *w = letter;
      w[1] = '\0';
      strcat(w, words[i]);
      for (j = 0; j < order && j < len && reduces; j++) {
        reduces = w[j] == letter;
      }
      if (reduces) {
        if (len == order + 1) {
          scalar = coeff;
          found_I = 1;
        }
      } else {
        int num = find_word(w, words, max_prod);
        if (num < 0) {
          new_word = 1;
        } else if ((unsigned)num + 1 == max_prod) {
          meets_current = 1;
        }
      }
      free(w);
    }
    prev_power = power;
    power *= prime;
  }
  if (0 != new_word || 0 == found_I) {
    ignore = 0; /* Can't ignore */
  } else if (0 == meets_current) {
    ignore = 1;
  } else {
    ignore = 0; /* Can't ignore */
  }
  /* Then look at right multiplication, if we can't guarantee to ignore it */
  if (0 == ignore) {
    prev_power = 1;
    power = prime;
    new_word = 0;
    found_I = 0;
    meets_current = 0;
    for (i = 1; i < max_prod && 0 == new_word; i++) {
      unsigned int coeff = (word % power) / prev_power;
      if (0 != coeff) {
        unsigned int len, j;
        int reduces = 1;
        char *w;
        assert(NULL != words[i]);
        len = 1 + strlen(words[i]) + 1;
        w = my_malloc(len);
        strcpy(w, words[i]);
        w[len - 2] = letter;
        w[len - 1] = '\0';
        for (j = len; j + order > len && j > 0 && reduces; j--) {
          reduces = w[j - 2] == letter;
        }
        if (reduces) {
          if (len == order + 1) {
            scalar = coeff;
            found_I = 1;
          }
        } else {
          int num = find_word(w, words, max_prod);
          if (num < 0) {
            new_word = 1;
          } else if ((unsigned)num + 1 == max_prod) {
            meets_current = 1;
          }
        }
        free(w);
      }
      prev_power = power;
      power *= prime;
    }
    if (0 != new_word || 0 == found_I) {
      ignore = 0; /* Can't ignore */
    } else if (0 == meets_current) {
      ignore = 1;
    } else {
      ignore = 0; /* Can't ignore */
    }
  }
  return ignore;
}

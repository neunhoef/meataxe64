/*
 * $Id: sums_utils.c,v 1.4 2004/11/18 15:29:44 jon Exp $
 *
 * Utilities for sums, sumf etc
 *
 */

#include "sums_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "add.h"
#include "utils.h"

void copy_string(char **out, const char *in)
{
  unsigned int len;
  assert(NULL != in);
  assert(NULL != out);
  if (NULL != *out) {
    free(*out);
  }
  len = strlen(in);
  *out = my_malloc(len + 1);
  strcpy(*out, in);
}

char *make_elt_script(unsigned int prime_power, unsigned int cur_power,
                      unsigned int r, unsigned int l, unsigned int i,
                      unsigned int nod, const char **words)
{
  char *initial, *name;
  assert(r < prime_power);
  assert(l < cur_power);
  assert(NULL != words);
  /* First see if a recursion is required */
  if (prime_power <= cur_power) {
    /* Need to recurse */
    unsigned int s, m, n;
    n = cur_power / prime_power;
    s = l / n;
    m = l % n;
    initial = make_elt_script(prime_power, n, s, m, i - 1, nod, words);
  } else {
    /* We're starting with I+gen */
    assert(1 == cur_power);
    assert(0 == l);
    assert(1 == i);
    initial = my_malloc(2 /* I */);
    sprintf(initial, "%s", "I");
  }
  /* Now add in the current word */
  name = my_malloc(strlen(initial) + strlen(words[i]) + nod + 2 /* + and 0 */);
  if (0 == r) {
    sprintf(name, "%s", initial);
  } else {
    sprintf(name, "%s+%d%s", initial, r, words[i]);
  }
  free(initial);
  return name;
}

char *make_elt_name(const char *base, unsigned int num)
{
  unsigned int len = strlen(base);
  unsigned int digits = ((CHAR_BIT) * sizeof(unsigned int) + 2) / 3;
  /* Enought digits to print in decimal */
  char *name = my_malloc(len + 1 + 3 + digits);
  sprintf(name, "%s.e.%d", base, num);
  return name;
}

int make_element(unsigned int pos, unsigned int prime, unsigned int prime_power,
                 unsigned int i, const char **names, const char *base, const char *name)
{
  assert(pos < prime_power);
  /* prime_power = prime ** (i - 1) */
  if (0 != pos) {
    unsigned int l, r;
    char *elt_pos, *elt_l;
    if (pos >= prime) {
      assert(prime_power >= prime * prime);
      assert(0 == prime_power % prime);
      prime_power /= prime;
      /* Make the earlier element we need */
      l = pos % prime_power;
      i--;
      if (0 == make_element(l, prime, prime_power, i, names, base, name)) {
        return 0;
      }
      if (pos >= prime_power) {
        r = pos / prime_power;
      } else {
        return 1;
      }
    } else {
      l = 0;
      i = 1;
      r = pos;
    }
    elt_l = make_elt_name(base, l);
    elt_pos = make_elt_name(base, pos);
    if (0 == scaled_add(names[i], elt_l, elt_pos, r, name)) {
      free(elt_l);
      free(elt_pos);
      return 0;
    }
    /* Delete any earlier element we made */
    if (pos >= prime && 0 != l) {
      (void)remove(elt_l);
    }
    free(elt_l);
    free(elt_pos);
  }
  return 1;
}

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

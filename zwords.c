/*
 * $Id: zwords.c,v 1.1 2001/12/04 23:14:47 jon Exp $
 *
 * Compute words in two matrices
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "memory.h"
#include "mul.h"
#include "utils.h"

static const char *name = "zwords";

static void words_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file a> <in_file b> <out_file_stem> <order a> <order b> <n> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;
  unsigned int memory = MEM_SIZE;
  unsigned int o_a, o_b, n, i, j, k, cur_word = 0;
  int m;
  const char **names;
  const char **words;

  if (7 != argc && 8 != argc) {
    words_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  o_a = strtoul(argv[4], NULL, 0);
  o_b = strtoul(argv[5], NULL, 0);
  n = strtoul(argv[6], NULL, 0);
  if (8 == argc) {
    memory = strtoul(argv[7], NULL, 0);
  }
  if (0 == n) {
    fprintf(stderr, "%s: no words requested\n", name);
    exit(1);
  }
  /* Don't worry if either is zero */
  memory_init(name, memory);
  i = 1;
  m = 0;
  j = strlen(out);
  k = j + 13;
  n += 1;
  names = my_malloc((n) * sizeof(const char *));
  words = my_malloc((n) * sizeof(const char *));
  words[0] = "A";
  names[0] = in1;
  while (i < n) {
    char *buf = my_malloc(k);
    char *a;
    const char *b;
    const char *c;
    const char *chosen_letter;
    unsigned int word_len;
    sprintf(buf, "%s%d", out, i - 1);
    a = my_malloc(k);
    strcpy(a, buf);
    free(buf);
    names[i] = a;
    while (1) {
      const char *word = words[cur_word];
      char letter = (0 == m) ? 'A' : 'B';
      unsigned int order = (0 == m) ? o_a : o_b;
      unsigned int len = strlen(word);
      /* Now find maximum number of letter at end of word */
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
          /* Oh dear, we've reached the order of this element */
          /* Increment etc */
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
    a = my_malloc(word_len + 2);
    strcpy(a, words[cur_word]);
    strcat(a, chosen_letter);
    words[i] = a;
    c = names[cur_word];
    b = (0 == m) ? in1 : in2;
    printf("Computing %s in %s from %s, %s\n", words[i], names[i], words[cur_word], b);
    if (0 == mul(c, b, names[i], "zwords")) {
      exit(1);
    }
    i++;
    if (0 == m) {
      m = 1;
    } else {
      m = 0;
      cur_word++;
    }
  }
  memory_dispose();
  return 0;
}

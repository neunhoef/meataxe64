/*
      zsums.c     meataxe-64 find element of given nullity
      =======     J. G. Thackray   12.05.2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "field.h"
#include "mfuns.h"
#include "io.h"
#include "slab.h"
#include "funs.h"
#include "bitstring.h"
#include "util.h"
#include "utils.h"
#include "primes.h"
#include "sums_utils.h"

static void cleanup(u32 *orders)
{
  assert(NULL != orders);
  free(orders);
}

static u32 nullity = 0;

static const char prog_name[] = "zsums";

static void script_usage(void)
{
  fprintf(stderr, "%s: usage: %s <out_file_stem> <n> <nullity> <in_file a> <order a> <in_file b> <order b>\n", prog_name, prog_name);
}

static int acceptor(u32 rank, u32 nor, const char *file, const char *form)
{
  NOT_USED(file);
  NOT_USED(form);
  if (rank < nor && rank + nullity >= nor) {
    return 3;
  } else {
    return 0;
  }
}

char *fun_tmp;

int main(int argc, const char *argv[])
{
  const char *out;
  char *buf;
  u32 *orders;
  unsigned int argc2 = argc / 2;
  u32 n, sub_order = 0, keep = 0, invertible = 1, verbose = 1;
  u32 i, j, k, l, r, s, cur_word = 0, cur_power = 1, base_prime;
  u32 prime = 1, nod = 0, nor = 0, noc = 0, count;
  u32 order_sum = 0;
  int m;
  int found = 0;
  const char **names;
  const char **words;
  const char *gen_names[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
  char *elt_l, *elt_pos;
  const char *tmp_root = tmp_name();
  /* How long the temporary filename root is */
  size_t tmp_len = strlen(tmp_root);

  CLogCmd(argc, argv);
  if (argc2 > 8 || argc < 5) {
    script_usage();
    exit(1);
  }
  out = argv[1];
  n = strtoul(argv[2], NULL, 0);
  nullity = strtoul(argv[3], NULL, 0);
  if (0 == n) {
    fprintf(stderr, "%s: no sums requested\n", prog_name);
    exit(1);
  }
  /* Temporary root for functions */
  fun_tmp = malloc(tmp_len + sizeof(FUN_TMP) + 1);
  strcpy(fun_tmp, tmp_root);
  strcat(fun_tmp, FUN_TMP);
  argv += 4;
  argc -= 4;
  argc2 -= 2;
  orders = my_malloc(argc2 * sizeof(u32));
  for (i = 0; i < argc2; i++) {
    orders[i] = strtoul(argv[1 + 2 * i], NULL, 0);
    order_sum += orders[i];
  }
  if (2 > order_sum || 0 == n) {
    fprintf(stderr, "%s: unexpected zeroes in orders, or zero depth, terminating\n", prog_name);
    cleanup(orders);
    exit(1);
  }
  if (0 != sub_order && 0 == is_a_prime_power(sub_order)) {
    fprintf(stderr, "%s: bad value %u for subfield order, terminating\n", prog_name, sub_order);
    cleanup(orders);
    exit(1);
  }
  for (i = 0; i < argc2; i++) {
    header hdr;
    EPeek(argv[2 * i], hdr.hdr);
    if (1 != hdr.named.fdef) {
      prime = hdr.named.fdef;
      nor = hdr.named.nor;
      noc = hdr.named.noc;
      nod = digits_of(prime); /* For printing element names */
      break;
    }
  }
  if (1 == prime) {
    fprintf(stderr, "%s: cannot handle both have both generators as permutations\n", prog_name);
    cleanup(orders);
    exit(1);
  }
  if (nor != noc) {
    fprintf(stderr, "%s: %s is not square, terminating\n", prog_name, argv[2 * i]);
    cleanup(orders);
    exit(1);
  }
  if (0 != sub_order) {
    if (0 != prime % sub_order) {
      fprintf(stderr, "%s: %u is not a field order, terminating\n", prog_name, sub_order);
      cleanup(orders);
      exit(1);
    }
    base_prime = prime_divisor(prime);
    if (0 != prime_index(prime, base_prime) % prime_index(sub_order, base_prime)) {
      fprintf(stderr, "%s: %u is not a sub field order for %u, terminating\n", prog_name, sub_order, prime);
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
    fprintf(stderr, "%s: too many elements requested (%u ** %u), terminating\n", prog_name, prime, n);
    cleanup(orders);
    exit(1);
  }
  names = my_malloc((n + 1) * sizeof(const char *));
  words = my_malloc((n + 1) * sizeof(const char *));
  elt_l = make_elt_name(out, 0);
  if (0 == ident(prime, nor, noc, 1, elt_l)) {
    fprintf(stderr, "%s: cannot write identity, terminating\n", prog_name);
    cleanup(orders);
    exit(1);
  }
  free(elt_l);
  words[0] = "";
  names[0] = "I";
  for (j = 0; j < argc2; j++) {
    names[j + 1] = argv[2 * j];
    words[j + 1] = NULL;
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
      sprintf(buf, "%s%u", out, i - 1);
      names[i] = buf;
    }
    for (;;) {
      const char *word = words[cur_word];
      m = next_gen(m, argc2, &letter, orders, word);
      if (m >= 0) {
        break;
      } else {
        m = -1; /* Will be incremented by next_gen */
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
    b = argv[2 * m];
    if (0 != word_len) {
      fMultiply(fun_tmp, c, 1, b, 1, names[i], 1);
    }
    /* Now compute the sums we require, and their ranks */
    for (l = 0; l < cur_power; l++) {
      /* Make the base element if necessary */
      elt_l = make_elt_name(out, l);
      if (0 == keep) {
        /* We're discarding as we go for space reasons, so we must remake */
        if (0 == make_element(l, prime, cur_power, nor, i, names, out, tmp_root)) {
          fprintf(stderr, "%s: failed to make %s, terminating\n", prog_name, elt_l);
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
                printf("%s: Checking %s with %s\n", prog_name, elt_script, words[j + 1]);
              }
              if (ignore_word(pos, i + 1, words, j, orders[j], prime)) {
                if (verbose) {
                  printf("%s: Ignoring %s after checking with %s\n", prog_name, elt_script, words[j + 1]);
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
            printf("%s: making %s, formula %s by %s + %u * %s\n", prog_name, elt_pos, elt_script, elt_l, r, names[i]);
            fflush(stdout);
          }
          /* scale names[i] and add to elt_l */
          if (0 == scaled_add(names[i], elt_l, elt_pos, r, prime, nor, tmp_root)) {
            fprintf(stderr, "%s: scaled add failed on %s + %u * %s, terminating\n",
                    prog_name, elt_l, r, names[i]);
            cleanup(orders);
            exit(1);
          }
          /* We've made the element, do we want its rank? */
          /* l != 0 mean we have 1 + old element + lambda * new element, ie at least 3 in sum */
          if (0 != l && 0 == ignore) {
            int res;
            s = rank(elt_pos, tmp_root);
            if (verbose) {
              printf("%s: checking element %s of rank %u\n", prog_name, elt_script, s);
              fflush(stdout);
            }
            res = (*acceptor)(s, nor, elt_pos, elt_script);
            if (res & 1) {
              found = 1;
              printf("%s: found element %s of nullity %u, form %s\n",
                     prog_name, elt_pos, nor - s, elt_script);
            }
            if (res & 2) {
              printf("%s: terminating\n", prog_name);
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

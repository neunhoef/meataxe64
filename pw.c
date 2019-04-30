/*
 * Function to search for peak words
 *
 */

#include "pw.h"
#include "add.h"
#include "files.h"
#include "header.h"
#include "ident.h"
#include "mul.h"
#include "read.h"
#include "rn.h"
#include "system.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* An array, one but per code, of things with nullity 0 almost everywhere */
static unsigned char *zeroes;
/* An array, one char per code, of representations on which we have the right rank */
static unsigned char *ones;

/* A root for temporay names */
static const char *tmp_base;
/* The name of an identity */
static char *id_name;

/* An array of names of group elements in creation order */
static char **group_names;
/* An array of final generators for each of the above */
static u32 *multipliers;
/* An array of previous elements */
static u32 *previous;
/* An array of words giving the group elements */
static char **group_words;
/*
 * Each element in group_names apart from the generators
 * is made by group_names[i] = group_names{previous[i]] * group_names[multipliers[i]]
 */

/*
 * A function to create the content of the final three arrays
 */
static int make_group_words(u32 depth, u32 n_gens, u32 *orders)
{
  u32 i, cur_app = 0, cur_base = 0;
  for (i = 0; i < depth; i++) {
    group_words[i] = my_malloc((depth + 1) * sizeof(*(group_words[i])));
  }
  /* Set up those words from generators */
  for (i = 0; i < n_gens; i++) {
    sprintf(group_words[i], "%c", 'A' + i);
    previous[i] = 0;
    multipliers[i] = 0;
  }
  /* And now the rest */
  for (i = n_gens; i < depth; i++) {
    for (;;) {
      if (cur_base >= i) {
        /* We've failed, run out of bases to multiply onto */
        return 0;
      }
      if (cur_app < n_gens) {
        size_t len;
        u32 k = 0;
        /* We can still try appending something to cur_base */
        sprintf(group_words[i], "%s%c", group_words[cur_base], 'A' + cur_app);
        /* Now check if we've exceeded the order of generator i */
        len = strlen(group_words[i]);
        while (len > 0) {
          if (group_words[i][len - 1] == (char)('A' + cur_app)) {
            k++;
            len--;
          } else {
            break;
          }
        }
        if (k >= orders[cur_app]) {
          /* Try next generator */
          cur_app++;
          continue;
        }
        /*
         * Success, we've made a new generator
         * It's form as a word is in group_words[i] already
         * Now set multipliers and previous
         */
        multipliers[i] = cur_app;
        cur_app++; /* Try next multiplier */
        previous[i] = cur_base;
        break; /* We've done this element, move on */
      } else {
        /* Out of generators to append on, increment cur_base */
        cur_base++;
        cur_app = 0;
        continue; /* Try again */
      }
    }
  }
  return 1;
}

static const char *print_ga_element(u64 code, u64 prime, u32 depth)
{
  /* Allocate space, very conservative */
  char *out = my_malloc(2 + (depth + 1) * depth);
  u32 index = 0;
  /* Initialise */
  strcpy(out, "I");
  while (0 != code) {
    u64 rem = code % prime;
    code /= prime;
    if (0 != rem) {
      char digit[3];
      sprintf(digit, "+%" U64_F, rem);
      strcat(out, digit);
      strcat(out, group_words[index]);
    }
    index++;
  }
  return out;
}

/*
 * Produce code in the form x*q^n+y where y < q^n
 * returns y
 * index is n
 * rem is x
 */
static u64 split_code(u64 code, u64 prime_power, u64 *index, u64 *rem, u32 prime)
{
  u64 i, j, k;
  if (prime_power > code) {
    printf("Code %" U64_F ", prime_power %" U64_F "\n", code, prime_power);
    fflush(stdout);
  }
  assert(prime_power <= code);

  i = code % prime_power; /* The remainder, identifies the element we want next time */
  j = code / prime_power; /* The element we want this time */
  *rem = j;
  /* Now get the log of prime_power base prime */
  k = 0;
  while (prime_power >= prime) {
    prime_power /= prime;
    k++;
  }
  *index = k;
  return i;
}

static int make_group_element(u32 index, u32 n_gens, const char *name)
{
  /* Group element is placed in group_names[index] */
  if (index >= n_gens) {
    /* Make the next element */
    /* Multiply previous[index] by multipliers[index] into group_names[index] */
    return mul(group_names[previous[index]],
               group_names[multipliers[index]],
               group_names[index], name);
  }
  return 1;
}

static int make_element(u64 code, u32 prime, u64 prime_power, char *n, const char *name)
{
  u64 base, index, rem;
  int res;

  base = split_code(code, prime_power, &index, &rem, prime);
  sprintf(n, "%s.e.%" U64_F, tmp_base, code);
  /*
   * Now produce the intermediates if we need them
   * We want element base + index * rem, where rem is a group element
   */
  if (0 == base) {
    /* Just add I to index * rem */
    res = scaled_add(id_name, group_names[index], n, rem, name);
    if (0 == res) {
      fprintf(stderr, "%s: failed to create element %" U64_F ", terminating\n", name, code);
      return 0;
    }
  } else {
    char *base_elt;
    size_t len = strlen(tmp_base);

    /* Allocate an element for base */
    base_elt = malloc(len + 64 + 3);
    /* Set up the name */
    sprintf(base_elt, "%s.e.%" U64_F, tmp_base, base);
    while (prime_power > base) {
      prime_power /= prime;
    }
    res = make_element(base, prime, prime_power, base_elt, name);
    if (0 == res) {
      fprintf(stderr, "%s: failed to create element %" U64_F ", terminating\n", name, base);
      return 0;
    }
    res = scaled_add(base_elt, group_names[index], n, rem, name);
    if (0 == res) {
      fprintf(stderr, "%s: failed to create element %" U64_F ", terminating\n", name, code);
      return 0;
    }
    remove(base_elt);
    free(base_elt);
  }
  return 1;
}

int pw(u32 n_irreds, u32 n_gens, int peak, u32 depth, u32 *orders, char **irred_names, u32 *nullities, const char **words, const char *name)
{
  u32 i, j, nor, prime = 0;
  u32 *nors;
  u64 tot_elts = 1, tot_bits, k, prime_power;
  int res;
  const header *headers;
  char *this_element;
  size_t len;

  /* Allocate the sizes */
  nors = my_malloc(n_irreds * sizeof(*nors));
  for (i = 0; i < n_irreds; i++) {
    size_t len = strlen(irred_names[i]);
    char *irred = malloc(len + 2); /* Space for _n */
    nor = 0;
    for (j = 0; j < n_gens; j++) {
      FILE *f;
      int ok;
      sprintf(irred, "%s_%u", irred_names[i], j + 1);
      ok = open_and_read_binary_header(&f, &headers, irred, name);
      if (ok) {
        if (0 == prime) {
          prime = header_get_prime(headers);
        }
        if (0 == nor) {
          nor = header_get_nor(headers);
        }
        if (header_get_noc(headers) != nor || header_get_nor(headers) != nor ||
            header_get_prime(headers) != prime) {
          fprintf(stderr, "%s: incompatible parameter %s, terminating\n", name, irred);
          return 1;
        }
      } else {
        return 1;
      }
    }
    nors[i] = nor;
  }
  /* Compute the depth, checking for overflow */
  for (i = 0; i < depth; i++) {
    tot_elts *= prime;
    if (0 != tot_elts % prime) {
      fprintf(stderr, "%s: depth %" U32_F " too great, terminating\n", name, depth);
      return 1;
    }
  }
  tot_bits = (tot_elts + 7) / 8;
  /* Now allocate the zeroes and ones arrays */
  zeroes = malloc(tot_bits); /* One bit per entry */
  ones = malloc(tot_elts); /* One byte per entry */
  memset(zeroes, 0, tot_bits);
  memset(ones, 0, tot_elts);
  tmp_base = tmp_name();
  len = strlen(tmp_base);
  /*
   * Allocate the group elements and previous and multipliers
   * Note: the first n_gens are irreducible names,
   * set each time round the loop
   */
  group_names = my_malloc(depth * sizeof(*group_names));
  previous = my_malloc(depth * sizeof(*previous));
  multipliers = my_malloc(depth * sizeof(*multipliers));
  group_words = my_malloc(depth * sizeof(*group_words));
  res = make_group_words(depth, n_gens, orders);
  if (0 == res) {
    fprintf(stderr, "%s: cannot make depth %" U32_F ", terminating\n", name, depth);
    exit(1);
  }
  /*
   * We allocate those that will be produced by the program
   * For those that are generators, we do the (de)allocation in the loop
   */
  for (i = n_gens; i < depth; i++) {
    group_names[i] = malloc(len + 64 + 2);
    sprintf(group_names[i], "%s.e%" U32_F, tmp_base, i);
  }
  /* Allocate some names */
  id_name = malloc(len + 64 + 3); /* 3 for ".e." and 64 (maximum) for index */
  this_element = malloc(len + 64 + 3); /* Same space for current element */
  sprintf(id_name, "%s.e.%u", tmp_base, 0);
  /* Now loop over the irreducibles */
  for (i = 0; i < n_irreds; i++) {
    u32 index = 0;
    nor = nors[i];
    /* First make an identity, element 0 */
    res = ident(prime, nor, nor, 1, id_name, name);
    if (0 == res) {
      fprintf(stderr, "%s: failed to create identity, terminating\n", name);
      exit(1);
    }
    /* Now start making elements and checking ranks */
    len = strlen(irred_names[i]);
    for (j = 0; j < n_gens; j++) {
      group_names[j] = malloc(len + 2); /* Enough for _<digit> */
      sprintf(group_names[j], "%s_%" U32_F, irred_names[i], j + 1);
    }
    prime_power = 1;
    for (k = 1; k < tot_elts; k++) {
      /* Are we looking at a valid code in the nullities array */
      u64 l = k / 8, m = k % 8;
      u32 r;

      if (prime_power * prime <= k) {
        prime_power *= prime;
        /* Make the next element */
        index++;
        make_group_element(index, n_gens, name);
      }
      if (zeroes[l] & (1 << m)) {
#if 0
        printf("Ignoring code %" U64_F " as invalid\n", k);
        fflush(stdout);
#endif
        continue; /* Not in the allowable nullities array */
      }
      res = make_element(k, prime, prime_power, this_element, name);
      if (0 == res) { 
        fprintf(stderr, "%s: failed to create element %" U64_F ", terminating\n", name, k);
        exit(1);
      }
      res = rank(this_element, &r, name);
      if (0 == res) { 
        fprintf(stderr, "%s: failed to get rank of element %" U64_F ", terminating\n", name, k);
        exit(1);
      }
      if (r == nor) {
        /* Nullity zero, delete and continue */
#if 0
        printf("Code %" U64_F " has nullity 0\n", k);
        fflush(stdout);
#endif
        remove(this_element);
        continue;
      }
      if (nor != nullities[i] + r || 0 != ones[k]) {
        /* Wrong nullity, throw away from zeroes, or already used for another irred */
        zeroes[l] |= 1 << m; /* Set this bit so we don't try again */
        ones[k] = 0; /* No good here either */
        remove(this_element);
#if 0
        printf("Code %" U64_F " has wrong nulity\n", k);
        fflush(stdout);
#endif
        continue;
      }
      /* This one looks ok */
      /* TBD: peak test */
#if 0
      printf("Code %" U64_F " with rank %" U32_F" added to ones\n", k, r);
      fflush(stdout);
#endif
      ones[k] = i + 1;
      remove(this_element);
    }
    for (j = 0; j < n_gens; j++) {
      free(group_names[j]); /* The next irred may be longer */
    }
  }
  /* Output results */
  for (i = 0; i < n_irreds; i++) {
    words[i] = NULL; /* Initialise */
  }
  for (k = 1; k < tot_elts; k++) {
    u64 l = k / 8, m = k % 8;
    if (zeroes[l] & (1 << m)) {
      continue; /* This one ruled out completely */
    }
    if (0 != ones[k] && NULL == words[ones[k] - 1]) {
      /* A hit that we've yet to output */
      words[ones[k] - 1] = print_ga_element(k, prime, depth);
      printf("irred %" U32_F ", %s\n", ones[k] - 1, words[ones[k] - 1]);
    }
  }
  for (j = n_gens; j < depth; j++) {
    remove(group_names[j]);
    free(group_names[j]);
  }
  free(group_names);
  NOT_USED(peak);
  return 0;
}

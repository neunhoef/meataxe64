/*
 * $Id: zfln.c,v 1.11 2004/11/18 15:29:44 jon Exp $
 *
 * Compute sums in the group algebra in two matrices finding one of lowest non-zero nullity
 * Expected to be used during computation of standard bases of irreducible but not absolutely
 * irreducible representations
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "sums.h"
#include "sums_utils.h"
#include "utils.h"

static const char *name = "zfln";

static unsigned int nullity = 0;

static unsigned int gcd = 0;

static char *best_name = NULL;
static char *best_form = NULL;

static void fln_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <out_file_stem> <n> <nullity> <in_file a> <order a> <in_file b> <order b>\n", name, name, parse_usage());
}

static void copy_names(const char *file, const char *form)
{
  copy_string(&best_name, file);
  copy_string(&best_form, form);
}

static unsigned int find_gcd(unsigned int a, unsigned int b)
{
  unsigned int n, m, r;
  assert(0 != a);
  assert(0 != b);
  n = (a > b) ? a : b;
  m = (a > b) ? b : a;
  r = n % m;
  if (0 == r) {
    return m;
  } else {
    return find_gcd(m, r);
  }
}

static int acceptor(unsigned int rank, unsigned int nor, const char *file, const char *form)
{
  assert(NULL != file);
  assert(NULL != form);
  if (0 == nullity) {
    if (rank == nor) {
      copy_names(file, form);
      gcd = 1;
      return 2;
    } else {
      return 0;
    }
  } else if (rank < nor && rank + nullity >= nor) {
    if (0 == gcd) {
      /* First possible candidate */
      gcd = nor - rank;
      copy_names(file, form);
    } else {
      /* Check if this reduces the gcd */
      unsigned int new = find_gcd(gcd, nor - rank);        
      assert(0 != new);
      if (new < gcd) {
        gcd = new;
        /* See if this element worked */
        if (rank + gcd == nor) {
          copy_names(file, form);
        } else {
          best_name = NULL;
          best_form = NULL;
        }
      } else if (NULL == best_name && gcd + rank == nor) {
        /* This element achieves a previously unachieved gcd */
        copy_names(file, form);
      }
    }
    if (1 == gcd) {
      return 2;
    } else {
      return 0;
    }
  } else {
    /* Not a candidate */
    return 0;
  }
}

int main(int argc, const char * const argv[])
{
  unsigned int n;
  int res;

  argv = parse_line(argc, argv, &argc);
  if (8 > argc || 0 != argc % 2) {
    fln_usage();
    exit(1);
  }
  n = strtoul(argv[2], NULL, 0);
  nullity = strtoul(argv[3], NULL, 0);
  if (0 == n) {
    fprintf(stderr, "%s: no ranks requested\n", name);
    exit(1);
  }
  endian_init();
  memory_init(name, memory);
  res = sums(argv[1], n, argc - 4, argv + 4, 0, &acceptor, 1, 1, name);
  memory_dispose();
  if (0 != gcd && NULL != best_name) {
    if (0 == nullity) {
      printf("%s: found element %s of nullity 0, form %s\n",
             name, best_name, best_form);
    } else {
      printf("%s: found element %s of nullity %d, form %s\n",
             name, best_name, gcd, best_form);
    }
    return 0;
  } else {
    printf("Failed to find a suitable element\n");
    return 255;
  }
}

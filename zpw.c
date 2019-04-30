/*
 * Find peak words
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pw.h"
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "utils.h"

static const char *name = "zpw";

static void pw_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <number of irreducibles> <number of generators> <peak(1)/delta(0) words> <depth> [<element order>] [<irreducible name> <nullity>]\n", name, name, parse_usage());
}

int main(int argc, const char *const argv[])
{
  char **irreds;
  u32 *nullities;
  u32 *orders;
  const char **words;
  u32 n_irreds, n_gens, depth, i;
  int peak;

  argv = parse_line(argc, argv, &argc);
  if (5 > argc) {
    pw_usage();
    exit(1);
  }
  n_irreds = strtoul(argv[1], NULL, 0);
  n_gens = strtoul(argv[2], NULL, 0);
  peak = strtoul(argv[3], NULL, 0);
  depth = strtoul(argv[4], NULL, 0);
  argc -= 4;
  argv += 4;
  if ((u32)argc != n_gens + n_irreds * 2 + 1 || 0 == n_gens) {
    pw_usage();
    exit(1);
  }
  /* Read orders */
  orders = my_malloc(n_gens * sizeof(*orders));
  for (i = 0; i < n_gens; i++) {
    orders[i] = strtoul(argv[1 + i], NULL, 0);
  }
  argv += n_gens;
  argc -= n_gens;
  nullities = my_malloc(n_irreds * sizeof(*nullities));
  /* Read nullities */
  for (i = 0; i < n_irreds; i++) {
    nullities[i] = strtoul(argv[2 + 2 * i], NULL, 0);
  }
  irreds = my_malloc(n_irreds * sizeof(*irreds));
  /* Read irreds */
  for (i = 0; i < n_irreds; i++) {
    irreds[i] = my_malloc(strlen(argv[1 + 2 * i]));
    strcpy (irreds[i], argv[1 + 2 * i]);
  }
  words = my_malloc(n_irreds * sizeof(*words));
  memory_init(name, memory);
  endian_init();
  if (0 == pw(n_irreds, n_gens, peak, depth, orders, irreds, nullities, words, name)) {
    exit(1);
  }
  /* TBD: print answer */
  memory_dispose();
  return 0;
}

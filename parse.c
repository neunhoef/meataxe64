/*
 * $Id: parse.c,v 1.2 2002/10/12 14:17:06 jon Exp $
 *
 * Function to parse command line flags
 *
 */

#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef enum parse_element
{
  nullary,
  unary
} parse_element_type;

typedef struct parse_element_struct
{
  int *flag;
  int set_value;
  parse_element_type type;
  const char *tag;
  const char *env_tag;
} parse_element, *pparse_element;

int verbose = 0;

unsigned int memory = MEM_SIZE;

static parse_element parse_table[] =
{
  {
    &verbose,
    1,
    nullary,
    "-v",
    "mtx_verbose"
  },
  {
    &verbose,
    0,
    nullary,
    "-q",
    "mtx_quiet"
  },
  {
    &memory,
    1,
    unary,
    "-m",
    "mtx_memory"
  },
};

static void set_env_values(void)
{
  unsigned int len = sizeof(parse_table) / sizeof(parse_element);
  unsigned int i;
  for(i = 0; i < len; i++) {
    if (NULL != parse_table[i].env_tag) {
      const char *env_val = getenv(parse_table[i].env_tag);
      if (NULL != env_val) {
        *(parse_table[i].flag) = parse_table[i].set_value;
      }
    }
  }
}

static unsigned int parse_tag(const char *tag, const char *val, int argc)
{
  unsigned int len = sizeof(parse_table) / sizeof(parse_element);
  unsigned int i;
  for(i = 0; i < len; i++) {
    pparse_element elt = parse_table + i;
    if ((nullary == elt->type || (unary == elt->type && argc > 2)) && 0 == strcmp(tag, elt->tag)) {
      switch (elt->type) {
      case nullary:
        *(parse_table[i].flag) = parse_table[i].set_value;
        return 1;
      case unary:
        *(parse_table[i].flag) = strtoul(val, NULL, 0);
        return 2;
      default:
        assert(0);
      }
    }
  }
  return 0;
}

const char *const *parse_line(int argc, const char *const argv[], int *new_argc)
{
  unsigned int n;
  assert(NULL != argv);
  assert(NULL != new_argc);
  set_env_values();
  while (argc > 1) {
    n = parse_tag(argv[1], (argc > 2) ? argv[2] : NULL, argc);
    if (0 == n) {
      break;
    }
    argc -= n;
    argv += n;
  }
  *new_argc = argc;
  return argv;
}

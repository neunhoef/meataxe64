/*
 * $Id: parse.c,v 1.11 2005/07/24 11:31:35 jon Exp $
 *
 * Function to parse command line flags
 *
 */

#include "parse.h"
#include "utils.h"
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
  unsigned int *flag;
  unsigned int set_value;
  parse_element_type type;
  const char *tag;
  const char *env_tag;
} parse_element, *pparse_element;

u32 verbose = 0;

u32 memory = MEM_SIZE;

u32 max_grease = 32;

unsigned int maximum_rows = 0;

static parse_element parse_table[] =
{
  {
    &max_grease,
    1,
    unary,
    "-mg",
    "mtx_max_grease"
  },
  {
    &maximum_rows,
    1,
    unary,
    "-mr",
    "mtx_max_rows"
  },
  {
    &memory,
    1,
    unary,
    "-m",
    "mtx_memory"
  },
  {
    &verbose,
    0,
    nullary,
    "-q",
    "mtx_quiet"
  },
  {
    &verbose,
    1,
    nullary,
    "-v",
    "mtx_verbose"
  },
};

static void set_env_values(void)
{
  u32 len = sizeof(parse_table) / sizeof(parse_element);
  u32 i;
  for(i = 0; i < len; i++) {
    pparse_element elt = parse_table + i;
    if (NULL != elt->env_tag) {
      const char *env_val = getenv(elt->env_tag);
      if (NULL != env_val) {
        switch (elt->type) {
        case nullary:
          *(elt->flag) = elt->set_value;
          return;
        case unary:
          *(elt->flag) = strtoul(env_val, NULL, 0);
          return;
        default:
          assert(assert_var_zero != 0);
        }
      }
    }
  }
}

static u32 parse_tag(const char *tag, const char *val, int argc)
{
  u32 len = sizeof(parse_table) / sizeof(parse_element);
  u32 i;
  for(i = 0; i < len; i++) {
    pparse_element elt = parse_table + i;
    if ((nullary == elt->type || (unary == elt->type && argc > 2)) && 0 == strcmp(tag, elt->tag)) {
      switch (elt->type) {
      case nullary:
        *(elt->flag) = parse_table[i].set_value;
        return 1;
      case unary:
        *(elt->flag) = strtoul(val, NULL, 0);
        return 2;
      default:
        assert(assert_var_zero != 0);
      }
    }
  }
  return 0;
}

const char *const *parse_line(int argc, const char *const argv[], int *new_argc)
{
  u32 n;
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

static const char *usage =
  "[-v] [-m <memory>]"
  " [-mg <maximum grease level>]"
  " [-mr <maximum rows>]";

const char *parse_usage(void)
{
  return usage;
}

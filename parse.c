/*
 * $Id: parse.c,v 1.1 2002/07/09 09:08:12 jon Exp $
 *
 * Function to parse command line flags
 *
 */

#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct parse_element_struct
{
  int *flag;
  int set_value;
  const char *tag;
  const char *env_tag;
} parse_element, *pparse_element;

int verbose = 0;

static parse_element parse_table[] =
{
  {
    &verbose,
    1,
    "-v",
    "mtx_verbose"
  },
  {
    &verbose,
    0,
    "-q",
    "mtx_quiet"
  }
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

static int parse_tag(const char *tag)
{
  unsigned int len = sizeof(parse_table) / sizeof(parse_element);
  unsigned int i;
  for(i = 0; i < len; i++) {
    if (0 == strcmp(tag, parse_table[i].tag)) {
      *(parse_table[i].flag) = parse_table[i].set_value;
      return 1;
    }
  }
  return 0;
}

const char *const *parse_line(int argc, const char *const argv[], int *new_argc)
{
  assert(NULL != argv);
  assert(NULL != new_argc);
  set_env_values();
  while (argc > 1) {
    if (0 == parse_tag(argv[1])) {
      break;
    }
    argc--;
    argv++;
  }
  *new_argc = argc;
  return argv;
}

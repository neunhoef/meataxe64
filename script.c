/*
 * $Id: script.c,v 1.10 2005/07/24 09:32:45 jon Exp $
 *
 * Function to compute a script in two generators
 *
 */

#include "script.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "add.h"
#include "files.h"
#include "ident.h"
#include "mul.h"
#include "read.h"
#include "scale.h"
#include "utils.h"

static u32 id = 0;

static const char *new_id(const char *base)
{
  u32 len;
  char *res;
  assert(NULL != base);
  len = 11 + strlen(base);
  res = my_malloc(len + 1);
  sprintf(res, "%s.%u", base, id);
  id++;
  return res;
}

static int close_files(u32 s, FILE **files, const header **headers)
{
  u32 i;
  assert(NULL != files);
  assert(NULL != headers);
  for (i = 0; i < s; i++) {
    assert(NULL != files[i]);
    assert(NULL != headers[i]);
    fclose(files[i]);
    header_free(headers[i]);
  }
  free(files);
  free(headers);
  return 0;
}

/* Parse the incoming script up to the first plus */
/* The result is returned, and the rest of the script is indicated in rest */
/* rest is set to NULL if there is no plus */
static const char *parse_plus(const char *script, const char **rest, u32 *scalar,
                              u32 prime, const char *name)
{
  const char *plus;
  const char *res;
  assert(NULL != script);
  assert(NULL != rest);
  assert(NULL != name);
  assert(NULL != scalar);
  plus = strchr(script, '+');
  if (NULL == plus) {
    *rest = NULL;
    res = script;
  } else {
    char *tmp;
    u32 len = plus - script;
    *rest = plus + 1; /* Point after plus */
    tmp = my_malloc(len + 1);
    strncpy(tmp, script, len);
    tmp[len] = '\0';
    res = tmp;
  }
  /* Now check for the scale */
  if (my_isdigit(*res)) {
    char *rest;
    u32 val = strtoul(res, &rest, 0);
    if (val >= prime || 0 == val) {
      fprintf(stderr, "%s: script scalar %u exceeds field order %u or is zero, terminating\n", name, val, prime);
      return NULL;
    }
    *scalar = val;
    res = rest;
  } else {
    *scalar = 1;
  }
  return res;
}

static const char *get_multiplier(unsigned int argc, const char gen, const char *const args[], const char *name)
{
  const char *multiplier;
  switch(gen) {
  case 'A':
    multiplier = args[0];
    if (argc >= 1) {
      break;
    }
  case 'B':
    multiplier = args[1];
    if (argc >= 2) {
      break;
    }
  case 'C':
    multiplier = args[2];
    if (argc >= 3) {
      break;
    }
  case 'D':
    multiplier = args[3];
    if (argc >= 4) {
      break;
    }
  case 'E':
    multiplier = args[4];
    if (argc >= 5) {
      break;
    }
  case 'F':
    multiplier = args[5];
    if (argc >= 6) {
      break;
    }
  case 'G':
    multiplier = args[6];
    if (argc >= 7) {
      break;
    }
  case 'H':
    multiplier = args[7];
    if (argc >= 8) {
      break;
    }
  default:
    fprintf(stderr, "%s: script element '%c' is not a generator, terminating\n", name, gen);
    multiplier =  NULL;
  }
  return multiplier;
}

static int script_mul(const char *id, const char **out, const char *tmp, const char *script,
                      unsigned int argc, const char *const args[], const char *name)
{
  char gen;
  const char *current, *new, *multiplier;
  assert(NULL != id);
  assert(NULL != out);
  assert(NULL != tmp);
  assert(NULL != args);
  assert(0 != argc);
  assert(NULL != name);
  if ('I' == *script) {
    if ('\0' != script[1]) {
      fprintf(stderr, "%s: unexpected characters '%s' following I in script, terminating\n", name, script + 1);
      return 0;
    }
    *out = id;
    return 1;
  }
  gen = *script;
  script++;
  if ('\0' == gen) {
    fprintf(stderr, "%s: script matrix section missing, terminating\n", name);
    return 0;
  }
  multiplier = get_multiplier(argc, gen, args, name);
  if (NULL == multiplier) {
    return 0;
  }
  if ('\0' == *script) {
    /* Only one term, just use generator */
    *out = multiplier;
  } else {
    current = multiplier;
    while ('\0' != *script) {
      gen = *script;
      script++;
      if ('\0' == gen) {
        fprintf(stderr, "%s: script matrix section missing, terminating\n", name);
        return 0;
      }
      multiplier = get_multiplier(argc, gen, args, name);
      if (NULL == multiplier) {
        return 0;
      }
      new = new_id(tmp);
      if (0 == mul(current, multiplier, new, name)) {
        fprintf(stderr, "%s: script multiplication of %s by %s failed, terminating\n", name, current, multiplier);
        return 0;
      }
      current = new;
    }
    *out = current;
  }
  return 1;
}

int exec_script(const char *out, const char *tmp, const char *script,
                unsigned int argc, const char *const args[], const char *name)
{
  FILE **files = NULL;
  const header **headers;
  u32 i, prime = 1, nor, scalar;
  const char *id;
  const char *current, *new, *summand, *rest;
  int all_perm = 1;
  assert(NULL != out);
  assert(NULL != tmp);
  assert(NULL != script);
  assert(1 <= argc);
  assert(NULL != args);
  assert(NULL != name);
  files = my_malloc(argc * sizeof(FILE *));
  headers = my_malloc(argc * sizeof(const header *));
  for (i = 0; i < argc; i++) {
    if (0 == open_and_read_binary_header(files + i, headers + i, args[i], name)) {
      return close_files(i, files, headers);
    }
    if (1 != header_get_prime(headers[i])) {
      prime = header_get_prime(headers[i]);
      all_perm = 0;
    }
  }
  nor = header_get_nor(headers[0]);
  for (i = 0; i < argc; i++) {
    if (nor != header_get_noc(headers[i]) ||
        nor != header_get_nor(headers[i]) ||
        (header_get_prime(headers[i]) != prime && header_get_prime(headers[i]) != 1)) {
      fprintf(stderr, "%s: unsuitable input %s, terminating\n", name, args[i]);
      return close_files(argc, files, headers);
    }
  }
  (void)close_files(argc, files, headers);
  id = new_id(tmp);
  summand = parse_plus(script, &rest, &scalar, prime, name);
  if (NULL == summand) {
    return 0;
  }
  if ((NULL != rest || 'I' == *summand) && all_perm) {
    fprintf(stderr, "%s: cannot handle both generators as maps when adding, terminating\n", name);
    return 0;
  }
  if (0 == all_perm && 0 == ident(prime, nor, nor, 1, id, name)) {
    fprintf(stderr, "%s: unable to create identity, terminating\n", name);
    return 0;
    /* Note, id not needed if no additions */
  }
  if (0 == script_mul(id, &current, tmp, summand, argc, args, name)) {
    fprintf(stderr, "%s: unable to create summand %s, terminating\n", name, summand);
    return 0;
  }
  if (1 != scalar) {
    new = new_id(tmp);
    if (0 == scale(current, new, scalar, name)) {
      fprintf(stderr, "%s: script scale of %s by %u failed, terminating\n", name, current, scalar);
      return 0;
    }
    current = new;
  }
  while (NULL != rest) {
    const char *sum = new_id(tmp);
    script = rest;
    summand = parse_plus(script, &rest, &scalar, prime, name);
    if (NULL == summand) {
      return 0;
    }
    if (0 == script_mul(id, &new, tmp, summand, argc, args, name)) {
      fprintf(stderr, "%s: unable to create summand '%s', terminating\n", name, summand);
      return 0;
    }
    if (0 == scaled_add(new, current, sum, scalar, name)) {
      fprintf(stderr, "%s: unable to add %s and %s, terminating\n", name, current, new);
      return 0;
    }
    current = sum;
  }
  for (i = 0; i < argc; i++) {
    if (0 == strcmp(args[i], current)) {
      return scale(current, out, 1, name);
    }
  }
  rename(current, out); /* Turn current summand into output */
  return 1;
}

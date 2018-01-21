/*
 * $Id: zscript.c,v 1.2 2018/01/15 22:33:10 jon Exp $
 *
 * Compute a script in two generators
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "utils.h"
#include "field.h"
#include "funs.h"
#include "io.h"
#include "util.h"
#include "mfuns.h"

static char *fun_tmp;
#define FUN_TMP "_funs"

static u32 id = 0;

static const char *new_id(const char *base)
{
  u32 len;
  char *res;
  assert(NULL != base);
  len = 11 + strlen(base);
  res = malloc(len + 1);
  sprintf(res, "%s.%u", base, id);
  id++;
  return res;
}

/* Scalar multiply m1 by scalar to give m2 */
static int scale(const char *m1, const char *m2, u32 scalar, u32 prime, u32 nor, const char *tmp)
{
  const char *sid = new_id(tmp);
  if (0 == ident(prime, nor, nor, scalar, sid)) {
    fprintf(stderr, "zscript: unable to create scaled identity, terminating\n");
    return 0;
  }
  fMultiply(fun_tmp, sid, 0, m1, 0, m2, 0);
  return 1;
}

/* m3 = s * m1 + m2 */
static int scaled_add(const char *m1, const char *m2, const char *m3, u32 scalar, u32 prime, u32 nor, const char *tmp)
{
  if (1 == scalar) {
    fAdd(m1, 0, m2, 0, m3, 0);
  } else {
    const char *scaled = new_id(tmp);
    if (0 == scale(m1, scaled, scalar, prime, nor, tmp)) {
      fprintf(stderr, "zscript: unable to scale, terminating\n");
      return 0;
    }
    fAdd(scaled, 0, m2, 0, m3, 0);
  }
  return 1;
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
    tmp = malloc(len + 1);
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
      fMultiply(fun_tmp, current, 0, multiplier, 0, new, 0);
      current = new;
    }
    *out = current;
  }
  return 1;
}

static int exec_script(const char *out, const char *tmp, const char *script,
                       unsigned int argc, const char *const args[], const char *name)
{
  u32 i, prime = 1, nor, scalar;
  const char *id;
  const char *current, *new, *summand, *rest;
  uint64_t hdr[5];

  assert(NULL != out);
  assert(NULL != tmp);
  assert(NULL != script);
  assert(1 <= argc);
  assert(NULL != args);
  assert(NULL != name);
  EPeek(args[0], hdr);
  prime = hdr[1];
  nor = hdr[2];
  if (hdr[3] != nor) {
    LogString(80,"Matrices not square");
    exit(21);
  }
  for (i = 1; i < argc; i++) {
    EPeek(args[i], hdr);
    if (prime != hdr[1] || nor != hdr[2] || nor != hdr[3]) {
      LogString(80,"Matrices not compatible");
      exit(21);
    }
  }
  id = new_id(tmp);
  summand = parse_plus(script, &rest, &scalar, prime, name);
  if (NULL == summand) {
    return 0;
  }
  if (0 == ident(prime, nor, nor, 1, id)) {
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
    if (0 == scale(current, new, scalar, prime, nor, tmp)) {
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
    if (0 == scaled_add(new, current, sum, scalar, prime, nor, tmp)) {
      fprintf(stderr, "%s: unable to add %s and %s, terminating\n", name, current, new);
      return 0;
    }
    current = sum;
  }
  for (i = 0; i < argc; i++) {
    if (0 == strcmp(args[i], current)) {
      return scale(current, out, 1, prime, nor, tmp);
    }
  }
  rename(current, out); /* Turn current summand into output */
  return 1;
}

static const char *name = "zscript";

static void script_usage(void)
{
  fprintf(stderr, "%s: usage: %s <out_file> <tmp> <script> <a> [<other generators>]\n", name, name);
}

int main(int argc, const char *argv[])
{
  const char *out;
  const char *tmp;
  const char *script;
  const char *tmp_root = tmp_name();
  /* How long the temporary filename root is */
  size_t tmp_len = strlen(tmp_root);

  CLogCmd(argc, argv);
  if (5 > argc) {
    script_usage();
    exit(1);
  }
  /* Temporary root for functions */
  fun_tmp = malloc(tmp_len + sizeof(FUN_TMP) + 1);
  strcpy(fun_tmp, tmp_root);
  strcat(fun_tmp, FUN_TMP);
  out = argv[1];
  tmp = argv[2];
  script = argv[3];
  if (0 == exec_script(out, tmp, script, argc - 4, argv + 4, name)) {
    exit(1);
  }
  return 0;
}

/*
 * $Id: script.c,v 1.5 2002/06/25 10:30:12 jon Exp $
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

static unsigned id = 0;

static const char *new_id(const char *base)
{
  unsigned int len;
  char *res;
  assert(NULL != base);
  len = 11 + strlen(base);
  res = my_malloc(len + 1);
  sprintf(res, "%s.%d", base, id);
  id++;
  return res;
}

static int cleanup(FILE *inp1, FILE *inp2, FILE *outp)
{
  if (NULL != inp1)
    fclose(inp1);
  if (NULL != inp2)
    fclose(inp2);
  if (NULL != outp)
    fclose(outp);
  return 0;
}

/* Parse the incoming script up to the first plus */
/* The result is returned, and the rest of the script is indicated in rest */
/* rest is set to NULL if there is no plus */
static const char *parse_plus(const char *script, const char **rest, unsigned int *scalar,
                              unsigned int prime, const char *name)
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
    unsigned int len = plus - script;
    *rest = plus + 1; /* Point after plus */
    tmp = my_malloc(len + 1);
    strncpy(tmp, script, len);
    tmp[len] = '\0';
    res = tmp;
  }
  /* Now check for the scale */
  if (my_isdigit(*res)) {
    char *rest;
    unsigned int val = strtoul(res, &rest, 0);
    if (val >= prime || 0 == val) {
      fprintf(stderr, "%s: script scalar %d exceeds field order %d or is zero, terminating\n", name, val, prime);
      return 0;
    }
    *scalar = val;
    res = rest;
  } else {
    *scalar = 1;
  }
  return res;
}

static int script_mul(const char *m1, const char *m2, const char *id, const char **out,
                      const char *tmp, const char *script, const char *name)
{
  char gen;
  const char *current, *new, *multiplier;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != id);
  assert(NULL != out);
  assert(NULL != tmp);
  assert(NULL != script);
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
  if ('A' == gen) {
    multiplier = m1;
  } else if ('B' == gen) {
    multiplier = m2;
  } else {
    fprintf(stderr, "%s: script element '%c' is not a generator, terminating\n", name, gen);
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
      if ('A' == gen) {
        multiplier = m1;
      } else if ('B' == gen) {
        multiplier = m2;
      } else {
        fprintf(stderr, "%s: script element '%c' is not a generator, terminating\n", name, gen);
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
int exec_script(const char *m1, const char *m2, const char *m3,
                const char *tmp, const char *script, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  const header *h1, *h2;
  unsigned int prime, nor, scalar;
  const char *id;
  const char *current, *new, *summand, *rest;
  int is_perm1, is_perm2, both_perm;
  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != m3);
  assert(NULL != tmp);
  assert(NULL != script);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != inp1) {
      fclose(inp1);
      header_free(h1);
    }
    return 0;
  }
  nor = header_get_nor(h1);
  prime = header_get_prime(h1);
  is_perm1 = 1 == prime;
  is_perm2 = 1 == header_get_prime(h2);
  both_perm = is_perm1 && is_perm2; /* We'll allow this if no additions */
  if (nor != header_get_noc(h1) ||
      nor != header_get_noc(h1) ||
      nor != header_get_nor(h2) ||
      nor != header_get_noc(h2) ||
      (0 == is_perm1 && 0 == is_perm2 && prime != header_get_prime(h2))) {
    fprintf(stderr, "%s: unsuitable inputs %s and %s, terminating\n", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  if (is_perm1) {
    prime = header_get_prime(h2);
  }
  header_free(h1);
  header_free(h2);
  id = new_id(tmp);
  summand = parse_plus(script, &rest, &scalar, prime, name);
  if ((NULL != rest || 'I' == *summand) && both_perm) {
    fprintf(stderr, "%s: cannot handle both generators as maps when adding, terminating\n", name);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  if (0 == both_perm && 0 == ident(prime, nor, nor, 1, id, name)) {
    fprintf(stderr, "%s: unable to create identity, terminating\n", name);
    return cleanup(inp1, inp2, NULL);
    /* Note, id not needed if no additions */
  }
  if (0 == script_mul(m1, m2, id, &current, tmp, summand, name)) {
    fprintf(stderr, "%s: unable to create summand %s, terminating\n", name, summand);
    return cleanup(inp1, inp2, NULL);
  }
  if (1 != scalar) {
    new = new_id(tmp);
    if (0 == scale(current, new, scalar, name)) {
      fprintf(stderr, "%s: script scale of %s by %d failed, terminating\n", name, current, scalar);
      return cleanup(inp1, inp2, NULL);
    }
    current = new;
  }
  while (NULL != rest) {
    const char *sum = new_id(tmp);
    script = rest;
    summand = parse_plus(script, &rest, &scalar, prime, name);
    if (0 == script_mul(m1, m2, id, &new, tmp, summand, name)) {
      fprintf(stderr, "%s: unable to create summand %s, terminating\n", name, summand);
      return cleanup(inp1, inp2, NULL);
    }
    if (0 == scaled_add(new, current, sum, scalar, name)) {
      fprintf(stderr, "%s: unable to add %s and %s, terminating\n", name, current, new);
      return cleanup(inp1, inp2, NULL);
    }
    current = sum;
  }
  if (0 == strcmp(m1, current) || 0 == strcmp(m2, current)) {
    return scale(current, m3, 1, name);
  } else {
    (void)ren(current, m3); /* Turn current summand into output */
  }
  return 1;
}

/*
 * $Id: script.c,v 1.1 2002/03/10 22:45:28 jon Exp $
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
static const char *parse_plus(const char *script, const char **rest)
{
  const char *plus;
  assert(NULL != script);
  assert(NULL != rest);
  plus = strchr(script, '+');
  if (NULL == plus) {
    *rest = NULL;
    return script;
  } else {
    char *res;
    unsigned int len = plus - script;
    *rest = plus + 1; /* Point after plus */
    res = my_malloc(len + 1);
    strncpy(res, script, len);
    res[len] = '\0';
    return res;
  }
}


static int script_mul(const char *m1, const char *m2, const char *id, const char *out,
                      const char *tmp, const char *script, unsigned int prime, const char *name)
{
  unsigned int scalar;
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
    assert('\0' == script[1]);
    return scale(id, out, 1, name);
  }
  if (my_isdigit(*script)) {
    char *rest;
    scalar = strtoul(script, &rest, 0);
    if (scalar >= prime || 0 == scalar) {
      fprintf(stderr, "%s: script scalar %d exceeds field order %d or is zero (isdigit gives %d), terminating\n", name, scalar, prime, isdigit(*script));
      return 0;
    }
    script = rest;
  } else {
    scalar = 1;
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
  current = new_id(tmp);
  if (0 == scale(multiplier, current, scalar, name)) {
    fprintf(stderr, "%s: script scale of %s by %d failed, terminating\n", name, multiplier, scalar);
    return 0;
  }
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
  ren(current, out);
  return 1;
}

int exec_script(const char *m1, const char *m2, const char *m3,
                const char *tmp, const char *script, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  const header *h1, *h2;
  unsigned int prime, nor;
  const char *id;
  const char *current, *new, *summand, *rest;
  NOT_USED(m3);
  NOT_USED(script);
  NOT_USED(new);
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
  if (0 == header_compare(h1, h2) || nor != header_get_noc(h1)) {
    fprintf(stderr, "%s: unsuitable inputs %s and %s, terminating", name, m1, m2);
    header_free(h1);
    header_free(h2);
    return cleanup(inp1, inp2, NULL);
  }
  header_free(h1);
  header_free(h2);
  id = new_id(tmp);
  if (0 == ident(prime, nor, nor, 1, id, name)) {
    fprintf(stderr, "%s: unable to create identity, terminating", name);
    return cleanup(inp1, inp2, NULL);
  }
  summand = parse_plus(script, &rest);
  current = new_id(tmp);
  if (0 == script_mul(m1, m2, id, current, tmp, summand, prime, name)) {
    fprintf(stderr, "%s: unable to create summand %s, terminating", name, summand);
    return cleanup(inp1, inp2, NULL);
  }
  while (NULL != rest) {
    const char *sum = new_id(tmp);
    script = rest;
    summand = parse_plus(script, &rest);
    new = new_id(tmp);
    if (0 == script_mul(m1, m2, id, new, tmp, summand, prime, name)) {
      fprintf(stderr, "%s: unable to create summand %s, terminating", name, summand);
      return cleanup(inp1, inp2, NULL);
    }
    if (0 == add(current, new, sum, name)) {
      fprintf(stderr, "%s: unable to add %s and %s, terminating", name, current, new);
      return cleanup(inp1, inp2, NULL);
    }
    current = sum;
  }
  (void)ren(current, m3); /* Trun current summand into output */
  return 1;
}

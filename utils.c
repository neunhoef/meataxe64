/*
 * $Id: utils.c,v 1.14 2001/11/12 13:43:38 jon Exp $
 *
 * Utils for meataxe
 *
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>

unsigned int bits_in_unsigned_int = CHAR_BIT * sizeof(unsigned int);

int my_isspace(int i)
{
  return (i == ' ') || (i == 9) || (i == 10) || (i == 13);
}

unsigned int read_decimal(const char *str, unsigned int len)
{
  char s[7];

  assert(0 < len);
  assert(len <= 6);
  assert(NULL != str);
  strncpy(s, str, len);
  s[len] = '\0';
  return strtoul(s, NULL, 0);
}

unsigned int getin(FILE *f, unsigned int a)
{
  unsigned int j = 0;
  char *s = my_malloc(a + 1);
  char *t = fgets(s, a + 1, f);
 
  if (NULL == t) {
    j = 0;
  } else {
    j = strtoul(s, NULL, 0);
  }
  free(s);
  return j;
}
 
static char *get_str_sub(FILE *f, unsigned int depth)
{
  int c = fgetc(f);
  char *out;
  if (c >= 0 && 0 == my_isspace(c)) {
    out = get_str_sub(f, depth + 1);
  } else {
    out = my_malloc(depth + 1);  
    c = '\0';
  }
  out[depth] = c;
  return out;
}

const char *get_str(FILE *f)
{
  int c = fgetc(f);
  while (my_isspace(c)) {
    c = fgetc(f);
  }
  if (c >= 0) {
    char *out = get_str_sub(f, 1);  
    *out = c;
    return out;
  } else {
    fprintf(stderr, "Failed to read string, terminating\n");
    exit(1);
    return NULL; /* Avoid compiler warning */
  }
}

unsigned int bits_of(unsigned int n)
{
  unsigned int i = 0;
  assert(n >= 1);
  n -= 1;
  while(n > 0) {
    n >>= 1;
    i++;
  }
  return i;
}

/* Compute the number of decimal digits needed to represent 0 - n-1 */
unsigned int digits_of(unsigned int n)
{
  unsigned int i = 0;
  assert(n >= 1);
  n -= 1;
  while(n > 0) {
    n /= 10;
    i++;
  }
  return i;
}

void *my_malloc(size_t size)
{
  void *ret;
  assert(0 != size);
  ret = malloc(size);
  if (NULL == ret) {
    fprintf(stderr, "Failed to allocate %d bytes, terminating\n", size);
    exit(1);
  }
  return ret;
}

void copy_rest(FILE *new, FILE *old)
{
  char temp[1000];
  do {
    unsigned int i = fread(temp, 1, 1000, old);
    if (0 != i) {
      fwrite(temp, 1, i, new);
    } else {
      break;
    }
  } while (1);
}

unsigned int skip_whitespace(unsigned int i, const char *chars)
{
  unsigned int j = strlen(chars);
  while (1) {
    if (i >= j-1) {
      return i;
    } else {
      int k = chars[i];
      if (my_isspace(k)) {
	++i;
      } else {
	return i;
      }
    }
  }
}

unsigned int skip_non_white(unsigned int i, const char *chars)
{
  unsigned int j = strlen(chars);
  while (1) {
    if (i >= j-1) {
      return i;
    } else {
      int k = chars[i];
      if (my_isspace(k)) {
	return i;
      } else {
	++i;
      }
    }
  }
}

int get_task_line(char *line, FILE *input)
{
  unsigned int i;
  char *poo = fgets(line, MAX_LINE-1, input);
  if (poo == NULL || strlen(line) <= 1) {
    return 0;
  } else {
    i = strlen(line);
    if (i >= MAX_LINE-2) {
      fprintf(stderr, "Command file line too long\n");
      return 0;
    }
    return 1;
  }
}

int pow(unsigned int n, unsigned int index, unsigned int *res)
{
  assert(0 != n);
  if (0 == index) {
    *res = 1;
    return 1;
  } else if (1 == index) {
    *res = n;
    return 1;
  } else {
    if (0 != pow(n, index-1, res)) {
      if (*res < (UINT_MAX / n)) {
        *res *= n;
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
}

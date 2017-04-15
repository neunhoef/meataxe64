/*
 * Compute the determinant of a matrix, using temporary files
 *
 */

#include "detf.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "clean.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "maps.h"
#include "matrix.h"
#include "memory.h"
#include "parse.h"
#include "primes.h"
#include "psign.h"
#include "read.h"
#include "system.h"
#include "utils.h"
#include "write.h"

typedef struct file_struct file_struct, *file;

struct file_struct
{
  FILE *f;
  const char *name;
  int created;
  file next;
};

static void cleanup(const file_struct t1, const file_struct t2)
{
  if (t1.created) {
    (void)remove(t1.name);
  }
  if (t2.created) {
    (void)remove(t2.name);
  }
}

static word sign_to_elt(int sign, word ch)
{
  return (1 == sign) ? 1 : ch - 1; /* map 1 -> 1, -1 -> p-1 */
}

word detf(const char *m1, const char *dir, const char *name)
{
  FILE *inp;
  const header *h;
  u32 prime, nob, nor, len, ch, n, i, r, rows_to_do, max_rows, step1, step2;
  word d;
  word sign;
  word **mat1, **mat2;
  int *int_map;
  row_ops row_operations;
  prime_ops prime_operations;
  grease_struct grease;
  const char *tmp = tmp_name();
  char *name1, *name2;
  file_struct f, t1, t2;
  file in, out;
  word *map;
  assert(NULL != tmp);
  assert(NULL != dir);
  assert(NULL != m1);
  i = strlen(tmp) + strlen(dir);
  name1 = my_malloc(i + 4);
  name2 = my_malloc(i + 4);
  sprintf(name1, "%s/%s.0", dir, tmp);
  sprintf(name2, "%s/%s.1", dir, tmp);
  f.name = m1;
  t1.name = name1;
  t2.name = name2;
  f.next = &t1;
  t1.next = &t2;
  t2.next = &t1;
  in = &f;
  out = &t1;
  t1.created = 0;
  t2.created = 0;
  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    exit(1);
  }
  f.f = inp;
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  ch = prime_divisor(prime);
  header_free(h);
  if (1 == prime) {
    word *map = malloc_map(nor);
    int sign;
    if (0 == read_map(inp, nor, map, name, m1)) {
      map_free(map);
      return 0;
    }
    sign = psign_value(map, nor);
    return sign_to_elt(sign, ch);
  }
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  max_rows = memory_rows(len, 800);
  r = memory_rows(len, 100);
  if (r < prime) {
    fprintf(stderr, "%s: cannot allocate %u rows for %s, terminating\n", name, prime, m1);
    fclose(inp);
    exit(2);
  }
  (void)grease_level(prime, &grease, r);
  /* Now read the matrix */
  rows_to_do = nor;
  step1 = (max_rows > nor) ? nor : max_rows;
  step2 = r;
  mat1 = matrix_malloc(step1);
  mat2 = matrix_malloc(step2);
  for (n = 0; n < step1; n++) {
    mat1[n] = memory_pointer_offset(0, n, len);
  }
  for (n = 0; n < step2; n++) {
    mat2[n] = memory_pointer_offset(800, n, len);
  }
  r = 0; /* Initialise rank */
  d = 1; /* Initial determinant */
  map = my_malloc(nor * sizeof(*map));
  while (rows_to_do > 0) {
    u32 rows_remaining = rows_to_do;
    u32 stride = (step1 > rows_remaining) ? rows_remaining : step1;
    errno = 0;
    if (0 == endian_read_matrix(in->f, mat1, len, stride)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
      fclose(in->f);
      cleanup(t1, t2);
      exit(1);
    }
    echelise_with_det(&row_operations, mat1, stride, &n, &int_map, &d, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 1, name);
    rows_remaining -= stride;
    if (0 != n) {
      /* Some addition to the rank */
      r += n;
      if (stride == n) {
        u32 j;
        for (j = 0; j < n; j++) {
          map[(nor - rows_to_do) + j] = int_map[j];
        }
      } else {
        d = 0; /* Singular */
        free(int_map);
        break; /* No point in continuing */
      }
      if (rows_remaining > 0) {
        u32 rows_written = 0;
        errno = 0;
        out->f = fopen64(out->name, "wb");
        out->created = 1;
        if (NULL == out->f) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, out->name);
          fclose(in->f);
          cleanup(t1, t2);
          exit(1);
        }
        for (i = 0; i < rows_remaining; i += step2) {
          u32 stride2 = (step2 + i > rows_remaining) ? rows_remaining - i : step2;
          u32 j;
          errno = 0;
          if (0 == endian_read_matrix(in->f, mat2, len, stride2)) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
            fclose(in->f);
            fclose(out->f);
            cleanup(t1, t2);
            exit(1);
          }
          clean(&row_operations, mat1, stride, mat2, stride2, int_map, NULL, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, verbose, name);
          for (j = 0; j < stride2; j++) {
            if (0 == row_is_zero(mat2[j], len)) {
              errno = 0;
              if (0 == endian_write_row(out->f, mat2[j], len)) {
                if ( 0 != errno) {
                  perror(name);
                }
                fprintf(stderr, "%s: cannot write matrix for %s, terminating\n", name, out->name);
                fclose(in->f);
                fclose(out->f);
                cleanup(t1, t2);
                exit(1);
              }
              rows_written++;
            }
          }
        }
        fclose(in->f);
        fclose(out->f);
        in = in->next;
        out = out->next;
        if (rows_written > 0) {
          errno = 0;
          in->f = fopen64(in->name, "rb");
          if (NULL == in->f) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, in->name);
            cleanup(t1, t2);
            exit(1);
          }
        }
        rows_remaining = rows_written; 
      }
    } else {
      /* Just keep reading from same input */
      d = 0; /* Singular */
      free(int_map);
      break;
    }
    rows_to_do = rows_remaining;
    free(int_map);
  }
  sign = sign_to_elt(psign_value(map, nor), ch);
  d = prime_operations.mul(d, sign);
  matrix_free(mat1);
  matrix_free(mat2);
  free(map);
  cleanup(t1, t2);
  return d;
}

word det2f(const char *m1, const char *dir, const char *name)
{
  FILE *inp;
  const header *h;
  u32 prime, nob, nor, len, n, i, r, rows_to_do, max_rows, step1, step2;
  word d;
  word sign;
  word **mat1, **mat2;
  int *int_map;
  row_ops row_operations;
  prime_ops prime_operations;
  grease_struct grease;
  const char *tmp = tmp_name();
  char *name1, *name2;
  file_struct f, t1, t2;
  file in, out;
  word *map;
  assert(NULL != tmp);
  assert(NULL != dir);
  assert(NULL != m1);
  i = strlen(tmp) + strlen(dir);
  name1 = my_malloc(i + 4);
  name2 = my_malloc(i + 4);
  sprintf(name1, "%s/%s.0", dir, tmp);
  sprintf(name2, "%s/%s.1", dir, tmp);
  f.name = m1;
  t1.name = name1;
  t2.name = name2;
  f.next = &t1;
  t1.next = &t2;
  t2.next = &t1;
  in = &f;
  out = &t1;
  t1.created = 0;
  t2.created = 0;
  if (0 == open_and_read_binary_header(&inp, &h, m1, name)) {
    exit(1);
  }
  f.f = inp;
  prime = header_get_prime(h);
  nob = header_get_nob(h);
  nor = header_get_nor(h);
  len = header_get_len(h);
  header_free(h);
  if (1 == prime) {
    return 0; /* Det2 isn't for permutations */
  }
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  max_rows = memory_rows(len, 800);
  r = memory_rows(len, 100);
  if (r < prime) {
    fprintf(stderr, "%s: cannot allocate %u rows for %s, terminating\n", name, prime, m1);
    fclose(inp);
    exit(2);
  }
  max_grease = 1;
  (void)grease_level(prime, &grease, r);
  /* Now read the matrix */
  rows_to_do = nor;
  step1 = (max_rows > nor) ? nor : max_rows;
  step2 = r;
  mat1 = matrix_malloc(step1);
  mat2 = matrix_malloc(step2);
  for (n = 0; n < step1; n++) {
    mat1[n] = memory_pointer_offset(0, n, len);
  }
  for (n = 0; n < step2; n++) {
    mat2[n] = memory_pointer_offset(800, n, len);
  }
  r = 0; /* Initialise rank */
  d = 1; /* Initial determinant */
  map = my_malloc(nor * sizeof(*map));
  while (rows_to_do > 0) {
    u32 rows_remaining = rows_to_do;
    u32 stride = (step1 > rows_remaining) ? rows_remaining : step1;
    errno = 0;
    if (0 == endian_read_matrix(in->f, mat1, len, stride)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
      fclose(in->f);
      cleanup(t1, t2);
      exit(1);
    }
    echelise_with_det2(&row_operations, mat1, stride, &n, &int_map, &d, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, 1, name);
    rows_remaining -= stride;
    if (0 != n) {
      /* Some addition to the rank */
      r += n;
      if (stride == n) {
        u32 j;
        for (j = 0; j < n; j++) {
          map[(nor - rows_to_do) + j] = int_map[j];
        }
      } else {
        d = 0; /* Singular */
        free(int_map);
        break; /* No point in continuing */
      }
      if (rows_remaining > 0) {
        u32 rows_written = 0;
        errno = 0;
        out->f = fopen64(out->name, "wb");
        out->created = 1;
        if (NULL == out->f) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, out->name);
          fclose(in->f);
          cleanup(t1, t2);
          exit(1);
        }
        for (i = 0; i < rows_remaining; i += step2) {
          u32 stride2 = (step2 + i > rows_remaining) ? rows_remaining - i : step2;
          u32 j;
          errno = 0;
          if (0 == endian_read_matrix(in->f, mat2, len, stride2)) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot read matrix for %s, terminating\n", name, in->name);
            fclose(in->f);
            fclose(out->f);
            cleanup(t1, t2);
            exit(1);
          }
          clean2(&row_operations, mat1, stride, mat2, stride2, int_map, NULL, NULL, 0, grease.level, prime, len, nob, 900, 0, 0, verbose, name);
          for (j = 0; j < stride2; j++) {
            if (0 == row_is_zero(mat2[j], len)) {
              errno = 0;
              if (0 == endian_write_row(out->f, mat2[j], len)) {
                if ( 0 != errno) {
                  perror(name);
                }
                fprintf(stderr, "%s: cannot write matrix for %s, terminating\n", name, out->name);
                fclose(in->f);
                fclose(out->f);
                cleanup(t1, t2);
                exit(1);
              }
              rows_written++;
            }
          }
        }
        fclose(in->f);
        fclose(out->f);
        in = in->next;
        out = out->next;
        if (rows_written > 0) {
          errno = 0;
          in->f = fopen64(in->name, "rb");
          if (NULL == in->f) {
            if ( 0 != errno) {
              perror(name);
            }
            fprintf(stderr, "%s: cannot open temporary output %s, terminating\n", name, in->name);
            cleanup(t1, t2);
            exit(1);
          }
        }
        rows_remaining = rows_written; 
      }
    } else {
      /* Just keep reading from same input */
      d = 0; /* Singular */
      free(int_map);
      break;
    }
    rows_to_do = rows_remaining;
    free(int_map);
  }
  sign = sign_to_elt(psign_value(map, nor), prime);
  d = prime_operations.mul(d, sign);
  matrix_free(mat1);
  matrix_free(mat2);
  free(map);
  cleanup(t1, t2);
  return d;
}

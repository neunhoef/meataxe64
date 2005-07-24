/*
 * $Id: signf.c,v 1.5 2005/07/24 09:32:45 jon Exp $
 *
 * Function compute the orthogonal group sign
 *
 */

#include "signf.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "grease.h"
#include "header.h"
#include "matrix.h"
#include "memory.h"
#include "mul.h"
#include "primes.h"
#include "read.h"
#include "rows.h"
#include "singular.h"
#include "system.h"
#include "utils.h"
#include "write.h"

int sign(const char *qform, const char *bform, const char *dir, const char *name)
{
  FILE *qinp = NULL, *binp = NULL;
  const header *h_inq, *h_inb;
  u32 prime, nob, nor, noc, len, m, n, out_num;
  word **mat;
  word *sing_row1, *sing_row2, *products;
  int res;
  long long *posns;
  grease_struct grease;
  prime_ops prime_operations;
  row_ops row_operations;
  const char *tmp = tmp_name();
  char *id_name;
  FILE *idp;
  assert(NULL != bform);
  assert(NULL != qform);
  assert(NULL != name);
  m = strlen(tmp) + strlen(dir);
  id_name = my_malloc(m + 4);
  sprintf(id_name, "%s/%s.1", dir, tmp);
  if (0 == open_and_read_binary_header(&qinp, &h_inq, qform, name) ||
      0 == open_and_read_binary_header(&binp, &h_inb, bform, name)) {
    if (NULL != qinp) {
      fclose(qinp);
      header_free(h_inq);
    }
    return 1;
  }
  prime = header_get_prime(h_inb);
  if (1 == prime || 1 == header_get_prime(h_inq)) {
    fprintf(stderr, "%s: form cannot be a map, terminating\n", name);
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
    header_free(h_inb);
    return 1;
  }
  nob = header_get_nob(h_inb);
  nor = header_get_nor(h_inb);
  noc = header_get_noc(h_inb);
  len = header_get_len(h_inb);
  if (noc != nor) {
    fprintf(stderr, "%s: form must be square, terminating\n", name);
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
    header_free(h_inb);
    return 1;
  }
  header_free(h_inb);
  if (header_get_nob(h_inq) != nob ||
      header_get_noc(h_inq) != noc ||
      header_get_nor(h_inq) != nor ||
      header_get_len(h_inq) != len ||
      header_get_prime(h_inq) != prime) {
    fclose(qinp);
    header_free(h_inq);
    fclose(binp);
  }
  header_free(h_inq);
  if (nor % 2 != 0) {
    fclose(qinp);
    fclose(binp);
    return 0; /* We'll call odd dimension + */
  }
  /* TODO: use the memory better */
  n = memory_rows(len, 100);
  m = memory_rows(len, 900);
  if (m < 8 || n < prime) {
    fprintf(stderr, "%s: cannot allocate %u rows, terminating\n",
            name, prime * 10);
    fclose(qinp);
    fclose(binp);
    exit(2);
  }
  if (m > 8) {
    /* Don't need more than this */
    m = 8;
  }
  (void)grease_level(prime, &grease, n);
  primes_init(prime, &prime_operations);
  rows_init(prime, &row_operations);
  grease_init(&row_operations, &grease);
  if (0 == grease_allocate(prime, len, &grease, 900)){
    fprintf(stderr, "%s: unable to allocate grease, terminating\n", name);
    fclose(qinp);
    fclose(binp);
    return 1;
  }
  /* Now allocate the matrix for the identity plus workspace */
  mat = matrix_malloc(m);
  for (n = 0; n < m; n++) {
    mat[n] = memory_pointer_offset(0, n, len);
  }
  sing_row1 = mat[6];
  sing_row2 = mat[7];
  posns = my_malloc(nor * sizeof(long long));
  idp = fopen64(id_name, "w+b");
  if (NULL == idp) {
    fprintf(stderr, "%s: unable to allocate intermediate identity, terminating\n", name);
    fclose(qinp);
    fclose(binp);
    matrix_free(mat);
    free(posns);
    return 1;
  }
  /* Now set up the identity */
  row_init(mat[0], len);
  /* TODO: Reflect the identity */
  for (n = 0; n < nor; n++) {
    put_element_to_row(nob, n, mat[0], 1);
    posns[n] = ftello64(idp);
    errno = 0;
    if (0 == endian_write_row(idp, mat[0], len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write row to %s, terminating\n",
              name, id_name);
      fclose(qinp);
      fclose(binp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      return 1;
    }
    put_element_to_row(nob, n, mat[0], 0);
  }
  fflush(idp);
  products = my_malloc(nor * sizeof(word));
  while (nor > 2) {
    int start_pos = -1;
    fseeko64(idp, posns[0], SEEK_SET);
    errno = 0;
    if (0 == endian_read_matrix(idp, mat, len, 3)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
              name, id_name);
      fclose(binp);
      fclose(qinp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      free(products);
      return 1;
    }
    assert(nor >= 3);
    res = singular_vector(&row_operations, mat, mat + 3, sing_row1, &out_num, qinp,
                          noc, 3, nob, len, prime, &grease, 0, qform, name);
    /* TODO: to use nor - 3 above */
    if (0 != res) {
      fprintf(stderr, "%s: cannot find a singular vector, terminating\n", name);
      fclose(binp);
      fclose(qinp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      free(products);
      return 1;
    }
    assert(nor > 3);
    /* TODO: Use shuffle */
    fseeko64(idp, posns[nor - 1], SEEK_SET);
    errno = 0;
    if (0 == endian_read_row(idp, mat[0], len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read row from %s, terminating\n",
              name, id_name);
      fclose(qinp);
      fclose(binp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      return 1;
    }
    fseeko64(idp, posns[out_num], SEEK_SET);
    errno = 0;
    if (0 == endian_write_row(idp, mat[0], len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write row to %s, terminating\n",
              name, id_name);
      fclose(qinp);
      fclose(binp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      return 1;
    }
    /* TODO: use skip_mul_from_store */
    if (0 == mul_from_store(&sing_row1, &sing_row2, binp, 0, noc, len, nob, 1, noc, prime,
                            &grease, 0, bform, name)) {
      fclose(binp);
      fclose(qinp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      free(products);
      return 1;
    }
    for (n = 0; n < len; n++) {
      if (0 != sing_row2[n]) {
        start_pos = n;
        break;
      }
    }
    assert(start_pos >= 0);
    fseeko64(idp, posns[0], SEEK_SET);
    for (n = 0; n + 1 < nor; n++) {
      errno = 0;
      if (0 == endian_read_row(idp, mat[0], len)) {
        if ( 0 != errno) {
          perror(name);
        }
        fprintf(stderr, "%s: failed to read row from %s, terminating\n",
                name, id_name);
        fclose(qinp);
        fclose(binp);
        fclose(idp);
        matrix_free(mat);
        free(posns);
        return 1;
      }
      products[n] = (*row_operations.product)(mat[0] + start_pos, sing_row2 + start_pos, len - start_pos);
    }
    n = 0;
    while (n + 1 < nor) {
      word elt = products[n];
      if (0 != elt) {
        /* Read this row and possibly convert to a weight one vector */
        fseeko64(idp, posns[n], SEEK_SET);
        errno = 0;
        if (0 == endian_read_row(idp, mat[0], len)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: failed to read row from %s, terminating\n",
                  name, id_name);
          fclose(qinp);
          fclose(binp);
          fclose(idp);
          matrix_free(mat);
          free(posns);
          return 1;
        }
        if (1 != elt) {
          elt = (*prime_operations.invert)(elt);
          (*row_operations.scaler_in_place)(mat[0], len, elt);
        }
        res = n;
        break;
      }
      n++;
    }
    if (nor == n) {
      /* Failed to find a non-zero product, catastrophe */
      fprintf(stderr, "%s: failed to find non-orthognal vector in %s, terminating\n",
              name, id_name);
      fclose(qinp);
      fclose(binp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      return 1;
    }
    n++;
    while (n + 1 < nor) {
      word elt = products[n];
      if (0 != elt) {
        elt = (*prime_operations.negate)(elt);
        /* Read the row and add in a multiple of the discard vector to keep this row in the perp space */
        fseeko64(idp, posns[n], SEEK_SET);
        errno = 0;
        if (0 == endian_read_row(idp, mat[1], len)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: failed to read row from %s, terminating\n",
                  name, id_name);
          fclose(qinp);
          fclose(binp);
          fclose(idp);
          matrix_free(mat);
          free(posns);
          return 1;
        }
        if (1 == elt) {
          (*row_operations.incer)(mat[0], mat[1], len);
        } else {
          (*row_operations.scaled_incer)(mat[0], mat[1], len, elt);
        }
        fseeko64(idp, posns[n], SEEK_SET);
        errno = 0;
        if (0 == endian_write_row(idp, mat[1], len)) {
          if ( 0 != errno) {
            perror(name);
          }
          fprintf(stderr, "%s: failed to write row to %s, terminating\n",
                  name, id_name);
          fclose(qinp);
          fclose(binp);
          fclose(idp);
          matrix_free(mat);
          free(posns);
          return 1;
        }
      }
      n++;
    }
    /* TODO: Use shuffle */
    /* Now remove mat[res] as this isn't a null vector */
    assert(nor > 3);
    fseeko64(idp, posns[nor - 2], SEEK_SET);
    errno = 0;
    if (0 == endian_read_row(idp, mat[0], len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to read row from %s, terminating\n",
              name, id_name);
      fclose(qinp);
      fclose(binp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      return 1;
    }
    fseeko64(idp, posns[res], SEEK_SET);
    errno = 0;
    if (0 == endian_write_row(idp, mat[0], len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: failed to write row to %s, terminating\n",
              name, id_name);
      fclose(qinp);
      fclose(binp);
      fclose(idp);
      matrix_free(mat);
      free(posns);
      return 1;
    }
    nor -= 2;
  }
  free(products);
  fclose(binp);
  fseeko64(idp, posns[0], SEEK_SET);
  errno = 0;
  if (0 == endian_read_matrix(idp, mat, len, 2)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read rows from %s, terminating\n",
            name, id_name);
    fclose(qinp);
    fclose(idp);
    matrix_free(mat);
    free(posns);
    return 1;
  }
  /* TODO: Use the right vectors */
  res = singular_vector(&row_operations, mat, mat + 2, sing_row1, &out_num, qinp,
                        noc, nor, nob, len, prime, &grease, 0, qform, name);
  fclose(idp);
  (void)remove(id_name);
  matrix_free(mat);
  free(posns);
  fclose(qinp);
  return res;
}

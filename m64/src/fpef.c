// Copyright (C) Richard Parker   2017
// Meataxe64 Nikolaus version
// fpef.c   fProduceNREF

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "mezz.h"
#include "field.h"
#include "funs.h"
#include "parse.h"
#include "utils.h"

uint64_t fProduceNREF(const char *tmp, const char *m1, int s1, 
                      const char *b2, int s2, const char *m3, int s3)
{
    uint64_t rank;
    NOT_USED(tmp);
    if (very_verbose) {
      printf("fProduceNREF %s %s %s\n", m1, b2, m3);
    }
    rank=mpef(m1,s1,b2,s2,m3,s3);
    if (very_verbose) {
      printf("fProduceNREF %s %s %s done\n", m1, b2, m3);
    }
    return rank;
}

/* end of fpef.c  */

/*
 * Implementation of mactype to allow fieldreg on ARM
 */

#include <stdint.h>
#include <stddef.h>
#include "field.h"
#include "pcrit.h"

void mactype(char *mact)
{
  mact[0] = 'a'; /* Minimal class */
  mact[1] = '0'; /* Cache class 0 */
}

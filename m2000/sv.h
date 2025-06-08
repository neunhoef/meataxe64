/*
 * $Id: sv.h,v 1.1 2006/03/04 09:02:06 jon Exp $
 *
 * Function to compute Schreier vector
 *
 */

#ifndef included__sv
#define included__sv

extern int sv(word point, const char *sv_out, const char *bv_out,
              u32 argc, const char *const args[],
              const char *name);

#endif

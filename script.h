/*
 * $Id: script.h,v 1.2 2002/09/10 17:10:50 jon Exp $
 *
 * Function to compute a script in two generators
 *
 */

#ifndef included__script
#define included__script

extern int exec_script(const char *out, const char *tmp, const char *script,
                       unsigned int argc, const char * const args[], const char *name);

#endif

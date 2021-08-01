/*
 * $Id: script.h,v 1.3 2021/08/01 09:53:56 jon Exp $
 *
 * Function to compute a script in two generators
 *
 */

#ifndef included__script
#define included__script

extern int exec_script(const char *out, const char *tmp, const char *script,
                       unsigned int argc, const char * const args[], const char *name);

extern int exec_scripts(const char *scripts_file, unsigned int argc,
                        const char *const args[], const char *name);

#endif

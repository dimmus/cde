/*
 * Force-included into every C and C++ compile by configure's
 * --enable-hardening (CDE_CPPFLAGS has -U_FORTIFY_SOURCE -include this).
 *
 * _FORTIFY_SOURCE only works when optimising, and glibc warns about every
 * file otherwise, so it is defined here, where __OPTIMIZE__ is known,
 * rather than on the command line: "make CFLAGS=-O0" then builds without
 * it and without warnings.  -D and -U options are processed before
 * -include files, so a target that sets its own level (FORTIFY_SOURCE_2)
 * keeps it.
 */
#if defined(__OPTIMIZE__) && !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

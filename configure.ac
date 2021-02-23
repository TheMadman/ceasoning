#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ceasoning], [0.1], [marcus@harrisonland.co.uk])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR
LT_INIT
# AC_CONFIG_SRCDIR([src/saltresources.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stddef.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC

#AC_CONFIG_TESTDIR([tests])
AC_CONFIG_FILES([Makefile
                 src/Makefile
		 tests/Makefile])
#AM_MISSING_PROG([AUTOM4TE], [autom4te])
AC_OUTPUT

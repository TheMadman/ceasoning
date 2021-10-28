#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ceasoning], [0.1], [marcus@harrisonland.co.uk])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AM_PROG_AR
LT_INIT
# AC_CONFIG_SRCDIR([src/saltresources.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h stddef.h stdlib.h string.h unistd.h socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

# Adds Automake conditionals.
AM_CONDITIONAL([DEVELOP], [test $DEVELOP])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

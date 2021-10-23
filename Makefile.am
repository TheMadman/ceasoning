ACLOCAL_AMFLAGS = -I m4
includes = -I$(top_srcdir)/src -I$(top_srcdir)/src/platforms/posix
if DEVELOP
AM_CFLAGS = $(includes) -g -O0 --coverage
else
AM_CFLAGS = $(includes)
endif

csaltincludedir = $(includedir)/csalt/
include $(srcdir)/src/Makefile.am
include $(srcdir)/tests/Makefile.am
dist_doc_DATA = README.md

.PHONY: static-check

static-check:
	cppcheck --cppcheck-build-dir="$(builddir)" "$(top_srcdir)"/src "$(top_srcdir)"/tests

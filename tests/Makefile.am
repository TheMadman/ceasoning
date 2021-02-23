TESTS = salt_resources_use
check_PROGRAMS = salt_resources_use
salt_resources_use_SOURCES = salt_resources_use.c

# Set this or explicitely #include "../src/*.h"?
# Doing this for now in case we move headers in
# the future
AM_CFLAGS = -I$(top_srcdir)/src
AM_LDFLAGS = $(top_srcdir)/src/saltresources.o

